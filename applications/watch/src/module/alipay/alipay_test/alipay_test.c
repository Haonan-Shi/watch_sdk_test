/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*
 * Alipay transit-code shell commands for engineering tests.
 *
 * Enable condition: CONFIG_ALIPAY_TEST=y (depends on CONFIG_ALIPAY and
 * CONFIG_SHELL). Run `alipay <subcmd>` from the UART console while the watch
 * UI does not yet expose alipay entries, to drive bind / transit / paycode /
 * unbind flows.
 *
 * Subcommands:
 *   alipay status                  Print bind / network status (local cache).
 *   alipay net                     Print PAN status.
 *   alipay init                    alipay_init_transit_module + alipay_pre_init.
 *   alipay devname                 alipay_init_device_name (run after PAN ready).
 *   alipay time                    Print system / GMT time, check sync.
 *   alipay bind                    Get binding code synchronously and print.
 *   alipay bindstatus              ALIPAY_MSG_GET_BIND_STATUS (async refresh).
 *   alipay unbind                  ALIPAY_MSG_DEVICE_UNBIND (async).
 *   alipay list                    ALIPAY_MSG_GET_CARD_LIST_ONLINE.
 *   alipay transit [<index>]       No arg = default card; otherwise by index.
 *   alipay paycode                 ALIPAY_MSG_GET_PAYCODE.
 *   alipay update                  ALIPAY_MSG_TRANSIT_SLIENT_UPDATE.
 *   alipay mem                     Print heap stats (RTK multi-heap + alipay TLSF).
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <zephyr/shell/shell.h>

#include "alipay_task.h"
#include "alipay_pan.h"
#include "alipay_bind.h"
#include "alipay_common.h"
#include "alipay_account_manage.h"

#if CONFIG_ALIPAY_TRANSIT
#include "alipay_transit.h"
#endif

/* Some SDK APIs are present in the .a archive but not declared in any public
 * header; forward-declare them here, matching usage from demo_platform.c. */
EXTERNC int alipay_init_device_name(void);
EXTERNC unsigned int alipay_get_system_second(void);
EXTERNC retval_e alipay_get_binding_code(PARAM_OUT char *result, PARAM_INOUT int *len_result);

struct alipay_timeval
{
    long tv_sec;
    long tv_usec;
};
EXTERNC int alipay_iot_gettimeofday(struct alipay_timeval *tv);

static int send_msg(e_alipay_task_msg type, uint32_t data)
{
    T_ALIPAY_MSG msg = { .type = type, .u.data = data, .func = NULL };
    return alipay_send_msg_to_alipay_task(&msg) ? 0 : -1;
}

static int cmd_alipay_status(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    alipay_device_status_ *bind = alipay_task_get_bind_status();
    e_pan_status net = alipay_get_network_status();
    if (bind)
    {
        shell_print(sh, "bind: status=%d binded=%d triggle=%d",
                    bind->status, bind->binded, bind->triggle);
    }
    else
    {
        shell_print(sh, "bind: <null>");
    }
    shell_print(sh, "pan : %s",
                net == BT_STATUS_CONNECTED ? "CONNECTED" : "DISCONNECT");
    return 0;
}

static int cmd_alipay_net(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    e_pan_status net = alipay_get_network_status();
    shell_print(sh, "pan = %s",
                net == BT_STATUS_CONNECTED ? "CONNECTED" : "DISCONNECT");
    return 0;
}

static int cmd_alipay_init(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
#if CONFIG_ALIPAY_TRANSIT
    int r1 = alipay_init_transit_module();
    shell_print(sh, "alipay_init_transit_module = %d", r1);
#endif
    int r2 = alipay_pre_init();
    shell_print(sh, "alipay_pre_init             = %d", r2);
    shell_print(sh, "(skip alipay_init_device_name -- run `alipay devname` after PAN connected)");
    return 0;
}

static int cmd_alipay_time(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    /* alipay_get_system_second : local epoch seconds (the core porting hook).
     * alipay_iot_gettimeofday  : GMT seconds + microseconds. */
    unsigned int local_sec = alipay_get_system_second();
    struct alipay_timeval gmt = {0};
    int ret = alipay_iot_gettimeofday(&gmt);

    shell_print(sh, "alipay_get_system_second  = %u (local epoch sec)", local_sec);
    shell_print(sh, "alipay_iot_gettimeofday   = %ld.%06ld  (ret=%d, GMT)",
                gmt.tv_sec, gmt.tv_usec, ret);

    /* Sanity check: epoch < 2020-01-01 (= 1577836800) means time is not
     * synced; alipay TLS handshake will fail. */
    if (local_sec < 1577836800u)
    {
        shell_warn(sh, "time NOT synced (< 2020-01-01) -- alipay TLS handshake will fail");
        shell_warn(sh, "fix: sync time over BLE (e.g. WearSDK_2.5.5.apk) or your own time-sync tool");
    }
    else
    {
        shell_print(sh, "time looks valid");
    }
    return 0;
}

static int cmd_alipay_devname(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    /* alipay_init_device_name must run after PAN is up; otherwise it blocks
     * or fails. Internal stack usage is large -- CONFIG_SHELL_STACK_SIZE
     * must be at least 8KB. */
    e_pan_status net = alipay_get_network_status();
    if (net != BT_STATUS_CONNECTED)
    {
        shell_warn(sh, "PAN not connected -- alipay_init_device_name will likely fail");
    }
    int r = alipay_init_device_name();
    shell_print(sh, "alipay_init_device_name     = %d", r);
    return 0;
}

static int cmd_async_simple(const struct shell *sh, e_alipay_task_msg t, const char *label)
{
    int rc = send_msg(t, 0);
    if (rc != 0)
    {
        shell_error(sh, "%s: queue full or task not ready", label);
        return rc;
    }
    shell_print(sh, "%s queued -- see UART log for result", label);
    return 0;
}

static int cmd_alipay_bind(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    /* Synchronously call alipay_get_binding_code and print the result. The
     * SDK issues a network request internally; the shell blocks for several
     * seconds. Shell stack >= 16K is required. */
    char buf[256] = {0};
    int len = sizeof(buf);
    retval_e ret = alipay_get_binding_code(buf, &len);

    shell_print(sh, "alipay_get_binding_code: ret=%d, len=%d", ret, len);

    if (ret == 0 && len > 0)
    {
        /* binding code is a printable string; print directly. */
        shell_print(sh, "binding code: %.*s", len, buf);
        shell_print(sh,
                    "-> paste the string into any QR-code generator, then scan it with the Alipay app");
    }
    else
    {
        shell_warn(sh, "binding code not generated (ret=%d, len=%d)", ret, len);
        shell_warn(sh, "debug: run `alipay status` / `alipay bindstatus` to inspect server-side state;");
        shell_warn(sh,
                   "       if status=2 (START_BINDING), run `alipay unbind` or unbind from the phone first");
    }
    return 0;
}

static int cmd_alipay_bindstatus(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    return cmd_async_simple(sh, ALIPAY_MSG_GET_BIND_STATUS, "bindstatus");
}

static int cmd_alipay_unbind(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    return cmd_async_simple(sh, ALIPAY_MSG_DEVICE_UNBIND, "unbind");
}

static int cmd_alipay_paycode(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    return cmd_async_simple(sh, ALIPAY_MSG_GET_PAYCODE, "paycode");
}

static int cmd_alipay_update(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    return cmd_async_simple(sh, ALIPAY_MSG_TRANSIT_SLIENT_UPDATE, "update");
}

static int cmd_alipay_list(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    return cmd_async_simple(sh, ALIPAY_MSG_GET_CARD_LIST_ONLINE, "list");
}

/* Heap stats. Forward-declared instead of pulling extra headers:
 *   - os_mem_peek_printf() lives in osif_zephyr.c, prototype in os_mem.h
 *   - csi_heap_stats_print() is defined in alipay_mem.c next to the
 *     alipay TLSF instance (file-static), exposed only via this extern. */
extern void os_mem_peek_printf(void);
extern void csi_heap_stats_print(void);

static int cmd_alipay_mem(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    /* RTK multi-heap (heap_sram + DTCM/ITCM/BUFFER_ON), free bytes per ram_type. */
    os_mem_peek_printf();
    /* alipay TLSF pool (csi_malloc, psram1_for_alipay 200K), walked totals. */
    csi_heap_stats_print();
    shell_print(sh, "see UART log for [mem] lines");
    return 0;
}

static int cmd_alipay_transit(const struct shell *sh, size_t argc, char **argv)
{
    if (argc < 2)
    {
        return cmd_async_simple(sh, ALIPAY_MSG_GET_DEFAULT_TRANSIT_CODE,
                                "transit (default)");
    }

    char *end = NULL;
    unsigned long idx = strtoul(argv[1], &end, 10);
    if (end == argv[1])
    {
        shell_error(sh, "usage: alipay transit [<index>]");
        return -1;
    }

    /* Synchronously prime p_transit_card[] from the local cache before
     * posting the async pair, matching the GUI flow. Without this, the
     * GET_TRANSIT_CODE handler would index into csi_malloc'd PSRAM that
     * has not yet been written by any handler. */
    uint8_t list_ret = alipay_task_check_local_card_list_exist();
    if (list_ret != 0)
    {
        shell_error(sh,
                    "transit: local card list not ready (ret=%u). Run `alipay update` first.",
                    list_ret);
        return -1;
    }

    int rc_a = send_msg(ALIPAY_MSG_GET_DEFAULT_TRANSIT_CODE, 0);
    if (rc_a != 0)
    {
        shell_error(sh, "transit: queue full sending GET_DEFAULT_TRANSIT_CODE");
        return rc_a;
    }

    int rc = send_msg(ALIPAY_MSG_GET_TRANSIT_CODE, (uint32_t)idx);
    if (rc != 0)
    {
        shell_error(sh, "transit: queue full or task not ready");
        return rc;
    }
    shell_print(sh,
                "transit idx=%lu queued (list refreshed, paired with default) -- see UART log",
                idx);
    return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(alipay_subcmds,
                               SHELL_CMD(status,     NULL, "Print bind / network status",
                                         cmd_alipay_status),
                               SHELL_CMD(net,        NULL, "Print PAN network status",
                                         cmd_alipay_net),
                               SHELL_CMD(init,       NULL,
                                         "Init: transit_module + pre_init (skip device_name)",
                                         cmd_alipay_init),
                               SHELL_CMD(devname,    NULL,
                                         "alipay_init_device_name -- run after PAN connected",
                                         cmd_alipay_devname),
                               SHELL_CMD(time,       NULL,
                                         "Print alipay system / GMT time, check if synced",
                                         cmd_alipay_time),
                               SHELL_CMD(bind,       NULL,
                                         "Get binding code (sync, prints code text)",
                                         cmd_alipay_bind),
                               SHELL_CMD(bindstatus, NULL, "Refresh bind status (async)",
                                         cmd_alipay_bindstatus),
                               SHELL_CMD(unbind,     NULL, "Unbind device (async)",
                                         cmd_alipay_unbind),
                               SHELL_CMD(list,       NULL, "Get card list (online)",
                                         cmd_alipay_list),
                               SHELL_CMD(transit,    NULL,
                                         "Get transit code [<index>] (no arg = default card)",
                                         cmd_alipay_transit),
                               SHELL_CMD(paycode,    NULL, "Get pay code (async)",
                                         cmd_alipay_paycode),
                               SHELL_CMD(update,     NULL, "Trigger silent update (async)",
                                         cmd_alipay_update),
                               SHELL_CMD(mem,        NULL,
                                         "Print heap stats (RTK multi-heap + alipay TLSF)",
                                         cmd_alipay_mem),
                               SHELL_SUBCMD_SET_END
                              );

SHELL_CMD_REGISTER(alipay, &alipay_subcmds,
                   "Alipay engineering test commands (CONFIG_ALIPAY_TEST)", NULL);
