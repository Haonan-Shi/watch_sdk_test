/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if CONFIG_REALTEK_FINDMY_SUPPORT_CUSTOMIZED_APP
#include "trace.h"
#include "gap_bond_le.h"
#include "gap_storage_le.h"
#include "gap_privacy.h"
#include "app_findmy.h"
#include "custom_app.h"
#include "fmna_connection.h"
#include "fmna_adv_platform.h"
#include "app_ble_rand_addr_mgr.h"
#include "app_adv_stop_cause.h"
#include "app_cfg.h"
#include "ble_ext_adv.h"
#include "app_timer.h"
#include "aes_api.h"
#include "bt_bond_le.h"

/*============================================================================*
 *                              Variables
 *============================================================================*/
static T_CUSTOM_DATA custom_data;
static uint8_t cust_adv_handle = 0xFF;
static uint8_t cust_adv_timer_id = 0;
static uint8_t timer_idx_cust_addr_update = 0;
bool cust_adv = false;

typedef enum
{
    APP_TIMER_CUST_ADDR_UPDATE,
} T_APP_CUST_ADV_TIMER;

typedef struct t_le_cust_adv
{
    uint8_t adv_handle;
    uint8_t conn_id;
    T_BLE_EXT_ADV_MGR_STATE state;
} T_LE_CUST_ADV;

static T_LE_CUST_ADV le_cust_adv =
{
    .adv_handle = 0xFF,
    .conn_id = 0xFF,
    .state = BLE_EXT_ADV_MGR_ADV_DISABLED,
};

/* Pairing advertisement data (max size = 31 bytes, though this is
 best kept short to conserve power while advertisting) */
static uint8_t cust_adv_data[31] =
{
    /* Flags */
    0x02,             /* length */
    GAP_ADTYPE_FLAGS, /* type="Flags" */
    GAP_ADTYPE_FLAGS_LIMITED | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,
#if F_APP_BLE_HID_DEVICE_SUPPORT
    /* Service */
    0x03,  /* length */
    0x03,  /* type="More 16-bit UUIDs available" */
    0x12, 0x18,  /* HID Service */
#endif
};

static uint8_t cust_scan_rsp_data[31] =
{
    0x06,//length = 0x02
    GAP_ADTYPE_LOCAL_NAME_COMPLETE,
    'R', 'T', 'K', '_', 'D'
};

uint8_t cust_scan_rsp_data_len;
/*============================================================================*
 *                          Functions
 *============================================================================*/

void cust_adv_timeout_cb(uint8_t timer_evt, uint16_t param)
{
    APP_PRINT_TRACE2("cust_adv_timeout_cb: timer_evt %d, param %d", timer_evt, param);

    switch (timer_evt)
    {
    case APP_TIMER_CUST_ADDR_UPDATE:
        {
            uint8_t cust_bt_addr[GAP_BD_ADDR_LEN];
            if (!custom_data.cust_paired)
            {
                return;
            }

            le_gen_rand_addr(GAP_RAND_ADDR_RESOLVABLE, cust_bt_addr);
            ble_ext_adv_mgr_set_random(le_cust_adv.adv_handle, cust_bt_addr);
            APP_PRINT_TRACE0("cust_adv_timeout_cb: addr update");
        }
        break;

    default:
        break;
    }
}

bool le_privacy_check_resolvable_private_address(uint8_t *rpa, uint8_t *irk)
{
    if ((rpa[5] & 0xC0) == 0x40)
    {
        uint8_t buffer[16] = {0};
        uint8_t key[16];
        uint8_t encrypt_buffer[16];

        buffer[13] = rpa[5];
        buffer[14] = rpa[4];
        buffer[15] = rpa[3];

        swap_buf(irk, key, 16);

        aes128_ecb_encrypt(buffer, key, encrypt_buffer);

        if ((encrypt_buffer[13] == rpa[2]) &&
            (encrypt_buffer[14] == rpa[1]) &&
            (encrypt_buffer[15] == rpa[0]))
        {
            APP_PRINT_INFO0("le_privacy_check_resolvable_private_address: match");
            return true;
        }
    }

    APP_PRINT_ERROR1("le_privacy_check_resolvable_private_address: not match, rpa %s",
                     TRACE_BDADDR(rpa));

    return false;
}

static bool cust_check_if_fmna_owner_device(uint8_t *remote_bd,
                                            T_GAP_REMOTE_ADDR_TYPE remote_bd_type)
{
    bool ret = false;

    if (remote_bd_type != GAP_REMOTE_ADDR_LE_RANDOM)
    {
        return ret;
    }

    uint8_t peer_bd_addr[6];

    memcpy(peer_bd_addr, remote_bd, 6);
    if ((peer_bd_addr[5] & 0xC0) == 0x80)
    {
        peer_bd_addr[5] &= 0x3F;
        peer_bd_addr[5] |= 0x40;
    }

    if ((peer_bd_addr[5] & 0xC0) != 0x40)
    {
        return ret;
    }

    if (fmna_connection_is_fmna_paired())
    {
        T_LE_BOND_ENTRY *findmy_key_entry = NULL;
        findmy_key_entry = bt_le_find_key_entry_by_idx(app_global_data.app_bond_idx[FINDMY_BOND]);
        uint8_t findmy_irk[16];
        ret = bt_le_get_dev_irk(findmy_key_entry, true, findmy_irk);
        if (!ret)
        {
            APP_PRINT_WARN0("cust_check_if_fmna_owner_device: le_get_dev_irk failed");
            return ret;
        }
        ret = le_privacy_check_resolvable_private_address(peer_bd_addr, findmy_irk);
    }
    APP_PRINT_INFO1("cust_check_if_fmna_owner_device ret=%d", ret);
    return ret;
}

static void cust_adv_callback(uint8_t cb_type, void *p_cb_data)
{
    T_BLE_EXT_ADV_CB_DATA cb_data;
    memcpy(&cb_data, p_cb_data, sizeof(T_BLE_EXT_ADV_CB_DATA));
    switch (cb_type)
    {
    case BLE_EXT_ADV_STATE_CHANGE:
        {
            le_cust_adv.state = cb_data.p_ble_state_change->state;
            APP_PRINT_TRACE2("cust_adv_callback: BLE_EXT_ADV_STATE_CHANGE, adv_state %d, adv_handle %d",
                             le_cust_adv.state, cb_data.p_ble_state_change->adv_handle);
            if (le_cust_adv.state == BLE_EXT_ADV_MGR_ADV_DISABLED)
            {
                APP_PRINT_TRACE2("cust_adv_callback: stop_cause %d, app_cause 0x%02x",
                                 cb_data.p_ble_state_change->stop_cause,
                                 cb_data.p_ble_state_change->app_cause);
            }
        }
        break;

    case BLE_EXT_ADV_SET_CONN_INFO:
        {
            if (cb_data.p_ble_conn_info->adv_handle == le_cust_adv.adv_handle)
            {
                cust_adv = true;
                le_cust_adv.conn_id = cb_data.p_ble_conn_info->conn_id;
            }
            else
            {
                cust_adv = false;
            }

            APP_PRINT_TRACE4("cust_adv_callback: BLE_EXT_ADV_SET_CONN_INFO conn_id 0x%x, adv_handle %d, local_addr_type %d, local_bd %b",
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

bool cust_app_is_cust_link(uint8_t conn_id)
{
    return le_cust_adv.conn_id == conn_id;
}

bool cust_app_is_cust_adv(void)
{
    return cust_adv;
}

void cust_app_disconnect(void)
{
    le_cust_adv.conn_id = 0xFF;
}

void cust_adv_init(void)
{
    T_GAP_CAUSE cause;
    T_LE_EXT_ADV_LEGACY_ADV_PROPERTY adv_event_prop = LE_EXT_ADV_LEGACY_ADV_CONN_SCAN_UNDIRECTED;
    uint16_t adv_interval_min = 0xA0;
    uint16_t adv_interval_max = 0xA0;
    T_GAP_LOCAL_ADDR_TYPE own_address_type = GAP_LOCAL_ADDR_LE_RANDOM;
    T_GAP_REMOTE_ADDR_TYPE peer_address_type = GAP_REMOTE_ADDR_LE_PUBLIC;
    uint8_t  peer_address[6] = {0, 0, 0, 0, 0, 0};
    T_GAP_ADV_FILTER_POLICY filter_policy = GAP_ADV_FILTER_ANY;

    uint8_t random_address[6] = {0};
    le_gen_rand_addr(GAP_RAND_ADDR_RESOLVABLE, random_address);

    ble_ext_adv_mgr_init_adv_params(&le_cust_adv.adv_handle, adv_event_prop, adv_interval_min,
                                    adv_interval_max, own_address_type, peer_address_type, peer_address,
                                    filter_policy, sizeof(cust_adv_data), cust_adv_data,
                                    sizeof(cust_scan_rsp_data), cust_scan_rsp_data, random_address);

    cause = ble_ext_adv_mgr_register_callback(cust_adv_callback, le_cust_adv.adv_handle);
    app_timer_reg_cb(cust_adv_timeout_cb, &cust_adv_timer_id);
    if (cause == GAP_CAUSE_SUCCESS)
    {
        APP_PRINT_INFO1("cust_adv_init: adv_handle %d", le_cust_adv.adv_handle);
    }
}


void cust_data_init(void)
{
    memset(&custom_data, 0, sizeof(custom_data));
    custom_data.cust_conn_id = 0xFF;
}

void cust_factory_reset(void)
{
    APP_PRINT_INFO0("cust_factory_reset");
    if (app_global_data.app_bond_idx[CUST_BOND_1] != 0xFF)
    {
        T_GAP_CAUSE ret = le_bond_delete_by_idx(app_global_data.app_bond_idx[CUST_BOND_1]);
        APP_PRINT_INFO2("cust_factory_reset: delete CUST_BOND_1 idx %d, ret %d",
                        app_global_data.app_bond_idx[CUST_BOND_1],
                        ret);
    }
    if (app_global_data.app_bond_idx[CUST_BOND_2] != 0xFF)
    {
        T_GAP_CAUSE ret = le_bond_delete_by_idx(app_global_data.app_bond_idx[CUST_BOND_2]);
        APP_PRINT_INFO2("cust_factory_reset: delete CUST_BOND_2 idx %d, ret %d",
                        app_global_data.app_bond_idx[CUST_BOND_2],
                        ret);
    }
    cust_feature_disable();
}

void cust_handle_connected_evt(uint8_t conn_id, uint8_t *remote_bd,
                               T_GAP_REMOTE_ADDR_TYPE remote_bd_type)
{
    APP_PRINT_INFO3("cust_handle_connected_evt: conn_id %d, remote_bd %s, remote_bd_type %d", conn_id,
                    TRACE_BDADDR(remote_bd), remote_bd_type);
    //cust_adv_stop(APP_STOP_ADV_CAUSE_FINDMY);
    custom_data.cust_conn_id = conn_id;

    bool ret = cust_check_if_fmna_owner_device(remote_bd, remote_bd_type);
    if (ret)
    {
        cust_adv_update_device_name(false);
    }
}

bool cust_handle_disconnected_evt(uint8_t conn_id, uint16_t disc_cause)
{
    if (custom_data.cust_conn_id == conn_id)
    {
        APP_PRINT_INFO1("cust_handle_disconnected_evt: conn_id %d", conn_id);
        cust_app_disconnect();
        custom_data.cust_conn_id = 0xFF;
        if (disc_cause != (HCI_ERR | HCI_ERR_LOCAL_HOST_TERMINATE))
        {
            if (fmna_connection_is_fmna_paired())
            {
                cust_adv_update_device_name(false);
            }
            cust_adv_start(0);
        }
        return true;
    }
    else
    {
        return false;
    }
}

void cust_set_paired_flag(bool flag)
{
    custom_data.cust_paired = flag;
}

bool cust_is_paired(void)
{
    return custom_data.cust_paired;
}

bool cust_feature_is_enabled(void)
{
    return custom_data.cust_enable;
}

void cust_feature_enable(void)
{
    custom_data.cust_enable = true;
    APP_PRINT_INFO0("cust_feature_enable");

    if (fmna_connection_is_fmna_paired())
    {
        cust_adv_update_device_name(true);
    }
    else
    {
        cust_adv_update_device_name(false);
    }

    //start cust adv
    cust_adv_start(0);

    app_start_timer(&timer_idx_cust_addr_update, "cust_addr_update",
                    cust_adv_timer_id, APP_TIMER_CUST_ADDR_UPDATE, 0, true,
                    15 * 60000);
}

void cust_feature_disable(void)
{
    custom_data.cust_enable = false;
    APP_PRINT_INFO0("cust_feature_disable");

    cust_adv_stop(APP_STOP_ADV_CAUSE_FINDMY);

    if (custom_data.cust_conn_id != 0xFF)
    {
        le_disconnect(custom_data.cust_conn_id);
    }

    app_stop_timer(&timer_idx_cust_addr_update);
}

void cust_feature_enable_disable_set(void)
{
    if (!cust_feature_is_enabled())
    {
        cust_feature_enable();
    }
    else
    {
        cust_feature_disable();
    }
}

void cust_adv_update_device_name(bool suffix)
{
    APP_PRINT_INFO1("cust_adv_update_device_name: suffix %d", suffix);
    T_GAP_CAUSE ret = GAP_CAUSE_SUCCESS;
//    uint16_t error_line = 0;
    uint8_t device_name[14] = {0}; //The device name is limited to 14 characters total.
    uint8_t device_name_len = 0;
    if (suffix)
    {
        memcpy(device_name, "RTK_D-Find My", 13);
        device_name_len = 13;
    }
    else
    {
        memcpy(device_name, "RTK_D", 5);
        device_name_len = 5;
    }

    cust_scan_rsp_data[0] = device_name_len + 1;
    cust_scan_rsp_data[1] = GAP_ADTYPE_LOCAL_NAME_COMPLETE;
    memcpy(&cust_scan_rsp_data[2], device_name, device_name_len);
    cust_scan_rsp_data_len = device_name_len + 2;
    le_set_gap_param(GAP_PARAM_DEVICE_NAME, device_name_len, device_name);
    ble_ext_adv_mgr_set_scan_response_data(le_cust_adv.adv_handle, cust_scan_rsp_data_len,
                                           cust_scan_rsp_data);

}

uint8_t cust_get_conn_id(void)
{
    return custom_data.cust_conn_id;
}

bool cust_adv_start(uint16_t duration_10ms)
{
    if (le_cust_adv.state == BLE_EXT_ADV_MGR_ADV_DISABLED)
    {
        if (ble_ext_adv_mgr_enable(le_cust_adv.adv_handle, duration_10ms) == GAP_CAUSE_SUCCESS)
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
        APP_PRINT_TRACE0("cust_adv_start: Already started");
        return true;
    }
}

bool cust_adv_stop(int8_t app_cause)
{
    APP_PRINT_INFO0("cust_adv_stop");
    if (ble_ext_adv_mgr_disable(le_cust_adv.adv_handle, app_cause) == GAP_CAUSE_SUCCESS)
    {
        return true;
    }
    else
    {
        return false;
    }
}

#endif
