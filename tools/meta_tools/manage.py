import os
import sys
sys.path.append('tools\\meta_tools')
from typing import Any, Dict, List, Optional, Union, Callable
import click
from importlib import import_module
from commands import merge_commands, PropertyDict
from collections import Counter, OrderedDict
from click.utils import make_str

def setup_cli():
    class CommandTask(object):
        def __init__(self, name: str, prerequisites: Optional[List], execution_order: Optional[List],
                     command_args: Dict, callback: Callable):
            self.callback = callback
            self.name = name
            self.prerequisites = prerequisites
            self.execution_order = execution_order
            self.command_args = command_args

        def __call__(self, global_options: PropertyDict, command_args: Dict=None):
            if command_args is None:
                command_args = self.command_args

            self.callback(global_options, **command_args)

    class CustomCommand(click.Command):
        """
            Encapsulate the command into a CommandTask
        """
        def __init__(self, name: Optional[str]=None, prerequisites: Optional[List]=None,
                      execution_order: Optional[List]=None, **kwargs: Any):
            super(CustomCommand, self).__init__(name, **kwargs)

            if prerequisites is None:
                prerequisites = []

            if execution_order is None:
                execution_order = []

            self.short_help: str = self.short_help or self.help.split('\n', 1)[0]
            self.allow_extra_args = True
            self.ignore_unknown_options = True

            self._callback = self.callback
            if self.callback:

                def task_callback(**command_args: Any):
                    return CommandTask(
                        name=self.name,
                        prerequisites=prerequisites,
                        execution_order=execution_order,
                        command_args=command_args,
                        callback=self._callback,
                    )

                self.callback = task_callback  #  Encapsulate the command into a CommandTask

        def invoke(self, ctx: click.core.Context):
            return super(CustomCommand, self).invoke(ctx)

    class Range(object):
        """
            The range of options.
            Return values:
            - global: It is used to define the command option, which can be used as a global option
            - shared: It is used to define a global range option that applies to all commands.
            - default: Only applies to the current option.
        """

        RANGES = ('global', 'shared', 'default')

        def __init__(self, range='default'):
            if isinstance(range, Range):
                range = str(range)
            if range not in self.RANGES:
                print("Error: Unknown {range}, please refer to Option class".format(range=range))
            self.range = range

        @property
        def is_global(self):
            return self.range == 'global'

        @property
        def is_shared(self):
            return self.range == 'shared'

        def __str__(self):
            return self.range

    class Option(click.Option):
        """
            Inherited from click.Option class
            Options for global and all commands
        """
        def __init__(self, range='default', **kwargs: str):
            kwargs['param_decls'] = kwargs.pop('names')  # 'param_decls' defined in click.Option
            super(Option, self).__init__(**kwargs)

            self.range = Range(range)

            if self.range.is_global:
                self.help = (self.help or '') + 'The option can only be defined once.'

    class customCLI(click.MultiCommand):
        """
            Inherited from click.MultiCommand class
            All commands with options are defined in param 'all_commands'.
        """
        def __init__(self, all_commands: Dict=None, help: str=None):
            super(customCLI, self).__init__(
                invoke_without_command=True,
                no_args_is_help=True,
                chain=True,
                result_callback=self.process_command_tasks,
                help=help,
            )
            self.commands = {}
            self.global_command_callbacks = []
            self.command = {}

            if all_commands is None:
                all_commands = {}

            shared_options = []

            # Global options
            for args in all_commands.get('global_options', []):
                option = Option(**args)
                self.params.append(option)

                if option.range.is_shared:
                    shared_options.append(option)

            # Global options validators
            self.global_command_callbacks = all_commands.get('global_command_callbacks', [])

            # Parse all commands
            for name, command in all_commands.get('commands', {}).items():
                options = command.pop('options', [])

                if options is None:
                    options = []

                self.commands[name] = CustomCommand(name=name, **command)

                for option in shared_options:
                    self.commands[name].params.append(option)

                for args in options:
                    option = Option(**args)
                    self.commands[name].params.append(option)

                    # Put global options to self.params
                    if option.range.is_global and option.name not in [option.name for option in self.params]:
                        self.params.append(option)

        def get_command(self, ctx: click.core.Context, name: str):
            return self.commands.get(name)

        def list_commands(self, ctx: click.core.Context):
            return sorted(self.commands)

        def resolve_command(self, ctx: click.core.Context, args: List[str]):
            import click.utils
            cmd_name = make_str(args[0])
            original_cmd_name = cmd_name

            # Get the command
            cmd = self.get_command(ctx, cmd_name)

            # If we can't find the command but there is a normalization
            # function available, we try with that one.
            if cmd is None and ctx.token_normalize_func is not None:
                cmd_name = ctx.token_normalize_func(cmd_name)
                cmd = self.get_command(ctx, cmd_name)

            # If we don't find the command we want to show an error message
            # to the user that it was not provided.  However, there is
            # something else we should do: if the first argument looks like
            # an option we want to kick off parsing again for arguments to
            # resolve things like --help which now should go to the main
            # place.
            if cmd is None and not ctx.resilient_parsing:
                if original_cmd_name.startswith('-'):
                    print("Error: No such argument {option}.".format(option=original_cmd_name))
            return cmd_name if cmd else None, cmd, args[1:]

        def invoke(self, ctx: click.core.Context):
            def _process_result(value):
                if self._result_callback is not None:
                    value = ctx.invoke(self._result_callback, value, **ctx.params)
                return value

            if not ctx.protected_args:
                if self.invoke_without_command:
                    # No subcommand was invoked, so the result callback is
                    # invoked with the group return value for regular
                    # groups, or an empty list for chained groups.
                    with ctx:
                        rv = super().invoke(ctx)
                        return _process_result([] if self.chain else rv)
                ctx.fail(_("Missing command."))

            # Fetch args back out
            args = [*ctx.protected_args, *ctx.args]
            ctx.args = []
            ctx.protected_args = []

            # If we're not in chain mode, we only allow the invocation of a
            # single command but we also inform the current context about the
            # name of the command to invoke.
            if not self.chain:
                # Make sure the context is entered so we do not clean up
                # resources until the result processor has worked.
                with ctx:
                    cmd_name, cmd, args = self.resolve_command(ctx, args)
                    assert cmd is not None
                    ctx.invoked_subcommand = cmd_name
                    super().invoke(ctx)
                    sub_ctx = cmd.make_context(cmd_name, args, parent=ctx)
                    with sub_ctx:
                        return _process_result(sub_ctx.command.invoke(sub_ctx))

            # In chain mode we create the contexts step by step, but after the
            # base command has been invoked.  Because at that point we do not
            # know the subcommands yet, the invoked subcommand attribute is
            # set to ``*`` to inform the command that subcommands are executed
            # but nothing else.
            with ctx:
                ctx.invoked_subcommand = "*" if args else None
                super().invoke(ctx)

                # Otherwise we make every single context and invoke them in a
                # chain.  In that case the return value to the result processor
                # is the list of all invoked subcommand's results.
                contexts = []
                while args:
                    cmd_name, cmd, args = self.resolve_command(ctx, args)
                    if cmd == None:
                        continue
                    sub_ctx = cmd.make_context(
                        cmd_name,
                        args,
                        parent=ctx,
                        allow_extra_args=True,
                        allow_interspersed_args=False,
                    )
                    contexts.append(sub_ctx)
                    args, sub_ctx.args = sub_ctx.args, []

                rv = []
                for sub_ctx in contexts:
                    with sub_ctx:
                        rv.append(sub_ctx.command.invoke(sub_ctx))
                return _process_result(rv)

        def process_command_tasks(self, command_tasks: List, **kwargs: str):
            ctx = click.get_current_context()
            global_args = PropertyDict(kwargs)

            for command_task in command_tasks:
                for name in list(command_task.command_args):
                    option = next((param for param in ctx.command.params if param.name == name), None)

                    # if command args conflict with global args, pop command args, use global args
                    if option and (option.range.is_global or option.range.is_shared):
                        command_task.command_args.pop(name)

            # process global command callback
            for action_callback in ctx.command.global_command_callbacks:
                action_callback(global_args)

            # setup ready_task to handle the order of commands
            ready_tasks: OrderedDict = OrderedDict()
            while command_tasks:
                command_task = command_tasks[0]
                cmd_tasks_dict = dict([(task.name, task) for task in command_tasks])

                prerequisites_processed = True

                # prerequisites must be processed before current cmd.
                for pre in command_task.prerequisites:
                    if pre not in ready_tasks.keys():
                        # If prerequisites is in command task list, it should be be put in front of the command task
                        if pre in cmd_tasks_dict.keys():
                            pre_task = command_tasks.pop(command_tasks.index(cmd_tasks_dict[pre]))
                        # else prerequisites must be put in front of the command task according to default option.
                        else:
                            print(
                                'According to default option, "%s" is added to the command task list before "%s".' %
                                (pre, command_task.name))
                            pre_task = ctx.invoke(ctx.command.get_command(ctx, pre))
                            # Remove options with global range from invoke command_tasks because they are already in global_args
                            for args in list(pre_task.command_args):
                                option = next((param for param in ctx.command.params if param.name == args), None)
                                if option and (option.range.is_global or option.range.is_shared):
                                    pre_task.command_args.pop(args)

                        command_tasks.insert(0, pre_task)
                        prerequisites_processed = False

                # commands in execution_order must be executed before the current task
                for pre in command_task.execution_order:
                    if pre in cmd_tasks_dict.keys() and pre not in ready_tasks.keys():
                        command_tasks.insert(0, command_tasks.pop(command_tasks.index(cmd_tasks_dict[pre])))
                        prerequisites_processed = False

                if prerequisites_processed:
                    command_tasks.pop(0)
                    # Add unprocessed command to the ready task list
                    if command_task.name not in ready_tasks.keys():
                        ready_tasks.update([(command_task.name, command_task)])

            # Execute all command_tasks in the ready task list
            for command_task in ready_tasks.values():
                command_task(global_args, command_task.command_args)

            return ready_tasks

    # That's a tiny parser that parse sdk-dir even before constructing
    # fully featured click parser to be sure that extensions are loaded from the right place
    @click.command(
        add_help_option=False,
        context_settings={
            'allow_extra_args': True,
            'ignore_unknown_options': True
        },
    )
    @click.option('-C', '--sdk-dir', default=os.getcwd(), type=click.Path())
    def parse_sdk_dir(sdk_dir: str):
        return os.path.realpath(sdk_dir)

    # Set `complete_var` to not existing environment variable name to prevent early cmd completion
    sdk_dir = parse_sdk_dir(standalone_mode=False, complete_var='_META_TOOL.PY_COMPLETE_NOT_EXISTING')

    all_commands: Dict = {}

    sys.path.append(os.path.realpath(__file__))
    extension =  import_module('commands')
    try:
        all_commands = extension.action_extensions(all_commands, sdk_dir)
    except AttributeError:
        print(f"Error: manage.py load commands configurations fail!")

    common_help = ('Realsil BT meta tool. ')

    return customCLI(help=common_help, all_commands=all_commands)

def main():
    cli = setup_cli()
    argv = sys.argv[1:]

    cli(argv, prog_name='manage.py', complete_var='_MANAGE.PY_COMPLETE')

if __name__ == "__main__":
    main()
