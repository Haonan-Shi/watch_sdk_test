/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if CONFIG_REALTEK_FINDMY_FEATURE_SUPPORT
#include <string.h>
#include "fmna_adv_platform.h"
#include "fmna_adv.h"
#include "fmna_battery_platform.h"
#include "fmna_state_machine.h"
#include "app_ble_rand_addr_mgr.h"
#include "app_adv_stop_cause.h"
#include "app_cfg.h"
#include "ble_ext_adv.h"
#include "trace.h"
#include "bt_bond_le.h"
#include "bt_bond_le_sync.h"
#include "gap_bond_le.h"
#include "app_findmy.h"

bool fmna_findmy_adv = false;
static fmna_pairing_payload_t m_fmna_pairing_adv_payload       = {0};
#define SERVICE_DATA_LEN               (4)

typedef struct t_le_findmy_adv
{
    uint8_t adv_handle;
    uint8_t conn_id;
    T_BLE_EXT_ADV_MGR_STATE state;
} T_LE_FINDMY_ADV;

static T_LE_FINDMY_ADV le_findmy_adv =
{
    .adv_handle = 0xFF,
    .conn_id = 0xFF,
    .state = BLE_EXT_ADV_MGR_ADV_DISABLED,
};

static uint8_t findmy_pairing_adv_data[29] =
{
    /* Flags */
    0x18,             /* length */
    GAP_ADTYPE_SERVICE_DATA, /* type="Flags" */
    LO_WORD(FINDMY_UUID_SERVICE),
    HI_WORD(FINDMY_UUID_SERVICE),
};

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
                fmna_findmy_adv = true;
                le_findmy_adv.conn_id = cb_data.p_ble_conn_info->conn_id;
            }
            else
            {
                fmna_findmy_adv = false;
            }

            APP_PRINT_TRACE4("app_findmy_adv_callback: BLE_EXT_ADV_SET_CONN_INFO conn_id 0x%x, link %d, adv_handle %d, finmdy_adv_handle %d",
                             cb_data.p_ble_conn_info->conn_id, fmna_findmy_adv, cb_data.p_ble_conn_info->adv_handle,
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

bool app_findmy_is_findmy_adv(void)
{
    return fmna_findmy_adv;
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

void app_findmy_ble_bond_get_cb(uint8_t cb_type, void *p_cb_data)
{
    switch (cb_type)
    {
    case BT_BOND_MSG_LE_BOND_GET:
        {
            T_BT_LE_BOND_CB_DATA cb_data;
            T_LE_BOND_ENTRY *p_entry = NULL;

            memcpy(&cb_data, p_cb_data, sizeof(T_BT_LE_BOND_CB_DATA));
            APP_PRINT_INFO5("app_findmy_ble_bond_get_cb: bd_addr %s, bd_type %d, local_bd_addr %s, local_bd_type %d, key_type %d",
                            TRACE_BDADDR(cb_data.p_le_bond_get->bd_addr),
                            cb_data.p_le_bond_get->bd_type,
                            TRACE_BDADDR(cb_data.p_le_bond_get->local_bd_addr),
                            cb_data.p_le_bond_get->local_bd_type,
                            cb_data.p_le_bond_get->key_type);

            p_entry = bt_le_find_key_entry_by_idx(app_global_data.app_bond_idx[FINDMY_APP]);
            if (p_entry != NULL)
            {
                T_BT_LE_LTK le_ltk;
                if (bt_le_dev_info_get_local_ltk(p_entry, (uint8_t *)&le_ltk))
                {
                    cb_data.p_le_bond_get->cfm_cause = GAP_CFM_CAUSE_ACCEPT;
                    cb_data.p_le_bond_get->key_len = le_ltk.link_key_length;
                    memcpy(cb_data.p_le_bond_get->key_data, le_ltk.key, 28);
                }
            }
        }
        break;

    default:
        break;
    }
    return;
}

void app_findmy_ble_bond_sync_init(void)
{
    bt_bond_register_bond_get_cb(app_findmy_ble_bond_get_cb);
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

    APP_PRINT_TRACE2("app_findmy_adv_init: findmy_pairing_adv_data %b,random address %b",
                     TRACE_BINARY(sizeof(findmy_pairing_adv_data), findmy_pairing_adv_data), TRACE_BDADDR(random_addr));

    ble_ext_adv_mgr_register_callback(app_findmy_adv_callback, le_findmy_adv.adv_handle);
}
#endif
