/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_ble_adv.h"
#include "gap_le_types.h"
#include "gap.h"
#include "string.h"
#include "app_cfg.h"
#include "gap_le.h"
#include "ble_ext_adv.h"
#include "os_queue.h"
#include "ble_mgr.h"
#include "module_global_data.h"
#include "app_main.h"
#include "dfu_transport.h"
#include "app_transfer.h"
#if CONFIG_REALTEK_SUBSYS_GATT_PROFILE_ANCS_CLIENT
#include "bt_gatt_client.h"
#endif

#define DEFAULT_ADVERTISING_INTERVAL_MIN            1600
/** @brief  Default maximum advertising interval */
#define DEFAULT_ADVERTISING_INTERVAL_MAX            1600

#define GATT_UUID128_BWPS_ADV   0x12, 0xA2, 0x4D, 0x2E, 0xFE, 0x14, 0x48, 0x8e, 0x93, 0xD2, 0x17, 0x3C, 0xFF, 0x01, 0x00, 0x00

#define ADV_DATA_MAX_LE_NAME_LEN     12

/** @brief  GAP - scan response data (max size = 31 bytes) */
static uint8_t scan_rsp_data[] =
{
    /* Service */
    17,             /* length     */
    GAP_ADTYPE_128BIT_MORE,            /* type="Complete 128-bit UUIDs available" */
    GATT_UUID128_BWPS_ADV,

    /* place holder for Local Name, filled by BT stack. if not present */
    /* BT stack appends Local Name.                                    */
    0x03,           /* length     */
    0x19,           /* type="Appearance" */
    0x42, 0x0c,     /* wrist worn */
};

/** @brief  GAP - Advertisement data (max size = 31 bytes, best kept short to conserve power) */
static  uint8_t adv_data[31] =
{
    /* Core spec. Vol. 3, Part C, Chapter 18 */
    /* Flags */
    /* place holder for Local Name, filled by BT stack. if not present */
    /* BT stack appends Local Name.                                    */
    0x02,            /* length     */
    GAP_ADTYPE_FLAGS,
    GAP_ADTYPE_FLAGS_GENERAL,
    /* Service */
    /* Alipay Service */
    0x03,           /* length     */
    0x03,           /* type="More 16-bit UUIDs available, service uuid 0xFEE7 0xA00A" */
    0x02,
    0x38,
    /* Manufacture specified data*/
    0x09,           /* length     */
    0xFF,           /* type: manufacture specific data*/
    0xC5, 0xFE,     /* company id */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* mac address*/

    0x00,           /* length     */
    GAP_ADTYPE_LOCAL_NAME_COMPLETE,           /* type="Complete local name" */
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0' /* SmartBracelet */
};

void app_ble_common_handle_dev_state(T_GAP_DEV_STATE new_state, uint16_t cause);
void app_ble_common_handle_conn_state(uint8_t conn_id, T_GAP_CONN_STATE new_state,
                                      uint16_t disc_cause);
void app_ble_common_handle_authen_state(uint8_t conn_id, uint8_t new_state, uint16_t cause);
void app_ble_common_handle_conn_mtu_info(uint8_t conn_id, uint16_t mtu_size);
void app_ble_common_handle_conn_param_update(uint8_t conn_id, uint8_t status, uint16_t cause);

const T_FUN_GAP_MSG_CBS  common_adv_gap_msg_cbs =
{
    .device_state_cb = app_ble_common_handle_dev_state,
    .conn_state_cb = app_ble_common_handle_conn_state,
    .authen_state_cb = app_ble_common_handle_authen_state,
    .mtu_info_cb = app_ble_common_handle_conn_mtu_info,
    .conn_param_cb = app_ble_common_handle_conn_param_update,
};

T_LE_ADV_CONN le_common_adv =
{
    .adv_handle = 0xff,
    .conn_id = 0xff,
    .role = GAP_LINK_ROLE_SLAVE,
    .state = BLE_EXT_ADV_MGR_ADV_DISABLED,
    .cb_tbl = &common_adv_gap_msg_cbs,
};

void app_ble_common_handle_dev_state(T_GAP_DEV_STATE new_state, uint16_t cause)
{
    APP_PRINT_INFO3("app_ble_common_handle_dev_state: init state %d, adv state %d, cause 0x%x",
                    new_state.gap_init_state, new_state.gap_adv_state, cause);
    if (RtkWristbandSys.gap_dev_state.gap_init_state != new_state.gap_init_state)
    {
        if (new_state.gap_init_state == GAP_INIT_STATE_STACK_READY)
        {
            if (!app_db.ble_is_ready)
            {
                app_db.ble_is_ready = true;
                APP_PRINT_INFO0("GAP stack ready");
                /*stack ready*/
                if (app_db.ble_is_ready && app_db.bt_is_ready)
                {
                    app_ble_common_adv_set_param();
                    app_ble_common_adv_start(0);
                }
            }
        }
    }
    RtkWristbandSys.gap_dev_state = new_state;
}

void app_ble_common_handle_conn_state(uint8_t conn_id, T_GAP_CONN_STATE new_state,
                                      uint16_t disc_cause)
{
    APP_PRINT_INFO4("app_ble_common_handle_conn_state: conn_id %d old_state %d new_state %d, disc_cause 0x%x",
                    conn_id, RtkWristbandSys.gap_conn_state, new_state, disc_cause);
    switch (new_state)
    {
    case GAP_CONN_STATE_DISCONNECTED:
        {
            if ((disc_cause != (HCI_ERR | HCI_ERR_REMOTE_USER_TERMINATE))
                && (disc_cause != (HCI_ERR | HCI_ERR_LOCAL_HOST_TERMINATE)))
            {
                APP_PRINT_ERROR1("app_ble_common_handle_conn_state: connection lost cause 0x%x", disc_cause);
            }

            if (dfu_switch_to_ota_mode_pending)
            {
                dfu_switch_to_ota_mode();
            }
            else
            {
                if (dfu_active_reset_pending)
                {
                    dfu_fw_reboot(RESET_ALL, DFU_ACTIVE_RESET);
                }
                else
                {
                    app_ble_common_adv_start(0);
                }

            }
            app_transfer_queue_reset(CMD_PATH_LE);
            le_common_adv.conn_id = 0xFF;
        }
        break;

    case GAP_CONN_STATE_CONNECTED:
        {
            uint16_t conn_interval;
            uint16_t conn_latency;
            uint16_t conn_supervision_timeout;
            uint8_t  remote_bd[6];

            RtkWristbandSys.wristband_conn_id = conn_id;
            T_GAP_REMOTE_ADDR_TYPE remote_bd_type;
            T_GAP_LOCAL_ADDR_TYPE local_bd_type;

            le_get_conn_param(GAP_PARAM_CONN_INTERVAL, &conn_interval, conn_id);
            le_get_conn_param(GAP_PARAM_CONN_LATENCY, &conn_latency, conn_id);
            le_get_conn_param(GAP_PARAM_CONN_TIMEOUT, &conn_supervision_timeout, conn_id);
            le_get_conn_addr(conn_id, remote_bd, &remote_bd_type);
            le_get_conn_param(GAP_PARAM_CONN_LOCAL_BD_TYPE, &local_bd_type, conn_id);
            APP_PRINT_INFO5("GAP_CONN_STATE_CONNECTED:remote_bd %s, remote_addr_type %d, conn_interval 0x%x, conn_latency 0x%x, conn_supervision_timeout 0x%x",
                            TRACE_BDADDR(remote_bd), remote_bd_type,
                            conn_interval, conn_latency, conn_supervision_timeout);
#if CONFIG_SC_KEY_DERIVE
            le_bond_pair(conn_id);
#endif
        }
        break;

    default:
        break;
    }
    RtkWristbandSys.gap_conn_state = new_state;
}

void app_ble_common_handle_authen_state(uint8_t conn_id, uint8_t new_state, uint16_t cause)
{
    APP_PRINT_INFO2("app_ble_common_handle_authen_state:conn_id %d, cause 0x%x", conn_id, cause);

    switch (new_state)
    {
    case GAP_AUTHEN_STATE_STARTED:
        {
            APP_PRINT_INFO0("app_ble_common_handle_authen_state: GAP_AUTHEN_STATE_STARTED");
        }
        break;

    case GAP_AUTHEN_STATE_COMPLETE:
        {
            if (cause == GAP_SUCCESS)
            {
#if CONFIG_REALTEK_SUBSYS_GATT_PROFILE_ANCS_CLIENT
                gatt_client_start_discovery_all(le_get_conn_handle(conn_id), NULL);
#endif
                APP_PRINT_INFO0("app_ble_common_handle_authen_state: GAP_AUTHEN_STATE_COMPLETE pair success");
            }
            else
            {
                APP_PRINT_INFO0("app_ble_common_handle_authen_state: GAP_AUTHEN_STATE_COMPLETE pair failed");
            }
        }
        break;

    default:
        {
            APP_PRINT_ERROR1("app_ble_common_handle_authen_state: unknown newstate %d", new_state);
        }
        break;
    }
}

void app_ble_common_handle_conn_mtu_info(uint8_t conn_id, uint16_t mtu_size)
{
    APP_PRINT_INFO2("app_ble_common_handle_conn_mtu_info: conn_id %d, mtu_size %d", conn_id, mtu_size);
    RtkWristbandSys.wristband_mtu = mtu_size;
}

void app_ble_common_handle_conn_param_update(uint8_t conn_id, uint8_t status, uint16_t cause)
{
    switch (status)
    {
    case GAP_CONN_PARAM_UPDATE_STATUS_SUCCESS:
        {
            uint16_t conn_interval;
            uint16_t conn_slave_latency;
            uint16_t conn_supervision_timeout;

            le_get_conn_param(GAP_PARAM_CONN_INTERVAL, &conn_interval, conn_id);
            le_get_conn_param(GAP_PARAM_CONN_LATENCY, &conn_slave_latency, conn_id);
            le_get_conn_param(GAP_PARAM_CONN_TIMEOUT, &conn_supervision_timeout, conn_id);
            APP_PRINT_INFO3("app_ble_common_handle_conn_param_update update success:conn_interval 0x%x, conn_slave_latency 0x%x, conn_supervision_timeout 0x%x",
                            conn_interval, conn_slave_latency, conn_supervision_timeout);
        }
        break;

    case GAP_CONN_PARAM_UPDATE_STATUS_FAIL:
        {
            APP_PRINT_ERROR1("app_ble_common_handle_conn_param_update update failed: cause 0x%x", cause);
        }
        break;

    case GAP_CONN_PARAM_UPDATE_STATUS_PENDING:
        {
            APP_PRINT_INFO0("app_ble_common_handle_conn_param_update update pending.");
        }
        break;

    default:
        break;
    }
}

static void app_ble_common_adv_callback(uint8_t cb_type, void *p_cb_data)
{
    T_BLE_EXT_ADV_CB_DATA cb_data;
    memcpy(&cb_data, p_cb_data, sizeof(T_BLE_EXT_ADV_CB_DATA));
    switch (cb_type)
    {
    case BLE_EXT_ADV_STATE_CHANGE:
        {
            APP_PRINT_TRACE2("app_ble_common_adv_callback: adv_state %d, adv_handle %d",
                             cb_data.p_ble_state_change->state, cb_data.p_ble_state_change->adv_handle);
            le_common_adv.state = cb_data.p_ble_state_change->state;
            if (le_common_adv.state == BLE_EXT_ADV_MGR_ADV_ENABLED)
            {
            }
            else
            {
                switch (cb_data.p_ble_state_change->stop_cause)
                {
                case BLE_EXT_ADV_STOP_CAUSE_APP:
                    break;

                case BLE_EXT_ADV_STOP_CAUSE_CONN:
                    break;

                case BLE_EXT_ADV_STOP_CAUSE_TIMEOUT:

                    break;

                default:
                    break;
                }
                APP_PRINT_TRACE2("app_ble_common_adv_callback: stack stop adv cause 0x%x, app stop adv cause 0x%02x",
                                 cb_data.p_ble_state_change->stop_cause, cb_data.p_ble_state_change->app_cause);
            }
        }
        break;

    case BLE_EXT_ADV_SET_CONN_INFO:
        {
            if (le_common_adv.adv_handle == cb_data.p_ble_conn_info->adv_handle)
            {
                le_common_adv.conn_id = cb_data.p_ble_conn_info->conn_id;
            }
            APP_PRINT_TRACE4("app_ble_common_adv_callback: BLE_EXT_ADV_SET_CONN_INFO conn_id 0x%x, adv_handle %d, local_addr_type %d, local_bd %s",
                             cb_data.p_ble_conn_info->conn_id,
                             cb_data.p_ble_conn_info->adv_handle,
                             cb_data.p_ble_conn_info->local_addr_type,
                             TRACE_BDADDR(cb_data.p_ble_conn_info->local_addr));
        }
        break;

    default:
        break;
    }
}

void app_ble_common_adv_set_param(void)
{
    /* update adv data with device name & bt address */
    uint8_t le_name_len;
    uint8_t bt_bd_addr[6] ;

    gap_get_param(GAP_PARAM_BD_ADDR, bt_bd_addr);

    le_name_len = strlen((char *)app_cfg_const.device_name_le_default);

    adv_data[11] = bt_bd_addr[5];
    adv_data[12] = bt_bd_addr[4];
    adv_data[13] = bt_bd_addr[3];
    adv_data[14] = bt_bd_addr[2];
    adv_data[15] = bt_bd_addr[1];
    adv_data[16] = bt_bd_addr[0];

    adv_data[17] = le_name_len + 1;
    if (le_name_len >= ADV_DATA_MAX_LE_NAME_LEN)
    {
        adv_data[17] = ADV_DATA_MAX_LE_NAME_LEN + 1;
    }

    memset(&adv_data[19], 0x00, ADV_DATA_MAX_LE_NAME_LEN);
    memcpy(&adv_data[19], app_cfg_nv.device_name_le, adv_data[17] - 1);

    le_set_gap_param(GAP_PARAM_DEVICE_NAME, adv_data[17] - 1, &adv_data[19]);

    /* set advtising parameters */
    T_LE_EXT_ADV_LEGACY_ADV_PROPERTY adv_event_prop = LE_EXT_ADV_LEGACY_ADV_CONN_SCAN_UNDIRECTED;
    uint16_t adv_interval_min = DEFAULT_ADVERTISING_INTERVAL_MIN;
    uint16_t adv_interval_max = DEFAULT_ADVERTISING_INTERVAL_MAX;
    T_GAP_LOCAL_ADDR_TYPE own_address_type = GAP_LOCAL_ADDR_LE_PUBLIC;//GAP_LOCAL_ADDR_LE_PUBLIC

    T_GAP_REMOTE_ADDR_TYPE peer_address_type = GAP_REMOTE_ADDR_LE_PUBLIC;
    uint8_t  peer_address[6] = {0, 0, 0, 0, 0, 0};
    T_GAP_ADV_FILTER_POLICY filter_policy = GAP_ADV_FILTER_ANY;


    ble_ext_adv_mgr_init_adv_params(&le_common_adv.adv_handle, adv_event_prop, adv_interval_min,
                                    adv_interval_max, own_address_type, peer_address_type, peer_address,
                                    filter_policy, 19 + adv_data[17] - 1, adv_data,
                                    sizeof(scan_rsp_data), scan_rsp_data, NULL);

    ble_ext_adv_mgr_register_callback(app_ble_common_adv_callback, le_common_adv.adv_handle);

    APP_PRINT_INFO1("le_common_adv.adv_handle = 0x%x", le_common_adv.adv_handle);

    APP_PRINT_TRACE1("app_set_adv_param: bt_bd_addr %s", TRACE_BDADDR(&adv_data[11]));
    APP_PRINT_TRACE1("app_set_adv_param: device_name %s", TRACE_STRING(&adv_data[19]));
}

bool app_ble_common_adv_start(uint16_t duration_10ms)
{
    if (le_common_adv.state == BLE_EXT_ADV_MGR_ADV_DISABLED)
    {
        if (ble_ext_adv_mgr_enable(le_common_adv.adv_handle, duration_10ms) == GAP_CAUSE_SUCCESS)
        {
            DBG_DIRECT("common adv start success");
            return true;
        }
        else
        {
            DBG_DIRECT("common adv start fail");
            return false;
        }
    }
    else
    {
        APP_PRINT_TRACE0("app_ble_common_adv_start: Already started");
        return true;
    }
}

bool app_ble_common_adv_stop(int8_t app_cause)
{
    APP_PRINT_INFO0("app_ble_common_adv_stop");
    if (ble_ext_adv_mgr_disable(le_common_adv.adv_handle, app_cause) == GAP_CAUSE_SUCCESS)
    {
        return true;
    }
    else
    {
        return false;
    }
}
