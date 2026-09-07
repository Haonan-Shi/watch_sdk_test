import json
import os
from typing import Any, Dict
import click
import subprocess
import shutil
import uuid
import sys
import struct
import re
import venv

BAUDRATE = {
    'names': ['-b', '--baudrate'],
    'help': 'Modified baudrate for flash operation.',
    'range': 'global',
    'default': 1000000,
}

COM_PORT = {
    'names': ['-c', '--com'],
    'help': 'Com port.',
    'range': 'global',
    'default': None,
}

FLASH_OPERATION = [BAUDRATE, COM_PORT]
ITEM_IMG_BASE = 0x001C
SDK_VENV_DIR = '.venv'

build_image_dir = r""


class PropertyDict(dict):
    def __getattr__(self, name: str):
        if name in self:
            return self[name]
        else:
            raise AttributeError("'PropertyDict' object has no attribute '%s'" % name)

    def __setattr__(self, name: str, value: Any):
        self[name] = value

    def __delattr__(self, name: str):
        if name in self:
            del self[name]
        else:
            raise AttributeError("'PropertyDict' object has no attribute '%s'" % name)


def merge_commands(*command_list: Dict):
    merged_actions: Dict = {
        'global_options': [],
        'commands': {},
        'global_command_callbacks': [],
    }
    for command in command_list:
        merged_actions['global_options'].extend(command.get('global_options', []))
        merged_actions['commands'].update(command.get('commands', {}))
        merged_actions['global_command_callbacks'].extend(command.get('global_command_callbacks', []))
    return merged_actions


def get_ic_type(ic_type):
    if ic_type == "RTL87X3EP":
        return "8773E"
    elif ic_type == "RTL87X3E":
        return "8763E"
    elif ic_type == "RTL87X3D":
        return "8773D"
    elif ic_type == "RTL87X3G":
        return "8773G"


def check_venv():
    print("Check Python virtual environment...")
    if os.path.exists(SDK_VENV_DIR):
        print("Python virtual environment exists")
    else:
        # Create virtual environment if it does not exist
        try:
            print("Python virtual environment does not exist")
            builder = venv.EnvBuilder(with_pip=True)
            builder.create(SDK_VENV_DIR)
            print("Python virtual environment created")
        except OSError as e:
            print(f"Error: Fail to create Python virtual environment: {e}")
            sys.exit(2)


def get_venv_commands():
    cmd_activate = None
    cmd_deactivate = None

    if os.name == 'nt':  # Windows
        cmd_activate = os.path.join(SDK_VENV_DIR, 'Scripts', 'activate.bat')
        cmd_deactivate = os.path.join(SDK_VENV_DIR, 'Scripts', 'deactivate.bat')
    else:  # Unix/Linux/MacOS
        cmd_activate = 'source ' + os.path.join(SDK_VENV_DIR, 'bin', 'activate')
        cmd_deactivate = 'deactivate'

    return cmd_activate, cmd_deactivate


def execute_command(args, env, path):
    str_args = ' '.join(str(arg) for arg in args)
    print("Execute command: \n" + str_args)
    check_venv()
    cmd_activate_venv, cmd_deactivate_venv = get_venv_commands()

    # If a parameter contains spaces, wrap it in double quotes; otherwise,
    # some commands may not be recognized correctly.
    cur_cmd = ' '.join([
        f'"{arg}"' if ' ' in arg else arg
        for arg in args
    ])
    if path is None:
        cmd = f"{cmd_activate_venv} && {cur_cmd} && {cmd_deactivate_venv}"
    else:
        ori_dir = os.getcwd()
        cmd = f"{cmd_activate_venv} && cd {path} && {cur_cmd} && cd {ori_dir} && {cmd_deactivate_venv}"

    try:
        subprocess.run(cmd, shell=True, env=env, text=True, check=True)
    except subprocess.CalledProcessError as e:
        print(f"An error occurred: {e}")
        assert not e

    finally:
        try:
            subprocess.run('.venv\\Scripts\\deactivate.bat', shell=True, check=True)
        except Exception as e:
            print("Failed to deactivate virtual environment:", str(e))


def action_extensions(base_actions: Dict, project_path: str):
    def construct_target(args: PropertyDict, kconfig: str, is_check_flow: str, compile_lib_only: str):
        global build_image_dir

        if kconfig is None:
            print(f"Error: Missing command arguments -k!")
            sys.exit(2)

        build_project_path = os.path.dirname(kconfig)
        build_image_dir = os.path.join(args.sdk_dir, build_project_path)
        build_image_dir = os.path.normpath(build_image_dir)

    def build_target(args: PropertyDict, project_id: str):
        def parse_bin_file(directory, pattern):
            for root, dirs, files in os.walk(directory):
                for file in files:
                    if pattern in file and file.endswith('.bin'):
                        return os.path.join(root, file)
            return None

        def parse_bin_start_addr(file_path):
            with open(file_path, 'rb') as f:
                while True:
                    type_bytes = f.read(2)
                    if len(type_bytes) < 2:
                        break
                    data_type = struct.unpack('<H', type_bytes)[0]

                    length_byte = f.read(1)
                    if len(length_byte) < 1:
                        break
                    data_length = struct.unpack('B', length_byte)[0]

                    data_content = f.read(data_length)
                    if len(data_content) < data_length:
                        break

                    if data_type == ITEM_IMG_BASE:
                        return hex(struct.unpack('<I', data_content)[0])

            return None

        def update_build_info(addr, image, elf):
            file_path = os.path.join(args.sdk_dir, 'tools/meta_tools/extension_info/build_info.json')
            file_path = os.path.normpath(file_path)
            os.makedirs(os.path.dirname(file_path), exist_ok=True)
            if os.path.exists(file_path):
                with open(file_path, "r") as json_file:
                    try:
                        info = json.load(json_file)
                    except json.JSONDecodeError:
                        info = {}
            else:
                info = {}

            info[project_id] = {
                "start_addr": addr,
                "image_path": image,
                "elf_path": elf
            }
            with open(file_path, "w") as json_file:
                json.dump(info, json_file, indent=4)

        global build_image_dir
        cmake_args = [
            'west',
            'build',
            '-b', 'rtl87x3g_evb'
        ]
        execute_command(cmake_args, None, build_image_dir)
#        after_build = [
#            'post_build.bat'
#        ]
#        post_build_path = os.path.abspath(os.path.join(build_image_dir, "..", "..", "tools", "PostBuild"))
#        execute_command(after_build, None, post_build_path)
        image_dir = os.path.join(build_image_dir, "bin")
        image_dir = os.path.normpath(image_dir)
        image_path = parse_bin_file(image_dir, 'MP')
        if image_path is None:
            print(f"Error: Missing the bin file.")
            sys.exit(2)
        elf_path = os.path.join(build_image_dir, "build", "zephyr", "zephyr.elf")
        if elf_path is None:
            print(f"Error: Missing the elf file.")
            sys.exit(2)
        start_addr = "0x7009E000"
        # start_addr = parse_bin_start_addr(image_path)
        # if start_addr is None:
        #     print("Error: Can not parse the start address, please check image base.")
        #     sys.exit(2)

        update_build_info(start_addr, image_path, elf_path)

    def config(args: PropertyDict, ic_type, project_dir):
        if ic_type is None:
            print("Error: Missing command arguments -T!")
            sys.exit(2)
        if project_dir is None:
            print("Error: Missing command arguments -T!")
            sys.exit(2)
        file_path = os.path.join(project_dir, 'Kconfig')
        if not os.path.exists(file_path):
            print("Error: Missing Kconfig file in project directory!")
            sys.exit(2)
        full_ic_type = "RTL" + get_ic_type(ic_type)
        env = os.environ.copy()
        env["BOARD_DIR"] = full_ic_type
        guiconfig_args = [
            'guiconfig',
            file_path
        ]
        execute_command(guiconfig_args, env, None)

    def clean(args: PropertyDict):
        if not os.path.isdir(args.build_dir):
            print("Error: Build directory '%s' not found. Nothing to clean." % args.build_dir)
            return
        clean_args = [
            'make',
            'clean'
        ]
        execute_command(clean_args, None, args.build_dir)

    def fullclean(args: PropertyDict):
        if not os.path.isdir(args.build_dir):
            print("Error: Build directory '%s' not found. Nothing to clean." % args.build_dir)
            return

        try:
            shutil.rmtree(args.build_dir)
            print(f"Successfully clean the build directory '{args.build_dir}'")
        except Exception as e:
            print(f"Error: cleaning the build directory: {e}")

    def flash_download(args: PropertyDict, json_path: str, file_path: str, addr: str, ic_type: str):
        """
        Run esptool to flash the entire project, from an argfile generated by the build system
        """
        if ic_type is None:
            print("Error: Missing command arguments -T!")
            sys.exit(2)
        if json_path is None and file_path is None:
            print("Error: Missing command arguments -f or -F!")
            sys.exit(2)
        if file_path:
            if addr is None:
                print("Error: Missing command arguments -A for -F!")
                sys.exit(2)
        if json_path is not None:
            full_path = os.path.join(args.sdk_dir, 'tool', 'meta_tools', 'extension_info', json_path)
            download_args = [
                'mpcli.exe',
                '-c', args.com,
                '-a', '-f', full_path,
                '-b', str(args.baudrate),
                '-M', '5',
                '-r',
                '-T', ic_type
            ]
        else:
            download_args = [
                'mpcli.exe',
                '-c', args.com,
                '-p', '-A', addr,  '-F', file_path,
                '-b', str(args.baudrate),
                '-M', '5',
                '-r',
                '-T', ic_type
            ]
        base_dir = os.path.join(args.sdk_dir, "tools", "meta_tools")
        execute_command(download_args, None, base_dir)

    def flash_erase(args: PropertyDict, size: str, addr: str, ic_type: str):
        if size is None:
            print("Error: Missing command arguments -S!")
            sys.exit(2)
        if addr is None:
            print("Error: Missing command arguments -A!")
            sys.exit(2)
        erase_args = [
            'mpcli.exe',
            '-c', args.com,
            '-e', '-A', addr, '-S', size,
            '-b', str(args.baudrate),
            '-M', '5',
            '-r',
            '-T', ic_type
        ]
        base_dir = os.path.join(args.sdk_dir, "tools", "meta_tools")
        execute_command(erase_args, None, base_dir)

    def flash_chip_erase(args: PropertyDict, ic_type: str):
        erase_args = [
            'mpcli.exe',
            '-c', args.com,
            '--chip_erase',
            '-M', '5',
            '-r',
            '-T', ic_type
        ]
        base_dir = os.path.join(args.sdk_dir, "tools", "meta_tools")
        execute_command(erase_args, None, base_dir)

    def query(args: PropertyDict, info, application, device):
        def query_info(cfg):
            repo_info = cfg['info']
            info = dict()
            info['uuid'] = str(uuid.uuid4())
            info['brief'] = repo_info['brief']
            info['details'] = None
            info['soc'] = repo_info['soc']
            info['os'] = repo_info['os']
            info['server'] = None
            info['vcs'] = repo_info['vcs']
            info['metaTool'] = repo_info['metaTool']
            info['metaToolPath'] = os.path.join('tools', 'meta_tools', repo_info['metaTool'])
            info['url'] = None
            info['target'] = os.path.join('zephyr', 'zephyr.elf')
            info['repoUrl'] = None
            info['toolchainUrl'] = None
            info['manifest'] = None
            info['revision'] = None
            info['worktree'] = False
            info['version'] = cfg['version']
            print(json.dumps(info, indent=4))

        def query_app(cfg):
            app_dir = os.path.join(args.sdk_dir, 'applications')
            folders = [d for d in os.listdir(app_dir) if os.path.isdir(os.path.join(app_dir, d))]
            app = {
                "apps": [
                    {
                        "applications": [{folder: folder} for folder in folders]
                    }
                ]
            }
            json_string = json.dumps(app, separators=(',', ':'))
            print(json_string)

        def query_device(cfg):
            ret = json.dumps(cfg['devices'], indent=4)
            print(ret)

        file_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'query.json')
        if os.path.exists(file_path):
            try:
                with open(file_path, 'r') as file:
                    cfg = json.load(file)
            except:
                raise RuntimeError('Error: Fail to load query configuration file "' + file_path + '"')
        else:
            raise RuntimeError('Error: Query configuration file "' + file_path + '" does not exist')

        if info:
            query_info(cfg)
        elif application:
            query_app(cfg)
        elif device:
            query_device(cfg)

    def setup(args: PropertyDict, pristine):
        print("Set up...")
        if pristine:
            print("Clean workspace...")
            os.system("git reset --hard && git clean -fd")
            print("Clean workspace done")
        else:
            pass

#        print("Update workspace...")
#        os.system('west update')
#        print("Update workspace done")

        zephyr_setup_args = [
            'pip',
            'install',
            '-r', '../zephyr/scripts/requirements.txt'
        ]
        execute_command(zephyr_setup_args, None, None)

        setup_args = [
            'pip',
            'install',
            '-r', 'tools/meta_tools/requirements.txt'
        ]
        execute_command(setup_args, None, None)
        print("Set up SDK done")

    def update(args: PropertyDict, pristine):
        if pristine:
            print("Clean workspace...")
            os.system("west forall -c 'git reset --hard && git clean -fd'")
            print("Clean workspace done")
        else:
            pass

        print("Update manifest...")
        os.system("cd .. && cd .. && cd manifest && git pull && cd -")
        print("Update manifest done")
        print("Update workspace...")
        os.system('west update')
        print("Update workspace done")

    def check_root_options(args: PropertyDict):
        args.sdk_dir = os.path.realpath(args.sdk_dir)
        if args.build_dir is None:
            args.build_dir = os.path.join(args.sdk_dir, 'build')
        args.build_dir = os.path.realpath(args.build_dir)

    root_options = {
        'global_options': [
            {
                'names': ['-C', '--sdk-dir'],
                'range': 'shared',
                'help': 'Project directory.',
                'type': click.Path(),
                'default': os.getcwd(),
            },
            {
                'names': ['-B', '--build-dir'],
                'help': 'Build directory.',
                'type': click.Path(),
                'default': None,
            },
        ],
        'global_command_callbacks': [check_root_options],
    }

    build_actions = {
        'commands': {
                'construct': {
                    'callback': construct_target,
                    'short_help': 'Construct the project.',
                    'help': (
                        'Construct the project. \n\n'
                        'Create the build directory if necessary. '
                        'The build directory is by default created in the SDK directory. \n\n '),
                    'options': [
                        {
                            'names': ['-k', '--kconfig'],
                            'help': 'Add kconfig file.',
                            'type': click.Path(),
                        },
                        {
                            'names': ['-i', '--is-check-flow'],
                            'help': 'Check whether it is release check flow.',
                            'default': 'OFF',
                        },
                        {
                            'names': ['-c', '--compile-lib-only'],
                            'help': 'Only compile lib.',
                            'default': 'OFF',
                        },
                    ],
                    'execution_order': [
                        # 'menuconfig',
                        'fullclean',
                    ],
                },
                'build': {
                    'callback': build_target,
                    'short_help': 'Build the project.',
                    'help': (
                        'Build the project. '
                        'Build the target according to the selected kconfig in the construct.'),
                    'options': [
                        # {
                        #     'names': ['-i', '--image-dir'],
                        #     'help': 'Add kconfig file.',
                        #     'type': click.Path(),
                        #     'default': None,
                        # },
                        {
                            'names': ['-p', '--project-id'],
                            'help': 'Add project id.',
                            'default': '1',
                        },
                    ],
                    'execution_order': [
                        'construct',
                    ],
                },
                'config': {
                    'callback': config,
                    'short_help': 'Run "guiconfig" project configuration tool.',
                    'help': (
                        'Configure the project. '
                        'Run "guiconfig" project configuration tool.'),
                    'options': [
                        {
                            'names': ['-T', '--ic-type'],
                            'help': 'IC type.',
                            'default': None,
                        },
                        {
                            'names': ['-p', '--project-dir'],
                            'help': 'Project directory.',
                            'type': click.Path(),
                            'default': os.getcwd(),
                        },
                    ]
                }
        },
    }

    flash_actions = {
        'commands': {
                'download': {
                    'callback': flash_download,
                    'help': 'Download images to the EVB.',
                    'options': FLASH_OPERATION + [
                        {
                            'names': ['-f', '--json-path'],
                            'help': 'Specify the json file for download images.',
                            'default': None,
                        },
                        {
                            'names': ['-F', '--file-path'],
                            'type': click.Path(),
                            'help': 'Specify the file path.',
                            'default': None,
                        },
                        {
                            'names': ['-A', '--addr'],
                            'help': 'Flash address.',
                            'default': None,
                        },
                        {
                            'names': ['-T', '--ic-type'],
                            'help': 'IC type.',
                            'default': None,
                        },
                    ],
                    'execution_order': ['erase-flash'],
                },
                'erase': {
                    'callback': flash_erase,
                    'help': 'Erase data on the flash.',
                    'options': FLASH_OPERATION + [
                        {
                            'names': ['-S', '--size'],
                            'help': 'Flash space size.',
                            'default': None,
                        },
                        {
                            'names': ['-A', '--addr'],
                            'help': 'Flash address.',
                            'default': None,
                        },
                        {
                            'names': ['-T', '--ic-type'],
                            'help': 'IC type.',
                            'default': None,
                        },
                    ],
                },
                'chip-erase': {
                    'callback': flash_chip_erase,
                    'help': 'Erase all data on the flash.',
                    'options': FLASH_OPERATION + [
                        {
                            'names': ['-T', '--ic-type'],
                            'help': 'IC type.',
                            'default': None,
                        },
                    ],
            },
        }
    }

    clean_actions = {
        'commands': {
            'clean': {
                'callback': clean,
                'short_help': 'Delete the compilation-generated files in the build directory.',
                'help': (
                    'Delete the compilation-generated files in the build directory, '
                    'retaining just files related to CMakeFiles.'),
                'execution_order': ['fullclean'],
            },
            'fullclean': {
                'callback': fullclean,
                'help': 'Delete the build directory.',
            },
        }
    }

    query_actions = {
        'commands': {
            'query': {
                'callback': query,
                'short_help': 'Query SDK information',
                'help': (
                    """Query SDK information description
                    """),
                'options': [
                        {
                            'names': ['-i', '--info'],
                            'help': 'The SDK infomation.',
                            'range': 'default',
                            'is_flag': True,
                            'default': None,
                        },
                        {
                            'names': ['-a', '--application'],
                            'help': 'The application infomation.',
                            'range': "default",
                            'is_flag': True,
                            'default': None,
                        },
                        {
                            'names': ['-d', '--device'],
                            'help': 'device info.',
                            'range': "default",
                            'is_flag': True,
                            'default': None,
                        },
                ],
                'execution_order': [],
            }
        }
    }

    setup_actions = {
        'commands': {
            'setup': {
                'callback': setup,
                'short_help': 'Setup SDK information',
                'help': (
                    """Setup SDK information description
                    """),
                'options': [
                        {
                            'names': ['-p', '--pristine'],
                            'help': 'Clean workspace.',
                            'range': 'default',
                            'is_flag': True,
                            'default': None,
                        }
                ],
                'execution_order': [],
            }
        }
    }

    update_actions = {
        'commands': {
            'update': {
                'callback': update,
                'short_help': 'update SDK',
                'help': (
                    """Update the current SDK to the latest commit.
                    """),
                'options': [
                    {
                        'names': ['-p', '--pristine'],
                        'help': 'Clean workspace.',
                        'range': 'default',
                        'is_flag': True,
                        'default': None,
                    }
                ],
                'execution_order': [],
            }
        }
    }

    return merge_commands(root_options, build_actions, flash_actions, clean_actions, 
                          query_actions, setup_actions, update_actions)
