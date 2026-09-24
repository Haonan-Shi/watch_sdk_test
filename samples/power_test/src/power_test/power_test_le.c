/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */
#include "power_test.h"
#include "string.h"
#include <zephyr/shell/shell.h>
#include <stdlib.h>
#include "pm.h"
#include "os_mem.h"
#include "power_test_le.h"
#include "gap_le_types.h"
#include "ble_ext_adv.h"
#include "app_msg.h"
#include <stdio.h>
#include "trace.h"
#include "ble_mgr.h"
#include "gap_bond_le.h"
#include "ble_conn.h"
#include "gap_conn_le.h"

static T_POWER_LE_ADV le_adv_power;
static T_POWER_LE_SCAN le_scan_power;
static T_POWER_LE_CONN le_conn_power;
static uint8_t power_adv_handle = 0xff;
static uint8_t power_le_conn_id = 0xFF;
static T_GAP_DEV_STATE le_state;
static T_POWER_LE_PENDING_ACTION le_pending_action;
static uint8_t le_cmd_header[3];
static uint8_t bd_addr1[6] = {0x19, 0x12, 0x14, 0x54, 0x67, 0x89};
static uint8_t bd_addr2[6] = {0x89, 0x67, 0x54, 0x14, 0x12, 0x19};
static uint8_t ble_conn_console_response = 0;
#define POWER_ADV_STOP_USER_STOP   0xF0

extern void power_test_send_msg(T_IO_CONSOLE subtype, void *param_buf);

/** @brief  GAP - Advertisement data (max size = 31 bytes, best kept short to conserve power) */
uint8_t power_adv_data[31];
uint8_t power_scan_rsp_data[] =
{
    /* Flags */
    0x02,             /* length */
    GAP_ADTYPE_FLAGS, /* type="Flags" */
    GAP_ADTYPE_FLAGS_LIMITED | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,
    /* Local name */
    0x0F,             /* length */
    GAP_ADTYPE_LOCAL_NAME_COMPLETE,
    'B', 'L', 'E', '_', 'P', 'O', 'W', 'E', 'R', '_', 'T', 'E', 'S', 'T',
};
uint8_t  device_name[40] = "BLE_POWER_TEST";
/**
 * @brief
 * adv_phy @ref T_GAP_PHYS_PRIM_ADV_TYPE
 * adv_event_prop @ref T_LE_EXT_ADV_LEGACY_ADV_PROPERTY
 * adv_interval In units of 0.625ms, range: 0x000020 to 0xFFFFFF.
 * adv_data_length 0-31
 *
 * conn_phy   GAP_PHYS_PREFER_1M_BIT, GAP_PHYS_PREFER_2M_BIT, GAP_PHYS_PREFER_CODED_BIT
 * conn_slave_latency  Range: 0x0000 to 0x01F3
 * conn_interval_min/conn_interval_max  In units of 1.25ms, Range: 0x0006 to 0x0C80.
 *
 * scan_mode @ref T_GAP_SCAN_MODE
 * scan_phy @ref T_LE_EXT_SCAN_PHY_TYPE
 * scan_interval  In units of 0.625ms, range: 0x0004 to 0xFFFF.
 * scan_window  In units of 0.625ms, range: 0x0004 to 0xFFFF.
 * duplicate_enable @ref T_GAP_SCAN_FILTER_DUPLICATE
 */
int cmd_gap_le(const struct shell *sh, size_t argc, char **argv)
{
    int32_t param_num  = argc - 1;   /* params after "gap_le" (sub-action + args) */
    uint8_t action     = POWER_TEST_CMD_LE_UNKNOW;

    //adv
    le_adv_power.adv_interval = 0x20;
    le_adv_power.adv_data_length  = 31;

    //conn
    le_conn_power.conn_phy           = GAP_PHYS_PREFER_1M_BIT;
    le_conn_power.conn_slave_latency = 0;
    le_conn_power.conn_interval_min  = 12;
    le_conn_power.conn_interval_max  = 24;

    //scan
    le_scan_power.scan_phy         = LE_SCAN_PHY_LE_1M;
    le_scan_power.scan_mode        = GAP_SCAN_MODE_PASSIVE;
    le_scan_power.scan_interval    = 0x30;
    le_scan_power.scan_window      = 0x30;
    le_scan_power.duplicate_enable = GAP_SCAN_FILTER_DUPLICATE_ENABLE;

    if (param_num < 1)
    {
        goto err;
    }

    if (!strcmp(argv[1], "le_adv_power_test_start"))
    {
        if (param_num != 3)
        {
            goto err;
        }

        le_adv_power.adv_interval = (uint16_t)strtol(argv[2], NULL, 0);
        le_adv_power.adv_data_length = (uint16_t)strtol(argv[3], NULL, 0);

        action = POWER_TEST_CMD_LE_ADV_START;
    }
    else if (!strcmp(argv[1], "le_adv_power_test_stop"))
    {
        if (param_num != 1)
        {
            goto err;
        }
        action = POWER_TEST_CMD_LE_ADV_STOP;
    }
    else if (!strcmp(argv[1], "le_conn_power_test_start"))
    {
        if (param_num != 4)
        {
            goto err;
        }

        le_conn_power.conn_slave_latency = (uint16_t)strtol(argv[2], NULL, 0);
        le_conn_power.conn_interval_min = (uint16_t)strtol(argv[3], NULL, 0);
        le_conn_power.conn_interval_max = (uint16_t)strtol(argv[4], NULL, 0);

        action = POWER_TEST_CMD_LE_CONN_START;
    }
    else if (!strcmp(argv[1], "le_conn_power_test_stop"))
    {
        if (param_num != 1)
        {
            goto err;
        }

        action = POWER_TEST_CMD_LE_CONN_STOP;
    }
    else if (!strcmp(argv[1], "le_scan_power_test_start"))
    {
        if (param_num != 5)
        {
            goto err;
        }

        le_scan_power.scan_mode = (uint8_t)strtol(argv[2], NULL, 0);
        le_scan_power.scan_interval = (uint16_t)strtol(argv[3], NULL, 0);
        le_scan_power.scan_window = (uint16_t)strtol(argv[4], NULL, 0);
        le_scan_power.duplicate_enable = (uint8_t)strtol(argv[5], NULL, 0);

        action = POWER_TEST_CMD_LE_SCAN_START;
    }
    else if (!strcmp(argv[1], "le_scan_power_test_stop"))
    {
        if (param_num != 1)
        {
            goto err;
        }
        action = POWER_TEST_CMD_LE_SCAN_STOP;
    }
    else if (!strcmp(argv[1], "le_create_conn"))
    {
        if (param_num != 7)
        {
            goto err;
        }

        bd_addr1[0] = (uint8_t)strtol(argv[2], NULL, 0);
        bd_addr2[5] = bd_addr1[0];

        bd_addr1[1] = (uint8_t)strtol(argv[3], NULL, 0);
        bd_addr2[4] = bd_addr1[1];

        bd_addr1[2] = (uint8_t)strtol(argv[4], NULL, 0);
        bd_addr2[3] = bd_addr1[2];

        bd_addr1[3] = (uint8_t)strtol(argv[5], NULL, 0);
        bd_addr2[2] = bd_addr1[3];

        bd_addr1[4] = (uint8_t)strtol(argv[6], NULL, 0);
        bd_addr2[1] = bd_addr1[4];

        bd_addr1[5] = (uint8_t)strtol(argv[7], NULL, 0);
        bd_addr2[0] = bd_addr1[5];

        action = POWER_TEST_CMD_LE_CREATE_CONN;
    }
    else if (!strcmp(argv[1], "le_cancel_conn"))
    {
        if (param_num != 1)
        {
            goto err;
        }
        action = POWER_TEST_CMD_LE_CANCEL_CONN;
    }
    else
    {
        goto err;
    }

    le_cmd_header[0] = GAP_LE_ID;
    le_cmd_header[1] = 0x00;
    le_cmd_header[2] = action;

    if (action == POWER_TEST_CMD_LE_ADV_START)
    {
        shell_print(sh, "gap_le_cmd: adv_interval %d, adv_data_length %d",
                    le_adv_power.adv_interval, le_adv_power.adv_data_length);
    }
    else if (action == POWER_TEST_CMD_LE_CONN_START)
    {
        shell_print(sh,
                    "gap_le_cmd: conn_slave_latency %d, conn_interval_min %d, conn_interval_max %d",
                    le_conn_power.conn_slave_latency, le_conn_power.conn_interval_min,
                    le_conn_power.conn_interval_max);
    }
    else if (action == POWER_TEST_CMD_LE_SCAN_START)
    {
        shell_print(sh,
                    "gap_le_cmd: scan_mode %d, scan_interval %d, scan_window %d, duplicate_enable %d",
                    le_scan_power.scan_mode, le_scan_power.scan_interval, le_scan_power.scan_window,
                    le_scan_power.duplicate_enable);
    }
    else if (action == POWER_TEST_CMD_LE_SCAN_STOP)
    {
        shell_print(sh, "gap_le_cmd: le_scan_power_test_stop");
    }
    else if (action == POWER_TEST_CMD_LE_ADV_STOP)
    {
        shell_print(sh, "gap_le_cmd: le_adv_power_test_stop");
    }
    else if (action == POWER_TEST_CMD_LE_CONN_STOP)
    {
        shell_print(sh, "gap_le_cmd: le_conn_power_test_stop");
    }

    power_test_send_msg(IO_MSG_CONSOLE_STRING_RX, le_cmd_header);
    return 0;

err:
    shell_error(sh, "gap_le_cmd: Invalid param %s, param_num %d", argv[1], param_num);
    return -EINVAL;
}

void power_le_adv_cback(uint8_t cb_type, void *p_cb_data)
{
    static uint8_t ble_adv_console_response = 1;
    static uint8_t ble_con_console_response = 1;
    T_BLE_EXT_ADV_CB_DATA cb_data;
    T_BLE_EXT_ADV_MGR_STATE le_adv_state;
    memcpy(&cb_data, p_cb_data, sizeof(T_BLE_EXT_ADV_CB_DATA));
    switch (cb_type)
    {
    case BLE_EXT_ADV_STATE_CHANGE:
        {
            le_adv_state = cb_data.p_ble_state_change->state;
            if (le_adv_state == BLE_EXT_ADV_MGR_ADV_ENABLED)
            {
                if (ble_adv_console_response)
                {
                    char *temp_buff = "BLE_EXT_ADV_MGR_ADV_ENABLED\r\n";
                    printk("%s", temp_buff);
                    ble_adv_console_response = 0;
                }
                APP_PRINT_TRACE1("power_le_adv_cback: BLE_EXT_ADV_MGR_ADV_ENABLED, adv_handle %d",
                                 cb_data.p_ble_state_change->adv_handle);
            }
            else if (le_adv_state == BLE_EXT_ADV_MGR_ADV_DISABLED)
            {
                APP_PRINT_TRACE1("power_le_adv_cback: BLE_EXT_ADV_MGR_ADV_DISABLED, adv_handle %d",
                                 cb_data.p_ble_state_change->adv_handle);
                switch (cb_data.p_ble_state_change->stop_cause)
                {
                case BLE_EXT_ADV_STOP_CAUSE_APP:
                    APP_PRINT_TRACE1("power_le_adv_cback: BLE_EXT_ADV_STOP_CAUSE_APP app_cause 0x%02x",
                                     cb_data.p_ble_state_change->app_cause);
                    break;

                case BLE_EXT_ADV_STOP_CAUSE_CONN:
                    {
                        if (ble_con_console_response)
                        {
                            char *temp_buff = "BLE_EXT_ADV_STOP_CAUSE_CONN\r\n";
                            printk("%s", temp_buff);
                            ble_con_console_response = 0;
                        }
                        APP_PRINT_TRACE0("power_le_adv_cback: BLE_EXT_ADV_STOP_CAUSE_CONN");
                    }
                    break;
                case BLE_EXT_ADV_STOP_CAUSE_TIMEOUT:
                    APP_PRINT_TRACE0("power_le_adv_cback: BLE_EXT_ADV_STOP_CAUSE_TIMEOUT");
                    break;
                default:
                    APP_PRINT_TRACE1("power_le_adv_cback: stop_cause %d",
                                     cb_data.p_ble_state_change->stop_cause);
                    break;
                }
            }
        }
        break;

    case BLE_EXT_ADV_SET_CONN_INFO:
        {
            APP_PRINT_TRACE1("power_le_adv_cback: BLE_EXT_ADV_SET_CONN_INFO conn_id 0x%x",
                             cb_data.p_ble_conn_info->conn_id);
            power_le_conn_id = cb_data.p_ble_conn_info->conn_id;
        }
        break;

    default:
        break;
    }
    return;
}

void power_le_init_adv_param()
{
    memset(power_adv_data, 0xFF, 31);
    uint16_t adv_len = 31;
    power_adv_data[0] = adv_len - 1;

    uint16_t adv_event_prop = LE_EXT_ADV_LEGACY_ADV_CONN_SCAN_UNDIRECTED;
    uint16_t adv_interval_min = 0x30;
    uint16_t adv_interval_max = 0x30;
    T_GAP_LOCAL_ADDR_TYPE own_address_type = GAP_LOCAL_ADDR_LE_PUBLIC;
    T_GAP_REMOTE_ADDR_TYPE peer_address_type = GAP_REMOTE_ADDR_LE_PUBLIC;
    uint8_t  peer_address[6] = {0, 0, 0, 0, 0, 0};
    T_GAP_ADV_FILTER_POLICY filter_policy = GAP_ADV_FILTER_ANY;

    ble_ext_adv_mgr_init_adv_params(&power_adv_handle, adv_event_prop, adv_interval_min,
                                    adv_interval_max, own_address_type, peer_address_type, peer_address,
                                    filter_policy, adv_len, power_adv_data,
                                    sizeof(power_scan_rsp_data), power_scan_rsp_data, NULL);

    ble_ext_adv_mgr_register_callback(power_le_adv_cback, power_adv_handle);
}

void power_handle_le_cmd(uint8_t action)
{
    DBG_DIRECT("power_handle_le_cmd: action %d", action);
    T_GAP_DEV_STATE dev_state;
    le_get_gap_param(GAP_PARAM_DEV_STATE, &dev_state);

    switch (action)
    {
    case POWER_TEST_CMD_LE_ADV_START:
        {
            if (dev_state.gap_init_state == GAP_INIT_STATE_INIT)
            {
                le_pending_action.le_adv_pending = true;
                DBG_DIRECT("power_handle_le_cmd: adv_pending %d", le_pending_action.le_adv_pending);
                return;
            }
            uint16_t adv_data_length = le_adv_power.adv_data_length;
            power_adv_data[0] = adv_data_length - 1;
            ble_ext_adv_mgr_set_adv_data(power_adv_handle, adv_data_length, power_adv_data);
            ble_ext_adv_mgr_change_adv_interval(power_adv_handle, le_adv_power.adv_interval);
            ble_ext_adv_mgr_enable(power_adv_handle, 0);
        }
        break;

    case POWER_TEST_CMD_LE_ADV_STOP:
        {
            ble_ext_adv_mgr_disable(power_adv_handle, POWER_ADV_STOP_USER_STOP);
        }
        break;

    case POWER_TEST_CMD_LE_CONN_START:
        {
            if (dev_state.gap_init_state == GAP_INIT_STATE_INIT)
            {
                le_pending_action.le_conn_adv_pending = true;
                DBG_DIRECT("power_handle_le_cmd: conn_adv_pending %d", le_pending_action.le_conn_adv_pending);
                return;
            }
            ble_ext_adv_mgr_enable(power_adv_handle, 0);
            le_pending_action.le_conn_update_pending = true;
        }
        break;
    case POWER_TEST_CMD_LE_CONN_STOP:
        {
            ble_ext_adv_mgr_disable(power_adv_handle, POWER_ADV_STOP_USER_STOP);
            if (power_le_conn_id != 0xFF)
            {
                le_disconnect(power_le_conn_id);
            }
        }
        break;
    case POWER_TEST_CMD_LE_SCAN_START:
        {
            if (dev_state.gap_init_state == GAP_INIT_STATE_INIT)
            {
                le_pending_action.le_scan_pending = true;
                DBG_DIRECT("power_handle_le_cmd: le_scan_pending %d", le_pending_action.le_scan_pending);
                return;
            }

            T_GAP_LE_EXT_SCAN_PARAM extended_scan_param[GAP_EXT_SCAN_MAX_PHYS_NUM];

            extended_scan_param[0].scan_type = (T_GAP_SCAN_MODE)le_scan_power.scan_mode;
            extended_scan_param[0].scan_interval = le_scan_power.scan_interval;
            extended_scan_param[0].scan_window = le_scan_power.scan_window;

            le_ext_scan_set_param(GAP_PARAM_EXT_SCAN_FILTER_DUPLICATES, 1, &le_scan_power.duplicate_enable);
            le_ext_scan_set_phy_param(LE_SCAN_PHY_LE_1M, &extended_scan_param[0]);
            le_ext_scan_start();
        }
        break;
    case POWER_TEST_CMD_LE_SCAN_STOP:
        {
            le_ext_scan_stop();
        }
        break;

    case POWER_TEST_CMD_LE_CREATE_CONN:
        {
            ble_conn_console_response = 1;
            T_GAP_LE_EXT_SCAN_PARAM extended_scan_param[GAP_EXT_SCAN_MAX_PHYS_NUM];

            extended_scan_param[0].scan_type = (T_GAP_SCAN_MODE)le_scan_power.scan_mode;
            extended_scan_param[0].scan_interval = le_scan_power.scan_interval;
            extended_scan_param[0].scan_window = le_scan_power.scan_window;

            le_ext_scan_set_param(GAP_PARAM_EXT_SCAN_FILTER_DUPLICATES, 1, &le_scan_power.duplicate_enable);
            le_ext_scan_set_phy_param(LE_SCAN_PHY_LE_1M, &extended_scan_param[0]);
            le_ext_scan_start();
        }
        break;

    default:
        break;
    }
}

T_APP_RESULT power_ble_gap_cb(uint8_t cb_type, void *p_cb_data)
{
    static uint8_t ble_scan_console_response = 1;
    T_APP_RESULT result = APP_RESULT_SUCCESS;
    T_LE_CB_DATA cb_data;

    memcpy(&cb_data, p_cb_data, sizeof(T_LE_CB_DATA));

    ble_mgr_handle_gap_cb(cb_type, p_cb_data);

    switch (cb_type)
    {
    case GAP_MSG_LE_EXT_ADV_REPORT_INFO:
        {
            if (ble_scan_console_response)
            {
                char *temp_buff = "GAP_MSG_LE_EXT_ADV_REPORT_INFO\r\n";
                printk("%s", temp_buff);
                ble_scan_console_response = 0;
            }
            APP_PRINT_INFO6("GAP_MSG_LE_EXT_ADV_REPORT_INFO:event_type 0x%x, bd_addr %s, addr_type %d, rssi %d, data_len %d,data %b",
                            cb_data.p_le_ext_adv_report_info->event_type,
                            TRACE_BDADDR(cb_data.p_le_ext_adv_report_info->bd_addr),
                            cb_data.p_le_ext_adv_report_info->addr_type,
                            cb_data.p_le_ext_adv_report_info->rssi,
                            cb_data.p_le_ext_adv_report_info->data_len,
                            TRACE_BINARY(cb_data.p_le_ext_adv_report_info->data_len, cb_data.p_le_ext_adv_report_info->p_data));

            if ((memcmp(cb_data.p_le_ext_adv_report_info->bd_addr, bd_addr1, 6) == 0) ||
                (memcmp(cb_data.p_le_ext_adv_report_info->bd_addr, bd_addr2, 6) == 0))
            {
                if (ble_conn_console_response)
                {
                    T_GAP_LE_CONN_REQ_PARAM conn_req_param;
                    conn_req_param.scan_interval = 0x10;
                    conn_req_param.scan_window = 0x10;
                    conn_req_param.conn_interval_min = 80;
                    conn_req_param.conn_interval_max = 80;
                    conn_req_param.conn_latency = 0;
                    conn_req_param.supv_tout = 1000;
                    conn_req_param.ce_len_min = 2 * (conn_req_param.conn_interval_min - 1);
                    conn_req_param.ce_len_max = 2 * (conn_req_param.conn_interval_max - 1);
                    le_set_conn_param(GAP_CONN_PARAM_1M, &conn_req_param);
                    le_connect(GAP_PHYS_CONN_INIT_1M_BIT, cb_data.p_le_ext_adv_report_info->bd_addr,
                               cb_data.p_le_ext_adv_report_info->addr_type,
                               GAP_LOCAL_ADDR_LE_PUBLIC, 1000);

                    char *temp_buff = "Receive Power Test ADV and create CONN\r\n";
                    printk("%s", temp_buff);
                    ble_conn_console_response = 0;
                }

            }
        }
        break;

    case GAP_MSG_LE_CONN_UPDATE_IND:
        result = APP_RESULT_ACCEPT;
        break;

    case GAP_MSG_LE_EXT_ADV_START_SETTING:
        break;

    default:
        break;
    }

    return result;
}

static void power_ble_handle_dev_state_change_evt(T_GAP_DEV_STATE new_state, uint16_t cause)
{
    APP_PRINT_TRACE5("app_ble_gap_handle_dev_state_change_evt: le_state.gap_adv_state %d, new_state.gap_adv_state %d, "
                     "le_state.gap_scan_state %d, new_state.gap_scan_state %d, cause 0x%04x",
                     le_state.gap_adv_state,
                     new_state.gap_adv_state,
                     le_state.gap_scan_state,
                     new_state.gap_scan_state,
                     cause);

    if (le_state.gap_init_state != new_state.gap_init_state)
    {
        if (new_state.gap_init_state == GAP_INIT_STATE_STACK_READY)
        {
            DBG_DIRECT("app_ble_gap_handle_dev_state_change_evt: GAP stack ready");
            if (le_pending_action.le_adv_pending)
            {
                uint16_t adv_data_length = le_adv_power.adv_data_length;
                power_adv_data[0] = adv_data_length - 1;
                ble_ext_adv_mgr_set_adv_data(power_adv_handle, adv_data_length, power_adv_data);
                ble_ext_adv_mgr_change_adv_interval(power_adv_handle, le_adv_power.adv_interval);
                ble_ext_adv_mgr_enable(power_adv_handle, 0);
                le_pending_action.le_adv_pending = false;
            }
            else if (le_pending_action.le_conn_adv_pending)
            {
                ble_ext_adv_mgr_enable(power_adv_handle, 0);
                le_pending_action.le_conn_adv_pending = false;
                le_pending_action.le_conn_update_pending = true;
            }
            else if (le_pending_action.le_scan_pending)
            {
                T_GAP_LE_EXT_SCAN_PARAM extended_scan_param[GAP_EXT_SCAN_MAX_PHYS_NUM];

                extended_scan_param[0].scan_type = (T_GAP_SCAN_MODE)le_scan_power.scan_mode;
                extended_scan_param[0].scan_interval = le_scan_power.scan_interval;
                extended_scan_param[0].scan_window = le_scan_power.scan_window;

                le_ext_scan_set_param(GAP_PARAM_EXT_SCAN_FILTER_DUPLICATES, 1, &le_scan_power.duplicate_enable);
                le_ext_scan_set_phy_param(LE_SCAN_PHY_LE_1M, &extended_scan_param[0]);
                le_ext_scan_start();
                le_pending_action.le_scan_pending = false;
            }
        }
    }

    le_state = new_state;
}

static void power_ble_handle_new_conn_state_evt(uint8_t conn_id, T_GAP_CONN_STATE new_state,
                                                uint16_t disc_cause)
{
    APP_PRINT_TRACE3("app_ble_gap_handle_new_conn_state_evt: conn_id %d, new_state %d, cause 0x%04x",
                     conn_id, new_state, disc_cause);

    switch (new_state)
    {
    case GAP_CONN_STATE_DISCONNECTED:
        break;

    case GAP_CONN_STATE_CONNECTED:
        {
            if (le_pending_action.le_conn_update_pending)
            {
                uint16_t conn_interval_min = le_conn_power.conn_interval_min;
                uint16_t conn_interval_max = le_conn_power.conn_interval_max;
                uint16_t conn_latency = le_conn_power.conn_slave_latency;
                uint16_t conn_supervision_timeout = 500;
                ble_set_prefer_conn_param(conn_id, conn_interval_min, conn_interval_max, conn_latency,
                                          conn_supervision_timeout);
                le_pending_action.le_conn_update_pending = false;
            }
        }
        break;

    default:
        break;
    }
}

void power_ble_handle_gap_msg(T_IO_MSG *p_io_msg)
{
    APP_PRINT_TRACE1("app_ble_gap_handle_gap_msg: subtype %d", p_io_msg->subtype);
    T_LE_GAP_MSG stack_msg;
    memcpy(&stack_msg, &p_io_msg->u.param, sizeof(p_io_msg->u.param));
    ble_mgr_handle_gap_msg(p_io_msg->subtype, &stack_msg);

    switch (p_io_msg->subtype)
    {
    case GAP_MSG_LE_DEV_STATE_CHANGE:
        {
            power_ble_handle_dev_state_change_evt(stack_msg.msg_data.gap_dev_state_change.new_state,
                                                  stack_msg.msg_data.gap_dev_state_change.cause);
        }
        break;

    case GAP_MSG_LE_CONN_STATE_CHANGE:
        {
            power_ble_handle_new_conn_state_evt(stack_msg.msg_data.gap_conn_state_change.conn_id,
                                                (T_GAP_CONN_STATE)stack_msg.msg_data.gap_conn_state_change.new_state,
                                                stack_msg.msg_data.gap_conn_state_change.disc_cause);
        }
        break;

    case GAP_MSG_LE_CONN_MTU_INFO:
        break;

    case GAP_MSG_LE_CONN_PARAM_UPDATE:
        break;

    default:
        break;
    }
}

void power_ble_gap_param_init(void)
{
    //device name and device appearance
    uint16_t appearance = GAP_GATT_APPEARANCE_UNKNOWN;
    T_GAP_SCAN_MODE  scan_mode = GAP_SCAN_MODE_PASSIVE;
    uint8_t scan_filter_policy = GAP_SCAN_FILTER_ANY;
    uint8_t scan_filter_duplicate = GAP_SCAN_FILTER_DUPLICATE_ENABLE;
    //GAP Bond Manager parameters
    uint32_t passkey = 0; // passkey "000000"
    uint8_t use_fixed_passkey = false;
    uint8_t sec_req_enable = false;
    uint16_t sec_req_requirement = GAP_AUTHEN_BIT_SC_FLAG;
    /* Initialize extended scan parameters */
    T_GAP_LOCAL_ADDR_TYPE  own_address_type = GAP_LOCAL_ADDR_LE_PUBLIC;
    T_GAP_LE_EXT_SCAN_PARAM extended_scan_param[GAP_EXT_SCAN_MAX_PHYS_NUM];
    extended_scan_param[0].scan_type = scan_mode;
    extended_scan_param[0].scan_interval = 0x10;
    extended_scan_param[0].scan_window = 0x10;
    uint8_t  scan_phys = GAP_EXT_SCAN_PHYS_1M_BIT;
    uint16_t ext_scan_duration = 0;
    uint16_t ext_scan_period = 0;

    uint8_t link_num = 2;
    le_gap_init(link_num);

    le_set_gap_param(GAP_PARAM_DEVICE_NAME, GAP_DEVICE_NAME_LEN, device_name);
    //Set device appearance
    le_set_gap_param(GAP_PARAM_APPEARANCE, sizeof(appearance), &appearance);

    // Setup the GAP Bond Manager
    le_bond_set_param(GAP_PARAM_BOND_FIXED_PASSKEY, sizeof(uint32_t), &passkey);
    le_bond_set_param(GAP_PARAM_BOND_FIXED_PASSKEY_ENABLE, sizeof(uint8_t), &use_fixed_passkey);
    le_bond_set_param(GAP_PARAM_BOND_SEC_REQ_ENABLE, sizeof(uint8_t), &sec_req_enable);
    le_bond_set_param(GAP_PARAM_BOND_SEC_REQ_REQUIREMENT, sizeof(uint16_t), &sec_req_requirement);

    le_ext_scan_set_param(GAP_PARAM_EXT_SCAN_LOCAL_ADDR_TYPE, sizeof(own_address_type),
                          &own_address_type);
    le_ext_scan_set_param(GAP_PARAM_EXT_SCAN_PHYS, sizeof(scan_phys),
                          &scan_phys);
    le_ext_scan_set_param(GAP_PARAM_EXT_SCAN_DURATION, sizeof(ext_scan_duration),
                          &ext_scan_duration);
    le_ext_scan_set_param(GAP_PARAM_EXT_SCAN_PERIOD, sizeof(ext_scan_period),
                          &ext_scan_period);
    le_ext_scan_set_param(GAP_PARAM_EXT_SCAN_FILTER_POLICY, sizeof(scan_filter_policy),
                          &scan_filter_policy);
    le_ext_scan_set_param(GAP_PARAM_EXT_SCAN_FILTER_DUPLICATES, sizeof(scan_filter_duplicate),
                          &scan_filter_duplicate);
    le_ext_scan_set_phy_param(LE_SCAN_PHY_LE_1M, &extended_scan_param[0]);
}
