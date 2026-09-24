/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*
 * PSRAM usage sample. Demonstrates four ways to use the on-package psram:
 *   [1] static/global variables placed in a psram memory-region
 *   [2] psram as a heap (PSRAM0 + PSRAM1) via os_mem_alloc()
 *   [3] psram as a thread stack (dynamically created task, with a dynamic or a
 *       static psram stack)
 *   [4] cacheable psram access (cache/MPU set up in app_lower_init.c)
 *
 * All demos run once at boot and print a PASS/FAIL summary. When the shell is
 * enabled they can be re-run individually with e.g. `psram heap`, `psram all`.
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>

#include "app_lower_init.h"
#include "os_mem.h"
#include "osif_zephyr.h"
#include "psram_demo.h"

static int run_all(void)
{
    int fail = 0;

    fail += psram_static_var_demo();
    fail += psram_heap_demo();
    fail += psram_thread_stack_demo();
    fail += psram_cache_demo();

    printf("\n==== PSRAM sample summary: %s (%d demo group(s) failed) ====\n",
           fail ? "FAIL" : "ALL PASS", fail);
    return fail;
}

int main(void)
{
    printf("Hello World! %s\n", CONFIG_BOARD_TARGET);

    app_system_lower_init();
    psram_heap_init();

    run_all();

    return 0;
}

#ifdef CONFIG_SHELL
static int cmd_all(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(sh); ARG_UNUSED(argc); ARG_UNUSED(argv);
    run_all();
    return 0;
}

static int cmd_static(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(sh); ARG_UNUSED(argc); ARG_UNUSED(argv);
    psram_static_var_demo();
    return 0;
}

static int cmd_heap(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(sh); ARG_UNUSED(argc); ARG_UNUSED(argv);
    psram_heap_demo();
    return 0;
}

static int cmd_stack(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(sh); ARG_UNUSED(argc); ARG_UNUSED(argv);
    psram_thread_stack_demo();
    return 0;
}

static int cmd_cache(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(sh); ARG_UNUSED(argc); ARG_UNUSED(argv);
    psram_cache_demo();
    return 0;
}

static int cmd_peek(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(sh); ARG_UNUSED(argc); ARG_UNUSED(argv);
    psram_heap_peek();
    return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(psram_cmds,
                               SHELL_CMD(all,    NULL, "run all psram demos",         cmd_all),
                               SHELL_CMD(static, NULL, "psram static-variable demo",  cmd_static),
                               SHELL_CMD(heap,   NULL, "psram heap demo",             cmd_heap),
                               SHELL_CMD(stack,  NULL, "psram thread-stack demo",     cmd_stack),
                               SHELL_CMD(cache,  NULL, "psram cache demo",            cmd_cache),
                               SHELL_CMD(peek,   NULL, "psram heap usage peek",       cmd_peek),
                               SHELL_SUBCMD_SET_END
                              );
SHELL_CMD_REGISTER(psram, &psram_cmds, "PSRAM usage demos", NULL);
#endif /* CONFIG_SHELL */
