/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if CONFIG_REALTEK_GFPS_FINDER_SUPPORT
#include "stdint.h"
#include "ble_ext_adv.h"
#include "string.h"
#include "trace.h"
#include "app_link_util.h"
#include "app_gfps_finder.h"
#include "app_gfps_cfg.h"
#include "app_adv_stop_cause.h"

T_GAP_DEV_STATE  gfps_finder_gap_state;
static bool key_stop_track = false;
/**
 * @brief GFPS finder beacon advertising data
 *
 */
static uint8_t gfps_finder_advdata[] =
{
    0x02,//length = 0x02
    0x01,//type = GAP_ADTYPE_FLAGS
    0x06,//data = GAP_ADTYPE_FLAGS_GENERAL|GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED

    0x19,//length = 0x18
    0x16,//type = GAP_ADTYPE_SERVICE_DATA
    0xAA, 0xFE, //data: 16-bit Eddystone UUID = 0xFEAA
    0x40,//E2EE-EID Frame type
    /*20-byte Ephemeral Identifier.*/
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /*hashed flags?*/
    0x00,
};

static uint8_t gfps_finder_scanrspdata[] =
{
    12,//length = 0x02
    GAP_ADTYPE_LOCAL_NAME_COMPLETE,
    'G', 'F', 'P', 'S', '_', 'F', 'i', 'n', 'd', 'e', 'r',
};

void gfps_finder_handle_dev_state(T_GAP_DEV_STATE new_state, uint16_t cause);
void gfps_finder_handle_conn_state(uint8_t conn_id, T_GAP_CONN_STATE new_state,
                                   uint16_t disc_cause);
void gfps_finder_handle_authen_state(uint8_t conn_id, uint8_t new_state, uint16_t cause);
void gfps_finder_handle_conn_mtu_info(uint8_t conn_id, uint16_t mtu_size);
void gfps_finder_handle_conn_param_update(uint8_t conn_id, uint8_t status, uint16_t cause);

const T_FUN_GAP_MSG_CBS  gfps_finder_gap_msg_cbs =
{
    .device_state_cb = gfps_finder_handle_dev_state,
    .conn_state_cb = gfps_finder_handle_conn_state,
    .authen_state_cb = gfps_finder_handle_authen_state,
    .mtu_info_cb = gfps_finder_handle_conn_mtu_info,
    .conn_param_cb = gfps_finder_handle_conn_param_update,
};

T_LE_ADV_CONN gfps_finder_adv =
{
    .adv_handle = 0xff,
    .conn_id = 0xff,
    .remote_bd = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
    .state = BLE_EXT_ADV_MGR_ADV_DISABLED,
    .cb_tbl = &gfps_finder_gap_msg_cbs,
};

void gfps_finder_handle_dev_state(T_GAP_DEV_STATE new_state, uint16_t cause)
{
    APP_PRINT_INFO3("gfps_finder_handle_dev_state: init state %d, adv state %d, cause 0x%x",
                    new_state.gap_init_state, new_state.gap_adv_state, cause);

    if (gfps_finder_gap_state.gap_init_state != new_state.gap_init_state)
    {
        if (new_state.gap_init_state == GAP_INIT_STATE_STACK_READY)
        {
            APP_PRINT_INFO0("GAP stack ready: gfps finder case");
            app_gfps_force_enter_pairing_mode(GFPS_KEY_FORCE_ENTER_PAIR_MODE);
        }
    }
    gfps_finder_gap_state = new_state;
}

void gfps_finder_handle_conn_state(uint8_t conn_id, T_GAP_CONN_STATE new_state,
                                   uint16_t disc_cause)
{
    APP_PRINT_INFO3("gfps_finder_handle_conn_state: conn_id %d new_state %d, disc_cause 0x%x",
                    conn_id, new_state, disc_cause);
    switch (new_state)
    {
    case GAP_CONN_STATE_DISCONNECTED:
        {
            APP_PRINT_INFO3("app_gfps_finder_beacon_le_disconnect_cb: conn_id %d, beacon_conn_id %d, disc_cause 0x%x",
                            conn_id, gfps_finder_adv.conn_id, disc_cause);
            gfps_finder_adv.conn_id = 0xFF;
            app_gfps_finder_beacon_set_conn_id(0xFF);
            app_gfps_finder_reset_conn();
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

        }
        break;

    default:
        break;
    }
}

void gfps_finder_handle_authen_state(uint8_t conn_id, uint8_t new_state, uint16_t cause)
{
    APP_PRINT_INFO2("gfps_finder_handle_authen_state:conn_id %d, cause 0x%x", conn_id, cause);

    switch (new_state)
    {
    case GAP_AUTHEN_STATE_STARTED:
        {
            APP_PRINT_INFO0("gfps_finder_handle_authen_state: GAP_AUTHEN_STATE_STARTED");
        }
        break;

    case GAP_AUTHEN_STATE_COMPLETE:
        {
            if (cause == GAP_SUCCESS)
            {
                APP_PRINT_INFO0("gfps_finder_handle_authen_state: GAP_AUTHEN_STATE_COMPLETE pair success");
            }
            else
            {
                APP_PRINT_INFO0("gfps_finder_handle_authen_state: GAP_AUTHEN_STATE_COMPLETE pair failed");
            }
        }
        break;

    default:
        {
            APP_PRINT_ERROR1("gfps_finder_handle_authen_state: unknown newstate %d", new_state);
        }
        break;
    }
}

void gfps_finder_handle_conn_mtu_info(uint8_t conn_id, uint16_t mtu_size)
{
    APP_PRINT_INFO2("gfps_finder_handle_conn_mtu_info: conn_id %d, mtu_size %d", conn_id, mtu_size);
}

void gfps_finder_handle_conn_param_update(uint8_t conn_id, uint8_t status, uint16_t cause)
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
            APP_PRINT_INFO3("gfps_finder_handle_conn_param_update update success:conn_interval 0x%x, conn_slave_latency 0x%x, conn_supervision_timeout 0x%x",
                            conn_interval, conn_slave_latency, conn_supervision_timeout);
        }
        break;

    case GAP_CONN_PARAM_UPDATE_STATUS_FAIL:
        {
            APP_PRINT_ERROR1("gfps_finder_handle_conn_param_update update failed: cause 0x%x", cause);
        }
        break;

    case GAP_CONN_PARAM_UPDATE_STATUS_PENDING:
        {
            APP_PRINT_INFO0("gfps_finder_handle_conn_param_update update pending.");
        }
        break;

    default:
        break;
    }
}

T_BLE_EXT_ADV_MGR_STATE gfps_finder_adv_get_adv_state()
{
    T_BLE_EXT_ADV_MGR_STATE adv_state;

    adv_state = ble_ext_adv_mgr_get_adv_state(gfps_finder_adv.adv_handle);
    APP_PRINT_TRACE1("gfps_finder_get_adv_state: adv_state %d", adv_state);

    return adv_state;
}

T_GAP_CAUSE gfps_finder_adv_update_rpa(void)
{
    uint8_t random_address[6] = {0};
    le_gen_rand_addr(GAP_RAND_ADDR_RESOLVABLE, random_address);
    T_GAP_CAUSE cause = ble_ext_adv_mgr_set_random(gfps_finder_adv.adv_handle, random_address);
    APP_PRINT_TRACE2("gfps_finder_adv_update_rpa: cause %d, RPA %b", cause,
                     TRACE_BDADDR(random_address));

    return cause;
}

bool gfps_finder_adv_get_key_stop_tracking_flag(void)
{
    return key_stop_track;
}

void gfps_finder_adv_set_key_stop_tracking_flag(bool key_stop)
{
    key_stop_track = key_stop;
}

void gfps_finder_adv_start(uint16_t duration_10ms)
{
    uint8_t err_code = 0;

    if (gfps_finder_adv_get_key_stop_tracking_flag() == true)
    {
        /*gfps_finder_adv_start: user do not want enable finder adv for tracking*/
        err_code = 1;
        goto err;
    }

    if (gfps_finder_adv.state == BLE_EXT_ADV_MGR_ADV_DISABLED)
    {
        ble_ext_adv_mgr_enable(gfps_finder_adv.adv_handle, duration_10ms);
    }

err:
    APP_PRINT_TRACE3("gfps_finder_adv_start: duration %d, adv_state %d, err_code %d",
                     duration_10ms, gfps_finder_adv.state, err_code);
    return;
}

void gfps_finder_adv_stop(uint8_t app_cause)
{
    APP_PRINT_TRACE2("gfps_finder_adv_stop: app_cause %d, adv_state %d", app_cause,
                     gfps_finder_adv.state);
    if (gfps_finder_adv.state == BLE_EXT_ADV_MGR_ADV_ENABLED)
    {
        ble_ext_adv_mgr_disable(gfps_finder_adv.adv_handle, app_cause);
    }
}

/**
 * @brief update frame type
 *
 * the frame type should be set to 0x41 when Unwanted Tracking Protection mode is on.
 * the frame type should be set to 0x40 when Unwanted Tracking Protection mode is off.
 * @param frame_type
 */
void gfps_finder_adv_update_frame_type(uint8_t frame_type)
{
    gfps_finder_advdata[7] = frame_type;
    ble_ext_adv_mgr_set_adv_data(gfps_finder_adv.adv_handle,
                                 sizeof(gfps_finder_advdata),
                                 gfps_finder_advdata);
}


T_GAP_CAUSE gfps_finder_adv_update_adv_interval(uint32_t adv_interval)
{
    T_GAP_CAUSE result = GAP_CAUSE_SUCCESS;

    result = ble_ext_adv_mgr_change_adv_interval(gfps_finder_adv.adv_handle, adv_interval);
    APP_PRINT_TRACE2("gfps_finder_adv_update_adv_interval: result %d, adv_interval %d", result,
                     adv_interval);
    return result;
}

/**
 * @brief update adv EI and hash
 *
 * @param p_ei
 * @param hash
 */
void gfps_finder_adv_update_adv_ei_hash(uint8_t *p_ei, uint8_t hash)
{
    if (p_ei)
    {
        memcpy(&gfps_finder_advdata[8], p_ei, 20);
    }
    gfps_finder_advdata[28] = hash;
    ble_ext_adv_mgr_set_adv_data(gfps_finder_adv.adv_handle,
                                 sizeof(gfps_finder_advdata),
                                 gfps_finder_advdata);

    APP_PRINT_TRACE2("gfps_finder_adv_update_adv_ei_hash: EI %b, hash result 0x%x",
                     TRACE_BINARY(20, p_ei), hash);
}

/**
 * @brief update hash
 *
 * @param hash
 */
void gfps_finder_adv_update_hash(uint8_t hash)
{
    gfps_finder_advdata[28] = hash;
    ble_ext_adv_mgr_set_adv_data(gfps_finder_adv.adv_handle,
                                 sizeof(gfps_finder_advdata),
                                 gfps_finder_advdata);

    APP_PRINT_TRACE1("gfps_finder_adv_update_hash: hash 0x%x", hash);
}

static void gfps_finder_adv_state_change(T_BLE_EXT_ADV_CB_DATA cb_data)
{
    gfps_finder_adv.state = cb_data.p_ble_state_change->state;
    uint8_t adv_handle = cb_data.p_ble_state_change->adv_handle;
    T_BLE_EXT_ADV_STOP_CAUSE stack_cause = cb_data.p_ble_state_change->stop_cause;
    uint8_t app_cause = cb_data.p_ble_state_change->app_cause;

    APP_PRINT_TRACE4("gfps_finder_adv_state_change: adv_state %d, adv_handle %d, stack stop cause 0x%x, app stop cause 0x%02x",
                     gfps_finder_adv.state, adv_handle, stack_cause, app_cause);

    if (gfps_finder_adv.state == BLE_EXT_ADV_MGR_ADV_DISABLED)
    {
        if (stack_cause == BLE_EXT_ADV_STOP_CAUSE_TIMEOUT)
        {
            /*in power off mode, finder adv will exist for a limited duration,
            when timeout, device will enter power down mode, and later will be
            wakeup by RTC.*/
            if (!app_gfps_cfg.disable_finder_adv_when_power_off
                && app_gfps_finder_provisioned())
            {
                //power_mode_set(POWER_POWERDOWN_MODE);
            }
        }
    }
}

static void gfps_finder_adv_callback(uint8_t cb_type, void *p_cb_data)
{
    T_BLE_EXT_ADV_CB_DATA cb_data;
    memcpy(&cb_data, p_cb_data, sizeof(T_BLE_EXT_ADV_CB_DATA));

    switch (cb_type)
    {
    case BLE_EXT_ADV_STATE_CHANGE:
        {
            gfps_finder_adv_state_change(cb_data);
        }
        break;

    case BLE_EXT_ADV_SET_CONN_INFO:
        {
            /*this ble link is connected by gfps finder beacon adv*/
            if (gfps_finder_adv.adv_handle == cb_data.p_ble_conn_info->adv_handle)
            {
                gfps_finder_adv.conn_id = cb_data.p_ble_conn_info->conn_id;
                app_gfps_finder_beacon_set_conn_id(cb_data.p_ble_conn_info->conn_id);
            }
        }
        break;
    };
}

void app_gfps_finder_adv_handle_mmi_action(void)
{
    if (app_gfps_cfg.gfps_finder_support)
    {
        uint8_t finder_conn_id = app_gfps_finder_beacon_get_conn_id();
        bool    finder_provisioned = app_gfps_finder_provisioned();

        if (app_gfps_cfg.gfps_support && finder_provisioned)
        {
            /*user keypress to disable the device (temporarily stop advertising) as required in the spec
            to ensure compliance with unwanted tracking protections*/
            gfps_finder_adv_stop(APP_STOP_ADV_CAUSE_GFPS_FINDER_KEY_PRESS);
            gfps_finder_adv_set_key_stop_tracking_flag(true);
        }

        APP_PRINT_INFO2("app_gfps_finder_adv_handle_mmi_action: finder_conn_id %d, provisioned %d",
                        finder_conn_id, finder_provisioned);
    }
}

void gfps_finder_adv_init(uint8_t *p_adv_ei, uint8_t hash)
{
    T_GAP_CAUSE cause;
    T_LE_EXT_ADV_LEGACY_ADV_PROPERTY adv_event_prop = LE_EXT_ADV_LEGACY_ADV_CONN_SCAN_UNDIRECTED;
    uint16_t adv_interval_min = 0x320; //500ms
    uint16_t adv_interval_max = 0x320; //500ms
    T_GAP_LOCAL_ADDR_TYPE own_address_type = GAP_LOCAL_ADDR_LE_RANDOM;
    T_GAP_REMOTE_ADDR_TYPE peer_address_type = GAP_REMOTE_ADDR_LE_PUBLIC;
    uint8_t  peer_address[6] = {0, 0, 0, 0, 0, 0};
    T_GAP_ADV_FILTER_POLICY filter_policy = GAP_ADV_FILTER_ANY;

    uint8_t random_address[6] = {0};
    le_gen_rand_addr(GAP_RAND_ADDR_RESOLVABLE, random_address);

    memcpy(&gfps_finder_advdata[8], p_adv_ei, 20);
    gfps_finder_advdata[28] = hash;
    ble_ext_adv_mgr_init_adv_params(&gfps_finder_adv.adv_handle, adv_event_prop, adv_interval_min,
                                    adv_interval_max, own_address_type, peer_address_type, peer_address,
                                    filter_policy, sizeof(gfps_finder_advdata), gfps_finder_advdata,
                                    sizeof(gfps_finder_scanrspdata), gfps_finder_scanrspdata, random_address);

    cause = ble_ext_adv_mgr_register_callback(gfps_finder_adv_callback, gfps_finder_adv.adv_handle);
    if (cause == GAP_CAUSE_SUCCESS)
    {
        APP_PRINT_INFO1("gfps_finder_adv_init: adv_handle %d", gfps_finder_adv.adv_handle);
    }
}
#endif
