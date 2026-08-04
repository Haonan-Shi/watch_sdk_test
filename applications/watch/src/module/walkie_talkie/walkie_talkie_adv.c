/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "gap_le_types.h"
#include "gap.h"
#include "string.h"
#include "gap_le.h"
#include "gap_conn_le.h"
#include "ble_ext_adv.h"
#include "bt_gatt_client.h"
#include "os_queue.h"
#include "ble_mgr.h"
#include "gap_lib_common.h"
#include "audio_track.h"
#include "audio.h"
#include "trace.h"
#include <stdio.h>
#include "os_mem.h"
#include "event_bus.h"
#include "walkie_talkie_voice.h"
#include "walkie_talkie_gatt_svc.h"
#include "walkie_talkie_adv.h"
#include "walkie_talkie_app.h"
#include "walkie_talkie_scan.h"

#define DEFAULT_ADVERTISING_INTERVAL_MIN            160
/** @brief  Default maximum advertising interval */
#define DEFAULT_ADVERTISING_INTERVAL_MAX            160

T_GAP_DEV_STATE  walkie_talkie_gap_state;

uint8_t transmit_adv_data[WALKIE_TALKIE_ADV_LEN] =
{
    /* Service */
    17,             /* length     */
    GAP_ADTYPE_128BIT_MORE,            /* type="Complete 128-bit UUIDs available" */
    GATT_UUID128_WALKIE_TALKIE_SERVICE,

    /* Manufacture specified data*/
    0x09,           /* length     */
    0xFF,           /* type: manufacture specific data*/
    0x01, 0x00,     /* group id */
    0x00, 0x00,     /* user id */
    0x00,           /* status: request/speaking/release */
    0x0B,           /* user name len */
    'U', 's', 'e', 'r', ' ', '0', '0', '0', '0', '0', '0',/* user name (max len = 20) */
    0x05,           /* voice data number */
    /* voice data */
};

void walkie_talkie_handle_dev_state(T_GAP_DEV_STATE new_state, uint16_t cause);
void walkie_talkie_handle_conn_state(uint8_t conn_id, T_GAP_CONN_STATE new_state,
                                     uint16_t disc_cause);
void walkie_talkie_handle_authen_state(uint8_t conn_id, uint8_t new_state, uint16_t cause);
void walkie_talkie_handle_conn_mtu_info(uint8_t conn_id, uint16_t mtu_size);
void walkie_talkie_handle_conn_param_update(uint8_t conn_id, uint8_t status, uint16_t cause);

const T_FUN_GAP_MSG_CBS  transmit_adv_gap_msg_cbs =
{
    .device_state_cb = walkie_talkie_handle_dev_state,
    .conn_state_cb = walkie_talkie_handle_conn_state,
    .authen_state_cb = walkie_talkie_handle_authen_state,
    .mtu_info_cb = walkie_talkie_handle_conn_mtu_info,
    .conn_param_cb = walkie_talkie_handle_conn_param_update,
};

T_LE_ADV_CONN transmit_adv =
{
    .adv_handle = 0xff,
    .conn_id = 0xff,
    .role = GAP_LINK_ROLE_SLAVE,
    .remote_bd = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
    .state = BLE_EXT_ADV_MGR_ADV_DISABLED,
    .cb_tbl = &transmit_adv_gap_msg_cbs,
};

void walkie_talkie_handle_dev_state(T_GAP_DEV_STATE new_state, uint16_t cause)
{
    APP_PRINT_INFO3("walkie_talkie_handle_dev_state: init state %d, adv state %d, cause 0x%x",
                    new_state.gap_init_state, new_state.gap_adv_state, cause);

    if (walkie_talkie_gap_state.gap_init_state != new_state.gap_init_state)
    {
        if (new_state.gap_init_state == GAP_INIT_STATE_STACK_READY)
        {
            /* This case is only for demo, user can init walkie talkie related functions after gui triggers*/
            APP_PRINT_INFO0("GAP stack ready: walkie talk case");
            // Vendor cmd for coded phy set.
            // Opcode: 0xFD80, len = 2
            // param1 (sub_cmd): 0x12
            // param2 (ci): 2 -> CI_8
            //              3 -> CI_2
            uint8_t param[2] = {0x12, 0x02};
            gap_vendor_cmd_req(0xFD80, 2, param);
            walkie_talkie_init();
        }
    }
    walkie_talkie_gap_state = new_state;
}

void walkie_talkie_handle_conn_state(uint8_t conn_id, T_GAP_CONN_STATE new_state,
                                     uint16_t disc_cause)
{
    APP_PRINT_INFO3("walkie_talkie_handle_conn_state: conn_id %d new_state %d, disc_cause 0x%x",
                    conn_id, new_state, disc_cause);
    switch (new_state)
    {
    case GAP_CONN_STATE_DISCONNECTED:
        {
            if ((disc_cause != (HCI_ERR | HCI_ERR_REMOTE_USER_TERMINATE))
                && (disc_cause != (HCI_ERR | HCI_ERR_LOCAL_HOST_TERMINATE)))
            {
                APP_PRINT_ERROR1("walkie_talkie_handle_conn_state: connection lost cause 0x%x", disc_cause);
            }

            transmit_adv.conn_id = 0xFF;
            memset(transmit_adv.remote_bd, 0xFF, 6);
            event_bus_publish(EVENT_BUS_TOPIC_WALKIE_TALKIE_DISCONNECTED, NULL, 0);
        }
        break;

    case GAP_CONN_STATE_CONNECTED:
        {
            uint16_t conn_interval;
            uint16_t conn_latency;
            uint16_t conn_supervision_timeout;
            uint8_t  remote_bd[6];
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

            transmit_adv.conn_id = conn_id;

            if (transmit_adv.role == GAP_LINK_ROLE_MASTER)
            {
                uint16_t tx_octets = 251;
                uint16_t tx_time = 2120;
                le_set_data_len(conn_id, tx_octets, tx_time);
            }
            else if (transmit_adv.role == GAP_LINK_ROLE_SLAVE)
            {
                walkie_talkie_scan_stop();
                walkie_talkie_adv_stop(0);
            }
            event_bus_publish(EVENT_BUS_TOPIC_WALKIE_TALKIE_CONNECTED, NULL, 0);

        }
        break;

    default:
        break;
    }
}

void walkie_talkie_handle_authen_state(uint8_t conn_id, uint8_t new_state, uint16_t cause)
{
    APP_PRINT_INFO2("walkie_talkie_handle_authen_state:conn_id %d, cause 0x%x", conn_id, cause);

    switch (new_state)
    {
    case GAP_AUTHEN_STATE_STARTED:
        {
            APP_PRINT_INFO0("walkie_talkie_handle_authen_state: GAP_AUTHEN_STATE_STARTED");
        }
        break;

    case GAP_AUTHEN_STATE_COMPLETE:
        {
            if (cause == GAP_SUCCESS)
            {
                APP_PRINT_INFO0("walkie_talkie_handle_authen_state: GAP_AUTHEN_STATE_COMPLETE pair success");
            }
            else
            {
                APP_PRINT_INFO0("walkie_talkie_handle_authen_state: GAP_AUTHEN_STATE_COMPLETE pair failed");
            }
        }
        break;

    default:
        {
            APP_PRINT_ERROR1("walkie_talkie_handle_authen_state: unknown newstate %d", new_state);
        }
        break;
    }
}

void walkie_talkie_handle_conn_mtu_info(uint8_t conn_id, uint16_t mtu_size)
{
    APP_PRINT_INFO2("walkie_talkie_handle_conn_mtu_info: conn_id %d, mtu_size %d", conn_id, mtu_size);
    if (transmit_adv.role == GAP_LINK_ROLE_MASTER)
    {
        gatt_client_start_discovery_all(le_get_conn_handle(conn_id), NULL);
    }
}

void walkie_talkie_handle_conn_param_update(uint8_t conn_id, uint8_t status, uint16_t cause)
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
            APP_PRINT_INFO3("walkie_talkie_handle_conn_param_update update success:conn_interval 0x%x, conn_slave_latency 0x%x, conn_supervision_timeout 0x%x",
                            conn_interval, conn_slave_latency, conn_supervision_timeout);
        }
        break;

    case GAP_CONN_PARAM_UPDATE_STATUS_FAIL:
        {
            APP_PRINT_ERROR1("walkie_talkie_handle_conn_param_update update failed: cause 0x%x", cause);
        }
        break;

    case GAP_CONN_PARAM_UPDATE_STATUS_PENDING:
        {
            APP_PRINT_INFO0("walkie_talkie_handle_conn_param_update update pending.");
        }
        break;

    default:
        break;
    }
}

bool walkie_talkie_transmitter_working(void)
{
    return false;
}

static void walkie_talkie_adv_callback(uint8_t cb_type, void *p_cb_data)
{
    T_BLE_EXT_ADV_CB_DATA cb_data;
    memcpy(&cb_data, p_cb_data, sizeof(T_BLE_EXT_ADV_CB_DATA));
    switch (cb_type)
    {
    case BLE_EXT_ADV_STATE_CHANGE:
        {
            APP_PRINT_TRACE2("walkie_talkie_adv_callback: adv_state %d, adv_handle %d",
                             cb_data.p_ble_state_change->state, cb_data.p_ble_state_change->adv_handle);
            transmit_adv.state = cb_data.p_ble_state_change->state;
            if (transmit_adv.state == BLE_EXT_ADV_MGR_ADV_ENABLED)
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
                    {
                        walkie_talkie_recorder_set_voice_data();
                        ble_ext_adv_mgr_set_adv_data(transmit_adv.adv_handle,
                                                     VOICE_DATA_OFFSET + transmit_adv_data[VOICE_NUM_OFFSET] * 21, transmit_adv_data);
                        walkie_talkie_adv_start(16);
                    }
                    break;

                default:
                    break;
                }
                APP_PRINT_TRACE2("walkie_talkie_adv_callback: stack stop adv cause 0x%x, app stop adv cause 0x%02x",
                                 cb_data.p_ble_state_change->stop_cause, cb_data.p_ble_state_change->app_cause);
            }
        }
        break;

    case BLE_EXT_ADV_SET_CONN_INFO:
        if (transmit_adv.adv_handle == cb_data.p_ble_conn_info->adv_handle)
        {
            transmit_adv.conn_id = cb_data.p_ble_conn_info->conn_id;
        }
        APP_PRINT_TRACE4("walkie_talkie_adv_callback: BLE_EXT_ADV_SET_CONN_INFO conn_id 0x%x, adv_handle %d, local_addr_type %d, local_bd %b",
                         cb_data.p_ble_conn_info->conn_id,
                         cb_data.p_ble_conn_info->adv_handle,
                         cb_data.p_ble_conn_info->local_addr_type,
                         TRACE_BDADDR(cb_data.p_ble_conn_info->local_addr));
        break;

    default:
        break;
    }
}

void walkie_talkie_adv_set_param(void)
{
    /* update adv data with device name */
    uint8_t bt_bd_addr[6] ;
    uint8_t temp_str[7];
    gap_get_param(GAP_PARAM_BD_ADDR, bt_bd_addr);
    snprintf(&temp_str[0], sizeof(temp_str), "%02X%02X%02X", bt_bd_addr[3], bt_bd_addr[4],
             bt_bd_addr[5]);
    memcpy(&transmit_adv_data[31], temp_str, 6);

    /* set advtising parameters */
    T_LE_EXT_ADV_EXTENDED_ADV_PROPERTY adv_event_prop =
        LE_EXT_ADV_EXTENDED_ADV_CONN_UNDIRECTED;
    uint16_t adv_interval_min = DEFAULT_ADVERTISING_INTERVAL_MIN;
    uint16_t adv_interval_max = DEFAULT_ADVERTISING_INTERVAL_MAX;
    T_GAP_LOCAL_ADDR_TYPE own_address_type = GAP_LOCAL_ADDR_LE_PUBLIC;
    T_GAP_REMOTE_ADDR_TYPE peer_address_type = GAP_REMOTE_ADDR_LE_PUBLIC;
    uint8_t  peer_address[6] = {0, 0, 0, 0, 0, 0};
    T_GAP_ADV_FILTER_POLICY filter_policy = GAP_ADV_FILTER_ANY;

    ble_ext_adv_mgr_init_adv_params(&transmit_adv.adv_handle, adv_event_prop, adv_interval_min,
                                    adv_interval_max, own_address_type, peer_address_type, peer_address,
                                    filter_policy, VOICE_NUM_OFFSET, transmit_adv_data,
                                    0, NULL, NULL);

    ble_ext_adv_mgr_register_callback(walkie_talkie_adv_callback, transmit_adv.adv_handle);
    ble_ext_adv_mgr_change_adv_phy(transmit_adv.adv_handle, GAP_PHYS_PRIM_ADV_CODED, GAP_PHYS_CODED);
    APP_PRINT_TRACE1("transmit_adv.adv_handle = 0x%x", transmit_adv.adv_handle);
}

bool walkie_talkie_adv_start(uint16_t duration_10ms)
{
    if (transmit_adv.state == BLE_EXT_ADV_MGR_ADV_DISABLED)
    {
        if (ble_ext_adv_mgr_enable(transmit_adv.adv_handle, duration_10ms) == GAP_CAUSE_SUCCESS)
        {
            APP_PRINT_TRACE0("walkie talkie adv start success");
            return true;
        }
        else
        {
            APP_PRINT_TRACE0("walkie talkie adv start fail");
            return false;
        }
    }
    else
    {
        APP_PRINT_TRACE0("walkie talkie adv: Already started");
        return true;
    }
}

bool walkie_talkie_adv_stop(int8_t app_cause)
{
    APP_PRINT_TRACE0("walkie talkie adv stop");
    if (ble_ext_adv_mgr_disable(transmit_adv.adv_handle, app_cause) == GAP_CAUSE_SUCCESS)
    {
        return true;
    }
    else
    {
        return false;
    }
}

T_GAP_CAUSE walkie_talkie_disconnect(void)
{
    if (transmit_adv.conn_id != 0xFF)
    {
        return le_disconnect(transmit_adv.conn_id);
    }
    else
    {
        return GAP_CAUSE_SUCCESS;
    }
}
