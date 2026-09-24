/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <zephyr/shell/shell.h>
#include "board.h"
#include "power_test.h"
#include "os_msg.h"
#include "app_msg.h"
#include "bt_types.h"
#include "os_mem.h"
#include "app_io_msg.h"
#include "pm.h"
#include "trace.h"

extern void *audio_io_queue_handle;
extern void *audio_evt_queue_handle;

/* gap_le / flash sub-command handlers live in their own files */
extern int cmd_gap_le(const struct shell *sh, size_t argc, char **argv);
extern int cmd_flash(const struct shell *sh, size_t argc, char **argv);

bool power_test_send_msg(T_IO_CONSOLE subtype, void *param_buf)
{
    T_IO_MSG        msg;

    msg.type    = IO_MSG_TYPE_CONSOLE;
    msg.subtype = subtype;
    msg.u.buf   = param_buf;

    return app_io_msg_send(&msg);
}

/* "power_test 32k on|off" */
static int cmd_32k(const struct shell *sh, size_t argc, char **argv)
{
    uint16_t    action = 0;
    void       *param_buf;

    if (!strcmp(argv[1], "on"))
    {
        action = POWER_TEST_CMD_32K_ON;
    }
    else if (!strcmp(argv[1], "off"))
    {
        action = POWER_TEST_CMD_32K_OFF;
    }
    else
    {
        shell_error(sh, "Invalid param %s (on or off).", argv[1]);
        return -EINVAL;
    }

    param_buf = malloc(4);
    if (param_buf != NULL)
    {
        uint8_t *p = param_buf;

        LE_UINT16_TO_STREAM(p, CLOCK_32K_ID);
        LE_UINT16_TO_STREAM(p, action);

        power_test_send_msg(IO_MSG_CONSOLE_STRING_RX, param_buf);
    }

    shell_print(sh, "Power test 32k %s.", argv[1]);
    return 0;
}

/* "power_test packet_rx <packet_type>" */
static int cmd_packet_rx(const struct shell *sh, size_t argc, char **argv)
{
    uint8_t     packet_type;
    void       *param_buf;

    packet_type = (uint8_t)strtol(argv[1], NULL, 0);

    param_buf = os_mem_alloc(OS_MEM_TYPE_DATA, 100);
    if (param_buf != NULL)
    {
        uint8_t *p = param_buf;

        LE_UINT16_TO_STREAM(p, PACKET_RX_ID);
        LE_UINT8_TO_STREAM(p, packet_type);

        power_test_send_msg(IO_MSG_CONSOLE_STRING_RX, param_buf);
    }

    shell_print(sh, "Power test packet_rx %s.", argv[1]);
    return 0;
}

/* "power_test cont_tx <tx_power> <packet_type>" */
static int cmd_cont_tx(const struct shell *sh, size_t argc, char **argv)
{
    uint8_t     tx_power, packet_type;
    void       *param_buf;

    tx_power    = (uint8_t)strtol(argv[1], NULL, 0);
    packet_type = (uint8_t)strtol(argv[2], NULL, 0);

    param_buf = os_mem_alloc(OS_MEM_TYPE_DATA, 100);
    if (param_buf != NULL)
    {
        uint8_t *p = param_buf;

        LE_UINT16_TO_STREAM(p, CONT_TX_ID);
        LE_UINT8_TO_STREAM(p, tx_power);
        LE_UINT8_TO_STREAM(p, packet_type);

        power_test_send_msg(IO_MSG_CONSOLE_STRING_RX, param_buf);
    }

    shell_print(sh, "Power test cont_tx %s %s.", argv[1], argv[2]);
    return 0;
}


/* "power_test gap_legacy <sub-action> [params...] <addr0..addr5>" */
static int cmd_gap_legacy(const struct shell *sh, size_t argc, char **argv)
{
    uint8_t     action;
    void       *param_buf;
    uint8_t     radio_mode = 0;
    uint8_t     link_policy = 0;
    uint8_t     type = 0;
    uint16_t    interval = 0;
    uint16_t    window = 0;
    uint8_t     inquiry_timeout = 0;
    uint16_t    page_timeout = 0;
    uint16_t    min_interval = 0;
    uint16_t    max_interval = 0;
    uint16_t    sniff_attempt = 0;
    uint16_t    sniff_timeout = 0;
    uint8_t     addr[6];
    uint8_t     i;
    int         param_num = argc - 1;   /* params after "gap_legacy" (sub-action + args + 6 addr) */
    int         addr_idx  = argc - 6;   /* bd_addr is always the trailing 6 arguments */

    if (param_num < 1)
    {
        goto err;
    }

    if (!strcmp(argv[1], "inquiry_scan_param_set"))
    {
        if (param_num != 10)
        {
            goto err;
        }
        type     = (uint8_t)strtol(argv[2], NULL, 0);
        interval = (uint16_t)strtol(argv[3], NULL, 0);
        window   = (uint16_t)strtol(argv[4], NULL, 0);
        action   = POWER_TEST_CMD_INQUIRY_SCAN_PARAM_SET;
    }
    else if (!strcmp(argv[1], "page_scan_param_set"))
    {
        if (param_num != 11)
        {
            goto err;
        }
        type         = (uint8_t)strtol(argv[2], NULL, 0);
        interval     = (uint16_t)strtol(argv[3], NULL, 0);
        window       = (uint16_t)strtol(argv[4], NULL, 0);
        page_timeout = (uint16_t)strtol(argv[5], NULL, 0);
        action       = POWER_TEST_CMD_PAGE_SCAN_PARAM_SET;
    }
    else if (!strcmp(argv[1], "radio_mode_set"))
    {
        if (param_num != 8)
        {
            goto err;
        }
        radio_mode = (uint8_t)strtol(argv[2], NULL, 0);
        action     = POWER_TEST_CMD_RADIO_MODE_SET;
    }
    else if (!strcmp(argv[1], "sniff_enter"))
    {
        if (param_num != 11)
        {
            goto err;
        }
        min_interval  = (uint16_t)strtol(argv[2], NULL, 0);
        max_interval  = (uint16_t)strtol(argv[3], NULL, 0);
        sniff_attempt = (uint16_t)strtol(argv[4], NULL, 0);
        sniff_timeout = (uint16_t)strtol(argv[5], NULL, 0);
        action        = POWER_TEST_CMD_SNIFF_ENTER;
    }
    else if (!strcmp(argv[1], "sniff_exit"))
    {
        if (param_num != 7)
        {
            goto err;
        }
        action = POWER_TEST_CMD_SNIFF_EXIT;
    }
    else if (!strcmp(argv[1], "default_link_policy_set"))
    {
        if (param_num != 8)
        {
            goto err;
        }
        link_policy = (uint8_t)strtol(argv[2], NULL, 0);
        action      = POWER_TEST_CMD_LINK_DEAULT_POLICY_SET;
    }
    else if (!strcmp(argv[1], "link_policy_set"))
    {
        if (param_num != 8)
        {
            goto err;
        }
        link_policy = (uint8_t)strtol(argv[2], NULL, 0);
        action      = POWER_TEST_CMD_LINK_POLICY_SET;
    }
    else if (!strcmp(argv[1], "inquiry_start"))
    {
        if (param_num != 8)
        {
            goto err;
        }
        inquiry_timeout = (uint8_t)strtol(argv[2], NULL, 0);
        action          = POWER_TEST_CMD_INQUIRY_START;
    }
    else if (!strcmp(argv[1], "inquiry_stop"))
    {
        if (param_num != 7)
        {
            goto err;
        }
        action = POWER_TEST_CMD_INQUIRY_STOP;
    }
    else if (!strcmp(argv[1], "page_start"))
    {
        if (param_num != 7)
        {
            goto err;
        }
        action = POWER_TEST_CMD_PAGE_START;
    }
    else if (!strcmp(argv[1], "page_stop"))
    {
        if (param_num != 7)
        {
            goto err;
        }
        action = POWER_TEST_CMD_PAGE_STOP;
    }
    else if (!strcmp(argv[1], "hfp_ag_connect"))
    {
        if (param_num != 7)
        {
            goto err;
        }
        action = POWER_TEST_CMD_HFP_AG_CONN;
    }
    else if (!strcmp(argv[1], "hfp_ag_disconnect"))
    {
        if (param_num != 7)
        {
            goto err;
        }
        action = POWER_TEST_CMD_HFP_AG_DISCON;
    }
    else if (!strcmp(argv[1], "legacy_disconnect"))
    {
        if (param_num != 7)
        {
            goto err;
        }
        action = POWER_TEST_CMD_LEGACY_DISCONNECT;
    }
    else if (!strcmp(argv[1], "remove_bond"))
    {
        if (param_num != 7)
        {
            goto err;
        }
        action = POWER_TEST_CMD_REMOVE_BOND;
    }
    else
    {
        goto err;
    }

    for (i = 0; i < 6; i++)
    {
        addr[i] = (uint8_t)strtol(argv[addr_idx + i], NULL, 0);
    }

    param_buf = os_mem_alloc(OS_MEM_TYPE_DATA, 80);
    if (param_buf != NULL)
    {
        uint8_t *p = param_buf;

        LE_UINT16_TO_STREAM(p, GAP_LEGACY_ID);
        LE_UINT8_TO_STREAM(p, action);

        if (action == POWER_TEST_CMD_INQUIRY_SCAN_PARAM_SET)
        {
            LE_UINT8_TO_STREAM(p, type);
            LE_UINT16_TO_STREAM(p, interval);
            LE_UINT16_TO_STREAM(p, window);
        }
        else if (action == POWER_TEST_CMD_PAGE_SCAN_PARAM_SET)
        {
            LE_UINT8_TO_STREAM(p, type);
            LE_UINT16_TO_STREAM(p, interval);
            LE_UINT16_TO_STREAM(p, window);
            LE_UINT16_TO_STREAM(p, page_timeout);
        }
        else if (action == POWER_TEST_CMD_RADIO_MODE_SET)
        {
            LE_UINT8_TO_STREAM(p, radio_mode);
        }
        else if ((action == POWER_TEST_CMD_LINK_POLICY_SET) ||
                 (action == POWER_TEST_CMD_LINK_DEAULT_POLICY_SET))
        {
            LE_UINT8_TO_STREAM(p, link_policy);
        }
        else if (action == POWER_TEST_CMD_INQUIRY_START)
        {
            LE_UINT8_TO_STREAM(p, inquiry_timeout);
        }
        else if (action == POWER_TEST_CMD_SNIFF_ENTER)
        {
            LE_UINT16_TO_STREAM(p, min_interval);
            LE_UINT16_TO_STREAM(p, max_interval);
            LE_UINT16_TO_STREAM(p, sniff_attempt);
            LE_UINT16_TO_STREAM(p, sniff_timeout);
        }

        ARRAY_TO_STREAM(p, addr, 6);

        power_test_send_msg(IO_MSG_CONSOLE_STRING_RX, param_buf);
    }

    shell_print(sh, "Power test gap legacy %s.", argv[1]);
    return 0;

err:
    shell_error(sh, "Invalid param %s (power test gap legacy cmd).", argv[1]);
    return -EINVAL;
}

/* "power_test txpower <br_1M> <edr_2M> <edr_3M> <le_1M> <le_2M>" */
static int cmd_txpower(const struct shell *sh, size_t argc, char **argv)
{
    uint8_t     br_1M, edr_2M, edr_3M, le_1M, le_2M;
    void       *param_buf;

    br_1M  = (uint8_t)strtol(argv[1], NULL, 0);
    edr_2M = (uint8_t)strtol(argv[2], NULL, 0);
    edr_3M = (uint8_t)strtol(argv[3], NULL, 0);
    le_1M  = (uint8_t)strtol(argv[4], NULL, 0);
    le_2M  = (uint8_t)strtol(argv[5], NULL, 0);

    param_buf = os_mem_alloc(OS_MEM_TYPE_DATA, 100);
    if (param_buf != NULL)
    {
        uint8_t *p = param_buf;

        LE_UINT16_TO_STREAM(p, TX_POWER_ID);
        LE_UINT8_TO_STREAM(p, br_1M);
        LE_UINT8_TO_STREAM(p, edr_2M);
        LE_UINT8_TO_STREAM(p, edr_3M);
        LE_UINT8_TO_STREAM(p, le_1M);
        LE_UINT8_TO_STREAM(p, le_2M);

        power_test_send_msg(IO_MSG_CONSOLE_STRING_RX, param_buf);
    }

    shell_print(sh, "Power test tx power set.");
    return 0;
}

#ifdef CONFIG_SOC_SERIES_RTL8773D
/* "power_test mclk2 xtal|pll" (RTL8773D only, dead code on rtl87x3g) */
static int cmd_mclk2(const struct shell *sh, size_t argc, char **argv)
{
    uint16_t    action = 0;
    void       *param_buf;

    if (!strcmp(argv[1], "xtal"))
    {
        action = POWER_TEST_CMD_MCLK2_XTAL;
    }
    else if (!strcmp(argv[1], "pll"))
    {
        action = POWER_TEST_CMD_MCLK2_PLL;
    }
    else
    {
        shell_error(sh, "Invalid param %s (xtal or pll).", argv[1]);
        return -EINVAL;
    }

    param_buf = malloc(4);
    if (param_buf != NULL)
    {
        uint8_t *p = param_buf;

        LE_UINT16_TO_STREAM(p, MCLK2_ID);
        LE_UINT16_TO_STREAM(p, action);

        power_test_send_msg(IO_MSG_CONSOLE_STRING_RX, param_buf);
    }

    shell_print(sh, "Power test mclk2 %s.", argv[1]);
    return 0;
}
#endif

/* "power_test dsp1 disable|<freq>" */
static int cmd_dsp1(const struct shell *sh, size_t argc, char **argv)
{
    uint16_t    action;
    void       *param_buf;

    action = atoi((const char *)argv[1]);

    if (!strcmp(argv[1], "disable"))
    {
        action = POWER_TEST_CMD_DSP1_DISABLE;
    }
    else if (action == 0)
    {
        shell_error(sh,
                    "Invalid param %s (disable or other freq like 20 40 80 120 140 160 180 200 280 320).",
                    argv[1]);
        return -EINVAL;
    }

    param_buf = malloc(4);
    if (param_buf != NULL)
    {
        uint8_t *p = param_buf;

        LE_UINT16_TO_STREAM(p, DSP1_FREQ_ID);
        LE_UINT16_TO_STREAM(p, action);

        power_test_send_msg(IO_MSG_CONSOLE_STRING_RX, param_buf);
    }

    shell_print(sh, "Power test dsp1 freq %s.", argv[1]);
    return 0;
}

/* "power_test cpu sleep|active|625k|max|<freq>" */
static int cmd_cpu(const struct shell *sh, size_t argc, char **argv)
{
    uint16_t    action = 0;
    void       *param_buf;

    action = atoi((const char *)argv[1]);

    if (!strcmp(argv[1], "625k"))
    {
        action = POWER_TEST_CMD_CPU_FREQ_625K;
    }
    else if (!strcmp(argv[1], "sleep"))
    {
        action = POWER_TEST_CMD_CPU_SLEEP;
    }
    else if (!strcmp(argv[1], "active"))
    {
        action = POWER_TEST_CMD_CPU_ACTIVE;
    }
    else if (!strcmp(argv[1], "max"))
    {
        action = POWER_TEST_CMD_CPU_FREQ_MAX;
    }
    else if (action == 0)
    {
        shell_error(sh,
                    "Invalid param %s (sleep active 625k or other freq like 20 40 80 100 120 160).",
                    argv[1]);
        return -EINVAL;
    }

    param_buf = malloc(4);
    if (param_buf != NULL)
    {
        uint8_t *p = param_buf;

        LE_UINT16_TO_STREAM(p, CPU_FREQ_ID);
        LE_UINT16_TO_STREAM(p, action);

        power_test_send_msg(IO_MSG_CONSOLE_STRING_RX, param_buf);
    }

    shell_print(sh, "Power test cpu freq %s.", argv[1]);
    return 0;
}

/* "power_test dvfs high|low|normal1v1|normal0v9" */
static int cmd_dvfs(const struct shell *sh, size_t argc, char **argv)
{
    uint16_t    action;
    void       *param_buf;

#ifdef CONFIG_SOC_SERIES_RTL8773D
    if (!strcmp(argv[1], "normal0v9"))
    {
        action = POWER_TEST_CMD_SET_DVFS_NORMAL0V9;
    }
    else if (!strcmp(argv[1], "normal0v8"))
    {
        action = POWER_TEST_CMD_SET_DVFS_NORMAL0V8;
    }
    else if (!strcmp(argv[1], "low0v9"))
    {
        action = POWER_TEST_CMD_SET_DVFS_LOW0V9;
    }
    else if (!strcmp(argv[1], "low0v8"))
    {
        action = POWER_TEST_CMD_SET_DVFS_LOW0V8;
    }
    else if (!strcmp(argv[1], "low0v7"))
    {
        action = POWER_TEST_CMD_SET_DVFS_LOW0V7;
    }
    else if (!strcmp(argv[1], "low0v65"))
    {
        action = POWER_TEST_CMD_SET_DVFS_LOW0V65;
    }
    else if (!strcmp(argv[1], "low0v6125"))
    {
        action = POWER_TEST_CMD_SET_DVFS_LOW0V6125;
    }
#else
    if (!strcmp(argv[1], "normal1v1"))
    {
        action = POWER_TEST_CMD_SET_DVFS_NORMAL1V1;
    }
    else if (!strcmp(argv[1], "normal0v9"))
    {
        action = POWER_TEST_CMD_SET_DVFS_NORMAL0V9;
    }
#endif
    else if (!strcmp(argv[1], "high"))
    {
        action = POWER_TEST_CMD_SET_DVFS_HIGH;
    }
    else if (!strcmp(argv[1], "low"))
    {
        action = POWER_TEST_CMD_SET_DVFS_LOW;
    }
#ifdef CONFIG_SOC_SERIES_RTL8773D
    else if (!strcmp(argv[1], "close"))
    {
        action = POWER_TEST_CMD_SET_VCORE2_CLOSE;
    }
#endif
    else
    {
#ifdef CONFIG_SOC_SERIES_RTL8773D
        shell_error(sh,
                    "Invalid param %s (high low normal0v9 normal0v8 low0v9 low0v8 low0v7 low0v65 low0v6125).",
                    argv[1]);
#else
        shell_error(sh, "Invalid param %s (high low normal1v1 normal0v9).", argv[1]);
#endif
        return -EINVAL;
    }

    param_buf = malloc(4);
    if (param_buf != NULL)
    {
        uint8_t *p = param_buf;

        LE_UINT16_TO_STREAM(p, DVFS_ID);
        LE_UINT16_TO_STREAM(p, action);

        power_test_send_msg(IO_MSG_CONSOLE_STRING_RX, param_buf);
    }

    shell_print(sh, "Power test dvfs %s.", argv[1]);
    return 0;
}

/* "power_test state lps|dlps|dlps_ret|dlps_pfm|down|off|btsleep|btactive" */
static int cmd_state(const struct shell *sh, size_t argc, char **argv)
{
    uint16_t    action;
    void       *param_buf;

    if (!strcmp(argv[1], "lps"))
    {
        action = POWER_TEST_CMD_SET_LPS;
    }
    else if (!strcmp(argv[1], "dlps"))
    {
        action = POWER_TEST_CMD_SET_DLPS;
    }
#ifndef CONFIG_SOC_SERIES_RTL8773D
    else if (!strcmp(argv[1], "dlps_ret"))
    {
        action = POWER_TEST_CMD_SET_DLPS_RET;
    }
#endif
    else if (!strcmp(argv[1], "dlps_pfm"))
    {
        action = POWER_TEST_CMD_SET_DLPS_PFM;
    }
    else if (!strcmp(argv[1], "down"))
    {
        action = POWER_TEST_CMD_SET_POWER_DOWN;
    }
    else if (!strcmp(argv[1], "off"))
    {
        action = POWER_TEST_CMD_SET_POWER_OFF;
    }
    else if (!strcmp(argv[1], "btsleep"))
    {
        action = POWER_TEST_CMD_SET_BT_MAC_SELLP;
    }
    else if (!strcmp(argv[1], "btactive"))
    {
        action = POWER_TEST_CMD_SET_BT_MAC_ACTIVE;
    }
    else
    {
#ifdef CONFIG_SOC_SERIES_RTL8773D
        shell_error(sh, "Invalid param %s (lps dlps dlps_pfm down off btsleep btactive).", argv[1]);
#else
        shell_error(sh, "Invalid param %s (lps dlps dlps_ret dlps_pfm down off btsleep btactive).",
                    argv[1]);
#endif
        return -EINVAL;
    }

    param_buf = malloc(4);
    if (param_buf != NULL)
    {
        uint8_t *p = param_buf;

        LE_UINT16_TO_STREAM(p, POWER_STATE_ID);
        LE_UINT16_TO_STREAM(p, action);

        power_test_send_msg(IO_MSG_CONSOLE_STRING_RX, param_buf);
    }

    shell_print(sh, "Power test state %s.", argv[1]);
    return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(power_test_subcmds,
                               SHELL_CMD_ARG(state, NULL,
                                             "lps dlps dlps_ret dlps_pfm down off btsleep btactive", cmd_state, 2, 0),
                               //    SHELL_CMD_ARG(dvfs, NULL, "high low normal1v1 normal0v9", cmd_dvfs, 2, 0),
                               SHELL_CMD_ARG(cpu, NULL, "sleep active 625k max or freq num", cmd_cpu, 2, 0),
                               SHELL_CMD_ARG(dsp1, NULL, "disable or freq num", cmd_dsp1, 2, 0),
#ifdef CONFIG_SOC_SERIES_RTL8773D
                               SHELL_CMD_ARG(mclk2, NULL, "xtal or pll", cmd_mclk2, 2, 0),
#endif
                               SHELL_CMD_ARG(txpower, NULL, "br_1M edr_2M edr_3M le_1M le_2M (int8_t 0.5dbm)", cmd_txpower, 6, 0),
                               SHELL_CMD_ARG(gap_legacy_cmd, NULL, "power test gap legacy cmd", cmd_gap_legacy, 8, 4),
                               SHELL_CMD_ARG(gap_le_cmd, NULL, "power test gap le cmd", cmd_gap_le, 2, 18),
                               SHELL_CMD_ARG(cont_tx, NULL, "tx_power packet_type", cmd_cont_tx, 3, 0),
                               SHELL_CMD_ARG(packet_rx, NULL, "packet_type(1DH5:2 2DH5:5 3DH5:8 LE1M:9 LE2M:11)", cmd_packet_rx, 2,
                                             0),
                               SHELL_CMD_ARG(flash, NULL,
                                             "write read erase xip cache half_cache dma_read write_prepare erase_prepare",
                                             cmd_flash, 2, 0),
                               SHELL_CMD_ARG(32k, NULL, "on or off", cmd_32k, 2, 0),
                               SHELL_SUBCMD_SET_END
                              );

SHELL_CMD_REGISTER(power_test, &power_test_subcmds, "Realtek power test commands", NULL);
