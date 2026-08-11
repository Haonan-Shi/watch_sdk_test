/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <string.h>
#include "fmna_adv_platform.h"
#include "fmna_adv.h"
#include "fmna_battery_platform.h"
#include "fmna_state_machine.h"
#include "fmna_connection_platform.h"
#include "app_cfg.h"
#include "ble_ext_adv.h"
#include "app_findmy.h"
#include "app_ble_adv.h"
#include "gap_vendor.h"
#include "gap_adv.h"

static fmna_pairing_payload_t m_fmna_pairing_adv_payload       = {0};
#define SERVICE_DATA_LEN               (4)

void app_findmy_handle_dev_state(T_GAP_DEV_STATE new_state, uint16_t cause);
void app_findmy_handle_conn_state(uint8_t conn_id, T_GAP_CONN_STATE new_state, uint16_t disc_cause);
void app_findmy_handle_authen_state(uint8_t conn_id, uint8_t new_state, uint16_t cause);
void app_findmy_handle_conn_mtu_info(uint8_t conn_id, uint16_t mtu_size);
void app_findmy_handle_conn_param_update(uint8_t conn_id, uint8_t status, uint16_t cause);

static uint8_t findmy_pairing_adv_data[29] =
{
    /* Flags */
    0x18,             /* length */
    GAP_ADTYPE_SERVICE_DATA, /* type="Flags" */
    LO_WORD(FINDMY_UUID_SERVICE),
    HI_WORD(FINDMY_UUID_SERVICE),
};

const T_FUN_GAP_MSG_CBS  findmy_gap_msg_cbs =
{
    .device_state_cb = app_findmy_handle_dev_state,
    .conn_state_cb = app_findmy_handle_conn_state,
    .authen_state_cb = app_findmy_handle_authen_state,
    .mtu_info_cb = app_findmy_handle_conn_mtu_info,
    .conn_param_cb = app_findmy_handle_conn_param_update,
};

T_LE_ADV_CONN le_findmy_adv =
{
    .adv_handle = 0xFF,
    .conn_id = 0xFF,
    .role = GAP_LINK_ROLE_SLAVE,
    .state = BLE_EXT_ADV_MGR_ADV_DISABLED,
    .cb_tbl = &findmy_gap_msg_cbs,
};

void app_findmy_handle_dev_state(T_GAP_DEV_STATE new_state, uint16_t cause)
{
    APP_PRINT_INFO3("app_findmy_handle_dev_state: init state %d, adv state %d, cause 0x%x",
                    new_state.gap_init_state, new_state.gap_adv_state, cause);
    if (new_state.gap_init_state == GAP_INIT_STATE_STACK_READY)
    {
        gap_vendor_le_set_host_feature(5, 1);
        app_findmy_adv_init();
        fmna_bond_info_restore();
        fmna_crypto_init();
        fmna_log_serial_number();
        le_adv_read_tx_power();
        /*stack ready*/
        fmna_state_machine_init();
    }
}

void app_findmy_handle_conn_state(uint8_t conn_id, T_GAP_CONN_STATE new_state, uint16_t disc_cause)
{
    APP_PRINT_INFO3("app_findmy_handle_conn_state: conn_id %d new_state %d, disc_cause 0x%x",
                    conn_id, new_state, disc_cause);
    switch (new_state)
    {
    case GAP_CONN_STATE_DISCONNECTED:
        {
            fmna_handle_ble_evt(FMNA_DISCONNECTED, conn_id);
        }
        break;

    case GAP_CONN_STATE_CONNECTED:
        {
            fmna_handle_ble_evt(FMNA_CONNECTED, conn_id);
        }
        break;

    default:
        break;
    }
}

void app_findmy_handle_authen_state(uint8_t conn_id, uint8_t new_state, uint16_t cause)
{
    APP_PRINT_INFO2("app_findmy_handle_authen_state:conn_id %d, cause 0x%x", conn_id, cause);

    switch (new_state)
    {
    case GAP_AUTHEN_STATE_STARTED:
        {
            APP_PRINT_INFO0("app_findmy_handle_authen_state: GAP_AUTHEN_STATE_STARTED");
        }
        break;

    case GAP_AUTHEN_STATE_COMPLETE:
        {
            if (cause == GAP_SUCCESS)
            {
                fmna_handle_ble_evt(FMNA_AUTHEN_SUCCESS, conn_id);
                APP_PRINT_INFO0("app_findmy_handle_authen_state: GAP_AUTHEN_STATE_COMPLETE pair success");
            }
            else
            {
                APP_PRINT_INFO0("app_findmy_handle_authen_state: GAP_AUTHEN_STATE_COMPLETE pair failed");
            }
        }
        break;

    default:
        {
            APP_PRINT_ERROR1("app_findmy_handle_authen_state: unknown newstate %d", new_state);
        }
        break;
    }
}

void app_findmy_handle_conn_mtu_info(uint8_t conn_id, uint16_t mtu_size)
{
    APP_PRINT_INFO2("app_findmy_handle_conn_mtu_info: conn_id %d, mtu_size %d", conn_id, mtu_size);
}

void app_findmy_handle_conn_param_update(uint8_t conn_id, uint8_t status, uint16_t cause)
{
    fmna_handle_ble_evt(FMNA_CONN_PARAM_UPDATE, conn_id);
}

/// Fills Pairing payload according to ADV spec.
static void app_findmy_pairing_adv_service_data_init(void)
{
    uint8_t product_data[PRODUCT_DATA_BLEN] = PRODUCT_DATA_VAL;

    memcpy(m_fmna_pairing_adv_payload.product_data,
           product_data,
           PRODUCT_DATA_BLEN);

    memset(m_fmna_pairing_adv_payload.acc_category, ACCESSORY_CATEGORY, sizeof(uint8_t));
    m_fmna_pairing_adv_payload.battery_state = fmna_battery_platform_get_battery_level();
}

static void app_findmy_adv_callback(uint8_t cb_type, void *p_cb_data)
{
    T_BLE_EXT_ADV_CB_DATA cb_data;

    memcpy(&cb_data, p_cb_data, sizeof(T_BLE_EXT_ADV_CB_DATA));
    switch (cb_type)
    {
    case BLE_EXT_ADV_STATE_CHANGE:
        {
            le_findmy_adv.state = cb_data.p_ble_state_change->state;
            APP_PRINT_TRACE2("app_findmy_adv_callback: BLE_EXT_ADV_STATE_CHANGE, adv_state %d, adv_handle %d",
                             le_findmy_adv.state, cb_data.p_ble_state_change->adv_handle);
            if (le_findmy_adv.state == BLE_EXT_ADV_MGR_ADV_DISABLED)
            {
                APP_PRINT_TRACE2("app_findmy_adv_callback: stop_cause %d, app_cause 0x%02x",
                                 cb_data.p_ble_state_change->stop_cause,
                                 cb_data.p_ble_state_change->app_cause);
            }
        }
        break;

    case BLE_EXT_ADV_SET_CONN_INFO:
        {
            if (cb_data.p_ble_conn_info->adv_handle == le_findmy_adv.adv_handle)
            {
                le_findmy_adv.conn_id = cb_data.p_ble_conn_info->conn_id;
            }

            APP_PRINT_TRACE3("app_findmy_adv_callback: BLE_EXT_ADV_SET_CONN_INFO conn_id 0x%x, adv_handle %d, finmdy_adv_handle %d",
                             cb_data.p_ble_conn_info->conn_id, cb_data.p_ble_conn_info->adv_handle,
                             le_findmy_adv.adv_handle);
        }
        break;

    default:
        break;
    }
    return;
}

bool app_findmy_is_findmy_link(uint8_t conn_id)
{
    return le_findmy_adv.conn_id == conn_id;
}

void app_findmy_disconnect(void)
{
    le_findmy_adv.conn_id = 0xFF;
}
bool app_findmy_adv_start(uint16_t timeout_sec)
{
    FMNA_LOG_INFO("app_findmy_adv_start: timeout_sec %d", timeout_sec);

    if (le_findmy_adv.state == BLE_EXT_ADV_MGR_ADV_DISABLED)
    {
        if (ble_ext_adv_mgr_enable(le_findmy_adv.adv_handle, timeout_sec) == GAP_CAUSE_SUCCESS)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        APP_PRINT_TRACE0("app_findmy_adv_start: Already started");
        return true;
    }
}

bool app_findmy_adv_stop(uint8_t app_cause)
{
    if (le_findmy_adv.state == BLE_EXT_ADV_MGR_ADV_ENABLED)
    {
        if (ble_ext_adv_mgr_disable(le_findmy_adv.adv_handle, app_cause) == GAP_CAUSE_SUCCESS)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        APP_PRINT_TRACE0("app_findmy_adv_stop: Already stoped");
        return true;
    }
}

uint8_t app_findmy_adv_get_adv_handle(void)
{
    return le_findmy_adv.adv_handle;
}

void app_findmy_enter_pair_mode(void)
{
    APP_PRINT_INFO0("app_findmy_enter_pair_mode");
    start_pair_adv();
}


void app_findmy_adv_init(void)
{
    T_LE_EXT_ADV_LEGACY_ADV_PROPERTY adv_event_prop = LE_EXT_ADV_LEGACY_ADV_CONN_SCAN_UNDIRECTED;
    uint16_t adv_interval_min = 0x30;
    uint16_t adv_interval_max = 0x30;
    T_GAP_LOCAL_ADDR_TYPE own_address_type = GAP_LOCAL_ADDR_LE_RANDOM;
    T_GAP_REMOTE_ADDR_TYPE peer_address_type = GAP_REMOTE_ADDR_LE_PUBLIC;
    uint8_t  peer_address[6] = {0, 0, 0, 0, 0, 0};
    T_GAP_ADV_FILTER_POLICY filter_policy = GAP_ADV_FILTER_ANY;
    uint8_t random_addr[6] = {0};

    gap_get_param(GAP_PARAM_BD_ADDR, random_addr);
    random_addr[5] |= (uint8_t)FMNA_ADV_ADDR_TYPE_MASK;

    app_findmy_pairing_adv_service_data_init();
    memcpy(findmy_pairing_adv_data + SERVICE_DATA_LEN, (uint8_t *)&m_fmna_pairing_adv_payload,
           sizeof(m_fmna_pairing_adv_payload));

    ble_ext_adv_mgr_init_adv_params(&le_findmy_adv.adv_handle, adv_event_prop, adv_interval_min,
                                    adv_interval_max, own_address_type, peer_address_type, peer_address,
                                    filter_policy, 25, (uint8_t *)findmy_pairing_adv_data,
                                    0, NULL,
                                    random_addr);

    APP_PRINT_TRACE2("app_findmy_adv_init: findmy_pairing_adv_data %b,random address %s",
                     TRACE_BINARY(sizeof(findmy_pairing_adv_data), findmy_pairing_adv_data), TRACE_BDADDR(random_addr));

    ble_ext_adv_mgr_register_callback(app_findmy_adv_callback, le_findmy_adv.adv_handle);
}
