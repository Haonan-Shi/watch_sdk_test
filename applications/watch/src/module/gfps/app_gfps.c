/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if CONFIG_REALTEK_GFPS_FEATURE_SUPPORT
#include "stdlib.h"
#include "trace.h"
#include "gap_br.h"
#include "gap_bond_le.h"
#include "bt_gfps.h"
#include "gfps.h"
#include "app_gfps_timer.h"
#include "ble_ext_adv.h"
#include "ble_conn.h"
#include "app_gfps.h"
#include "app_gfps_account_key.h"
#include "app_gfps_personalized_name.h"
#include "app_ble_gap.h"
#include "app_gfps_cfg.h"
#include "app_main.h"
#include "app_bt_policy_api.h"
#include "app_hfp.h"
#include "app_gfps_msg.h"
#include "app_multilink.h"
#include "app_gfps_device.h"
#include "app_bond.h"
#include "app_gfps_link.h"
#include "app_cfg.h"
#include "app_adv_stop_cause.h"
#include "app_dult.h"
#include "app_dult_device.h"
#include "app_module_init.h"
#include "app_ble_service_info.h"
#include "app_ecc.h"
#include "app_task.h"
#if CONFIG_REALTEK_GFPS_LE_DEVICE_SUPPORT
#include "bt_bond_le.h"
#endif
#if CONFIG_REALTEK_GFPS_FINDER_SUPPORT
#include "gfps_find_my_device.h"
#include "app_gfps_finder.h"
#include "app_gfps_finder_adv.h"
#endif

T_GFPS_DB gfps_db;
T_GAP_DEV_STATE  gfps_gap_state;
uint8_t gfps_adv_len;
uint8_t gfps_adv_data[GAP_MAX_LEGACY_ADV_LEN];
static T_EVENT_BUS_SUBSCRIBER_HANDLE gfps_ecc_msg_relay_handle;

void app_gfps_ble_conn_info_init(uint8_t conn_id);
static T_APP_RESULT app_gfps_cb(T_SERVER_ID service_id, void *p_data);
static void app_gfps_le_disconnect_cb(uint8_t conn_id, uint8_t local_disc_cause,
                                      uint16_t disc_cause);

static uint8_t gfps_scan_rsp_data[] =
{
    0x09,/* length */
    GAP_ADTYPE_LOCAL_NAME_COMPLETE,/* type="device name" */
    'G', 'F', 'P', 'S', '_', 'A', 'D', 'V',
};

static uint8_t gfps_get_scan_rsp_data_len(void)
{
    return sizeof(gfps_scan_rsp_data);
}

void gfps_handle_dev_state(T_GAP_DEV_STATE new_state, uint16_t cause);
void gfps_handle_conn_state(uint8_t conn_id, T_GAP_CONN_STATE new_state,
                            uint16_t disc_cause);
void gfps_handle_authen_state(uint8_t conn_id, uint8_t new_state, uint16_t cause);
void gfps_handle_conn_mtu_info(uint8_t conn_id, uint16_t mtu_size);
void gfps_handle_conn_param_update(uint8_t conn_id, uint8_t status, uint16_t cause);

const T_FUN_GAP_MSG_CBS  gfps_gap_msg_cbs =
{
    .device_state_cb = gfps_handle_dev_state,
    .conn_state_cb = gfps_handle_conn_state,
    .authen_state_cb = gfps_handle_authen_state,
    .mtu_info_cb = gfps_handle_conn_mtu_info,
    .conn_param_cb = gfps_handle_conn_param_update,
};

T_LE_ADV_CONN gfps_adv =
{
    .adv_handle = 0xff,
    .conn_id = 0xff,
    .role = GAP_LINK_ROLE_SLAVE,
    .state = BLE_EXT_ADV_MGR_ADV_DISABLED,
    .cb_tbl = &gfps_gap_msg_cbs,
};

void gfps_handle_dev_state(T_GAP_DEV_STATE new_state, uint16_t cause)
{
    APP_PRINT_INFO3("gfps_handle_dev_state: init state %d, adv state %d, cause 0x%x",
                    new_state.gap_init_state, new_state.gap_adv_state, cause);

    if (gfps_gap_state.gap_init_state != new_state.gap_init_state)
    {
        if (new_state.gap_init_state == GAP_INIT_STATE_STACK_READY)
        {
            APP_PRINT_INFO0("GAP stack ready: gfps finder case");
            /*Resolvable private address can only be successfully generate after BLE stack ready,
            app_gfps_adv_init() and app_gfps_finder_init() need to generate RPA, so we call them here*/
            if (app_gfps_cfg.gfps_support)
            {
                app_gfps_adv_init();
#if CONFIG_REALTEK_GFPS_FINDER_SUPPORT
                if (app_gfps_cfg.gfps_finder_support)
                {
                    app_gfps_finder_init();
                    app_dult_device_init();
                    app_dult_handle_power_on();
                }
#endif
                app_gfps_device_handle_power_on(false);
            }
        }
    }
    gfps_gap_state = new_state;
}

void gfps_handle_conn_state(uint8_t conn_id, T_GAP_CONN_STATE new_state,
                            uint16_t disc_cause)
{
    APP_PRINT_INFO3("gfps_handle_conn_state: conn_id %d new_state %d, disc_cause 0x%x",
                    conn_id, new_state, disc_cause);
    T_APP_LE_LINK *p_link;
    p_link = app_find_le_link_by_conn_id(conn_id);
    switch (new_state)
    {
    case GAP_CONN_STATE_DISCONNECTING:
        if (p_link != NULL)
        {
            p_link->state = LE_LINK_STATE_DISCONNECTING;
        }
        break;

    case GAP_CONN_STATE_DISCONNECTED:
        {
            APP_PRINT_INFO3("gfps_handle_diconnect_state: conn_id %d, beacon_conn_id %d, disc_cause 0x%x",
                            conn_id, gfps_adv.conn_id, disc_cause);
            gfps_adv.conn_id = 0xFF;
#if CONFIG_REALTEK_GFPS_LE_DEVICE_SUPPORT
            if (app_gfps_cfg.gfps_support)
            {
                app_gfps_le_disconnect_cb(conn_id, p_link->local_disc_cause, disc_cause);
            }
#endif

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

            if (p_link != NULL)
            {
                if (p_link->state == LE_LINK_STATE_CONNECTING)
                {
                    p_link->state = LE_LINK_STATE_CONNECTED;
                    le_get_conn_param(GAP_PARAM_CONN_MTU_SIZE, &p_link->mtu_size, conn_id);
                }

#if CONFIG_REALTEK_GFPS_LE_DEVICE_SUPPORT
                if (app_gfps_cfg.gfps_le_device_support)
                {
                    app_gfps_linkback_info_init(conn_id);
                    app_gfps_force_enter_pairing_mode(GFPS_EXIT_PAIR_MODE);
                }
#endif
            }
            else
            {
                APP_PRINT_ERROR0("gfps link not exist");
            }

        }
        break;

    default:
        break;
    }
}

void gfps_handle_authen_state(uint8_t conn_id, uint8_t new_state, uint16_t cause)
{
    APP_PRINT_INFO2("gfps_handle_authen_state:conn_id %d, cause 0x%x", conn_id, cause);

    switch (new_state)
    {
    case GAP_AUTHEN_STATE_STARTED:
        {
            APP_PRINT_INFO0("gfps_handle_authen_state: GAP_AUTHEN_STATE_STARTED");
        }
        break;

    case GAP_AUTHEN_STATE_COMPLETE:
        {
            if (cause == GAP_SUCCESS)
            {
                APP_PRINT_INFO0("gfps_handle_authen_state: GAP_AUTHEN_STATE_COMPLETE pair success");
            }
            else
            {
                APP_PRINT_INFO0("gfps_handle_authen_state: GAP_AUTHEN_STATE_COMPLETE pair failed");
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

void gfps_handle_conn_mtu_info(uint8_t conn_id, uint16_t mtu_size)
{
    APP_PRINT_INFO2("gfps_handle_conn_mtu_info: conn_id %d, mtu_size %d", conn_id, mtu_size);
}

void gfps_handle_conn_param_update(uint8_t conn_id, uint8_t status, uint16_t cause)
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
            APP_PRINT_INFO3("gfps_handle_conn_param_update update success:conn_interval 0x%x, conn_slave_latency 0x%x, conn_supervision_timeout 0x%x",
                            conn_interval, conn_slave_latency, conn_supervision_timeout);
        }
        break;

    case GAP_CONN_PARAM_UPDATE_STATUS_FAIL:
        {
            APP_PRINT_ERROR1("gfps_handle_conn_param_update update failed: cause 0x%x", cause);
        }
        break;

    case GAP_CONN_PARAM_UPDATE_STATUS_PENDING:
        {
            APP_PRINT_INFO0("gfps_handle_conn_param_update update pending.");
        }
        break;

    default:
        break;
    }
}

/*google Fast pair initialize*/
void app_gfps_init(void)
{
    uint8_t sec_req_enable = false;
    bool is_tag = app_gfps_cfg.gfps_device_type == GFPS_LOCATOR_TRACKER ? true : false;

    if (app_gfps_account_key_init(app_gfps_cfg.gfps_account_key_num) == false)
    {
        goto error;
    }

#if GFPS_PERSONALIZED_NAME_SUPPORT
    app_gfps_personalized_name_init();
#endif

    le_bond_set_param(GAP_PARAM_BOND_SEC_REQ_ENABLE, sizeof(uint8_t), &sec_req_enable);
    APP_PRINT_INFO3("app_gfps_init: model id 0x%x, 0x%x, 0x%x",
                    app_gfps_cfg.gfps_model_id[0], app_gfps_cfg.gfps_model_id[1],
                    app_gfps_cfg.gfps_model_id[2]);
    if (gfps_init(app_gfps_cfg.gfps_model_id, app_gfps_cfg.gfps_public_key,
                  app_gfps_cfg.gfps_private_key) == false)
    {
        goto error;
    }

#if CONFIG_REALTEK_GFPS_FINDER_SUPPORT
    gfps_set_finder_enable(app_gfps_cfg.gfps_finder_support);
#endif

    gfps_set_tx_power(app_gfps_cfg.gfps_enable_tx_power, app_gfps_cfg.gfps_tx_power);
    gfps_db.current_conn_id = 0xFF;

    for (uint8_t i = 0; i < MAX_BLE_LINK_NUM; i++)
    {
        T_APP_LE_LINK *le_link = &app_db.le_link[i];

        le_link->gfps_link.gfps_conn_id = 0xFF;
        le_link->gfps_link.gfps_conn_state = GAP_CONN_STATE_DISCONNECTED;
    }

    gfps_db.gfps_id = gfps_add_service(app_gfps_cb);

    gfps_ecc_manager_malloc();
    app_gfps_link_priority_queue_init();

#if CONFIG_REALTEK_GFPS_LE_DEVICE_SUPPORT
    gfps_le_device_init(app_gfps_cfg.gfps_le_device_support,
                        app_gfps_cfg.gfps_le_device_mode, is_tag);
#endif

    gap_get_param(GAP_PARAM_BOND_IO_CAPABILITIES, &(gfps_db.additional_default_io_cap));
    gap_get_param(GAP_PARAM_BOND_AUTHEN_REQUIREMENTS_FLAGS, &(gfps_db.additional_default_auth_flags));

    app_gfps_timer_init();
    app_gfps_set_allow_write_account_key(true);
    gfps_set_identity_address(app_cfg_nv.bud_local_addr, NULL, false);
    return;
error:
    APP_PRINT_ERROR0("app_gfps_init: failed");
}

void app_gfps_get_random_addr(uint8_t *random_bd)
{
    memcpy(random_bd, gfps_db.random_address, GAP_BD_ADDR_LEN);
}

bool app_gfps_adv_start(T_GFPS_ADV_MODE mode, bool show_ui)
{
    app_gfps_timer_stop_update_rpa();
    app_gfps_timer_start_update_rpa();

    if (gfps_gen_adv_data(mode, gfps_adv_data, &gfps_adv_len, show_ui))
    {
        uint16_t interval = app_gfps_cfg.gfps_discov_adv_interval;

        if (mode == NOT_DISCOVERABLE_MODE)
        {
            //ble_ext_adv_mgr_change_own_address_type(gfps_adv.adv_handle., GAP_LOCAL_ADDR_LE_RANDOM);
            interval = app_gfps_cfg.gfps_not_discov_adv_interval;
        }
        else if (mode == DISCOVERABLE_MODE_WITH_MODEL_ID)
        {
            //ble_ext_adv_mgr_change_own_address_type(gfps_adv.adv_handle, GAP_LOCAL_ADDR_LE_PUBLIC);
        }

        ble_ext_adv_mgr_set_multi_param(gfps_adv.adv_handle, NULL,
                                        interval, gfps_adv_len, gfps_adv_data, 0, NULL);
    }
    else
    {
        APP_PRINT_ERROR0("app_gfps_adv_start: gfps_gen_adv_data failed");
        return false;
    }

    if (gfps_adv.state == BLE_EXT_ADV_MGR_ADV_DISABLED)
    {
        if (ble_ext_adv_mgr_enable(gfps_adv.adv_handle, 0) == GAP_CAUSE_SUCCESS)
        {
            return true;
        }
    }
    return true;
}

T_GAP_CAUSE app_gfps_adv_update_adv_interval(uint32_t adv_interval)
{
    T_GAP_CAUSE result = GAP_CAUSE_SUCCESS;

    result = ble_ext_adv_mgr_change_adv_interval(gfps_adv.adv_handle, adv_interval);
    APP_PRINT_TRACE2("app_gfps_adv_update_adv_interval: result %d, adv_interval %d", result,
                     adv_interval);
    return result;
}

void app_gfps_get_ble_addr(uint8_t *ble_addr)
{
    if (gfps_db.force_enter_pair_mode)
    {
        gap_get_param(GAP_PARAM_BD_ADDR, ble_addr);
    }
    else
    {
        app_gfps_get_random_addr(ble_addr);
    }

    APP_PRINT_INFO2("app_gfps_get_ble_addr: %s, force_enter_pair_mode %d",
                    TRACE_BDADDR(ble_addr), gfps_db.force_enter_pair_mode);
}

void app_gfps_update_rpa(bool gfps_generate_rpa)
{
    if (gfps_generate_rpa)
    {
        //generate a valid rpa
        le_gen_rand_addr(GAP_RAND_ADDR_RESOLVABLE, gfps_db.random_address);
        //set this rpa into stack
        ble_ext_adv_mgr_set_random(gfps_adv.adv_handle, gfps_db.random_address);
    }
    else
    {
        //directly get rpa from stack
        //ble_ext_adv_get_rpa_by_adv_handle(gfps_adv.adv_handle, gfps_db.random_address);
        //generate a valid rpa
        le_gen_rand_addr(GAP_RAND_ADDR_RESOLVABLE, gfps_db.random_address);
        //set this rpa into stack
        ble_ext_adv_mgr_set_random(gfps_adv.adv_handle, gfps_db.random_address);
    }

    //update rpa in gfps through RFCOMM or LE L2CAP channel
    app_gfps_msg_update_rpa_addr();
    APP_PRINT_INFO1("app_gfps_update_rpa: RPA %s", TRACE_BDADDR(gfps_db.random_address));
}

T_BLE_EXT_ADV_MGR_STATE app_gfps_adv_get_state(void)
{
    return gfps_adv.state;
}

uint8_t app_gfps_adv_get_handle(void)
{
    return gfps_adv.adv_handle;
}

T_GFPS_ACTION app_gfps_adv_get_curr_action(void)
{
    return gfps_db.gfps_curr_action;
}

bool app_gfps_next_action(T_GFPS_ACTION gfps_next_action)
{
    uint8_t link_num = app_gfps_link_find_gfps_link();

    APP_PRINT_TRACE4("app_gfps_next_action: gfps_curr_action %d, gfps_next_action %d, gfps_adv_state %d, link_num %d",
                     gfps_db.gfps_curr_action, gfps_next_action,
                     gfps_adv.state, link_num);

    gfps_db.gfps_curr_action = gfps_next_action;

    switch (gfps_db.gfps_curr_action)
    {
    case GFPS_ACTION_IDLE:
        {
            if (gfps_adv.state == BLE_EXT_ADV_MGR_ADV_ENABLED)
            {
                ble_ext_adv_mgr_disable(gfps_adv.adv_handle, APP_STOP_ADV_CAUSE_GFPS_ACTION_IDLE);
            }
            else if (link_num != 0)
            {
                app_gfps_link_disconnect_all_gfps_link();
            }
        }
        break;

    case GFPS_ACTION_ADV_DISCOVERABLE_MODE_WITH_MODEL_ID:
        {
            app_gfps_adv_start(DISCOVERABLE_MODE_WITH_MODEL_ID, true);
        }
        break;

    case GFPS_ACTION_ADV_NOT_DISCOVERABLE_MODE:
        {
            app_gfps_adv_start(NOT_DISCOVERABLE_MODE, true);
        }
        break;

    case GFPS_ACTION_ADV_NOT_DISCOVERABLE_MODE_HIDE_UI:
        {
            app_gfps_adv_start(NOT_DISCOVERABLE_MODE, false);
        }
        break;

    default:
        break;
    }

    return true;
}

bool app_gfps_ble_conn_info_handle(uint8_t *remote_bd_addr, uint8_t remote_addr_type)
{
    bool receive_ble_link = true;

#if CONFIG_REALTEK_GFPS_LE_DEVICE_SUPPORT
    if (app_gfps_cfg.gfps_le_device_support)
    {
        uint8_t link_num = app_gfps_link_find_gfps_link();
        bool addr_reslove_ret = false;
        uint8_t resolved_addr[6] = {0};
        uint8_t resolved_bd_type = 0xFF;

        if (link_num >= GFPS_LE_MAX_LINK_NUMBER)
        {
            receive_ble_link = false;
            return receive_ble_link;
        }

        if (remote_addr_type == GAP_REMOTE_ADDR_LE_RANDOM)
        {
            addr_reslove_ret = bt_le_resolve_random_address(remote_bd_addr, resolved_addr,
                                                            (T_GAP_IDENT_ADDR_TYPE *)&resolved_bd_type);
        }

        if ((gfps_db.force_enter_pair_mode == false) && (addr_reslove_ret == true))
        {
            T_ACCOUNT_KEY *p_key_info = app_gfps_account_key_get_table();

            for (uint8_t i = 0; i < p_key_info->num; i++)
            {
                if (memcmp(resolved_addr, p_key_info->account_info[i].addr, 6) == 0)
                {
                    gfps_db.gfps_linkback_init = true;
                }
            }
        }
    }
#endif

    APP_PRINT_INFO1("app_gfps_ble_conn_info_handle: receive_ble_link %d", receive_ble_link);
    return receive_ble_link;
}

static void app_gfps_adv_callback(uint8_t cb_type, void *p_cb_data)
{
    T_BLE_EXT_ADV_CB_DATA cb_data;
    memcpy(&cb_data, p_cb_data, sizeof(T_BLE_EXT_ADV_CB_DATA));

    switch (cb_type)
    {
    case BLE_EXT_ADV_STATE_CHANGE:
        {
            gfps_adv.state = cb_data.p_ble_state_change->state;

            if (cb_data.p_ble_state_change->state == BLE_EXT_ADV_MGR_ADV_DISABLED)
            {
                gfps_db.gfps_curr_action = GFPS_ACTION_IDLE;
            }

            APP_PRINT_TRACE4("app_gfps_adv_callback: adv_state %d, adv_handle %d, stop_cause %d, app_cause 0x%02x",
                             cb_data.p_ble_state_change->state,
                             cb_data.p_ble_state_change->adv_handle,
                             cb_data.p_ble_state_change->stop_cause,
                             cb_data.p_ble_state_change->app_cause);
        }
        break;

    case BLE_EXT_ADV_SET_CONN_INFO:
        {
            APP_PRINT_TRACE4("app_gfps_adv_callback: BLE_EXT_ADV_SET_CONN_INFO conn_id 0x%x, adv_handle %d, local_addr_type %d, local_bd %s",
                             cb_data.p_ble_conn_info->conn_id,
                             cb_data.p_ble_conn_info->adv_handle,
                             cb_data.p_ble_conn_info->local_addr_type,
                             TRACE_BDADDR(cb_data.p_ble_conn_info->local_addr));
#if CONFIG_REALTEK_GFPS_LE_DEVICE_SUPPORT
            if (app_gfps_cfg.gfps_le_device_support)
            {
                if (cb_data.p_ble_conn_info->adv_handle == gfps_adv.adv_handle)
                {
                    gfps_adv.conn_id = cb_data.p_ble_conn_info->conn_id;
                    app_gfps_ble_conn_info_init(cb_data.p_ble_conn_info->conn_id);
                }
            }
            else
#endif
            {
                gfps_db.current_conn_id = cb_data.p_ble_conn_info->conn_id;
            }

        }
        break;

    default:
        break;
    }

    return;
}

void app_gfps_ble_conn_info_init(uint8_t conn_id)
{
    T_APP_LE_LINK *p_link;
    p_link = app_find_le_link_by_conn_id(conn_id);

    if (p_link == NULL)
    {
        return;
    }

    p_link->gfps_link.gfps_conn_state = GAP_CONN_STATE_CONNECTED;
    p_link->gfps_link.gfps_conn_id = conn_id;
    app_gfps_link_add_link_into_priority_queue(p_link->gfps_link.gfps_conn_id);
    gfps_db.current_conn_id = conn_id;

    if ((app_gfps_cfg.gfps_le_device_mode == GFPS_LE_DEVICE_MODE_LE_MODE_WITH_LEA ||
         app_gfps_cfg.gfps_le_device_mode == GFPS_LE_DEVICE_MODE_LE_MODE_WITHOUT_LEA))
    {
        T_GFPS_PAIRING_STATUS gfps_pair_status = GFPS_PAIRING_STATUS_IDLE;
        gfps_set_pairing_status(gfps_pair_status);
        app_gfps_force_enter_pairing_mode(GFPS_EXIT_PAIR_MODE);
    }
}

void app_gfps_linkback_info_init(uint8_t conn_id)
{
    if (gfps_db.gfps_linkback_init == true)
    {
        app_gfps_ble_conn_info_init(conn_id);
        gfps_db.gfps_linkback_init = false;

#if CONFIG_REALTEK_GFPS_LE_DEVICE_SUPPORT
        //need to start adv for LE audio multilink reconnect
        if (app_gfps_cfg.gfps_le_device_support)
        {
            T_ACCOUNT_KEY *p_key_info = app_gfps_account_key_get_table();

            if (p_key_info->num != 0)
            {
                app_gfps_le_device_adv_start();
            }
            app_gfps_set_ble_conn_param(conn_id);
        }
#endif
    }
}

void app_gfps_adv_init(void)
{
    bool ret = false;
    uint8_t random_address[6] = {0};
    uint8_t  peer_address[6] = {0, 0, 0, 0, 0, 0};
    T_GAP_ADV_FILTER_POLICY filter_policy = GAP_ADV_FILTER_ANY;
    T_GAP_REMOTE_ADDR_TYPE peer_address_type = GAP_REMOTE_ADDR_LE_PUBLIC;
    T_GAP_LOCAL_ADDR_TYPE own_address_type = GAP_LOCAL_ADDR_LE_RANDOM;
    uint16_t adv_interval = app_gfps_cfg.gfps_discov_adv_interval;
    T_LE_EXT_ADV_LEGACY_ADV_PROPERTY adv_event_prop = LE_EXT_ADV_LEGACY_ADV_CONN_SCAN_UNDIRECTED;

    T_GAP_CAUSE cause = le_gen_rand_addr(GAP_RAND_ADDR_RESOLVABLE, random_address);
    memcpy(gfps_db.random_address, random_address, GAP_BD_ADDR_LEN);

    ble_ext_adv_mgr_init_adv_params(&gfps_adv.adv_handle, adv_event_prop, adv_interval,
                                    adv_interval, own_address_type, peer_address_type,
                                    peer_address, filter_policy, 0, NULL,
                                    gfps_get_scan_rsp_data_len(), gfps_scan_rsp_data, random_address);

    ret = ble_ext_adv_mgr_register_callback(app_gfps_adv_callback, gfps_adv.adv_handle);

    APP_PRINT_INFO4("app_gfps_adv_init: gfps_adv_handle %d, cause %d, ret %d, gfps version %b",
                    gfps_adv.adv_handle, cause, ret, TRACE_STRING(GFPS_FIRMWARE_VERSION));
}

static void app_gfps_le_disconnect_cb(uint8_t conn_id, uint8_t local_disc_cause,
                                      uint16_t disc_cause)
{
    APP_PRINT_TRACE3("app_gfps_le_disconnect_cb: conn_id %d, local_disc_cause %d, disc_cause 0x%x",
                     conn_id, local_disc_cause, disc_cause);

    T_APP_LE_LINK *p_link;
    p_link = app_find_le_link_by_conn_id(conn_id);

    if (p_link == NULL)
    {
        return;
    }

#if CONFIG_REALTEK_GFPS_FINDER_SUPPORT
    /*gfps finder maybe connected by gfps ble link to do provising or provisioned actions,
    so when gfps ble link disconencted, we shall reset gfps finder link here*/
    if (app_gfps_cfg.gfps_finder_support)
    {
        app_gfps_finder_reset_conn();
    }
#endif

    APP_PRINT_ERROR3("app_gfps_le_disconnect_cb: plink connid %d not gfps link connid %d, gfps curr conn_id %d",
                     p_link->conn_id, p_link->gfps_link.gfps_conn_id, gfps_db.current_conn_id);

    if (p_link->conn_id != p_link->gfps_link.gfps_conn_id && p_link->conn_id != gfps_db.current_conn_id)
    {
        return;
    }

    if (p_link->conn_id == gfps_db.current_conn_id)
    {
        //reset current link aes key and pair status
        gfps_db.current_conn_id = 0xFF;
        gfps_reset_aeskey_and_pairing_status();

        //provider recover to original io capability
        if (p_link->gfps_link.auth_param_change)
        {
            APP_PRINT_INFO2("app_gfps_le_disconnect_cb: gap_set_pairable_mode, io_cap %d, authen_flag 0x%x",
                            p_link->gfps_link.io_cap, p_link->gfps_link.auth_flags);

            gap_set_param(GAP_PARAM_BOND_IO_CAPABILITIES, sizeof(uint8_t), &(p_link->gfps_link.io_cap));
            gap_set_param(GAP_PARAM_BOND_AUTHEN_REQUIREMENTS_FLAGS, sizeof(uint16_t),
                          &(p_link->gfps_link.auth_flags));
            gap_set_pairable_mode();
        }
    }

    app_gfps_link_delete_link_from_priority_queue(p_link->gfps_link.gfps_conn_id);
    p_link->gfps_link.gfps_conn_state = GAP_CONN_STATE_DISCONNECTED;
    p_link->gfps_link.gfps_conn_id = 0xff;

    memset(&p_link->gfps_link, 0, sizeof(T_GFPS_LINK));
    app_gfps_device_handle_ble_link_disconnected(local_disc_cause);
}

void app_gfps_send_le_bond_confirm(uint8_t conn_id)
{
    T_APP_LE_LINK *p_link;
    p_link = app_find_le_link_by_conn_id(conn_id);
    if (p_link == NULL)
    {
        return;
    }

    if (p_link->gfps_link.le_bond_passkey == p_link->gfps_link.gfps_raw_passkey)
    {
        le_bond_user_confirm(conn_id, GAP_CFM_CAUSE_ACCEPT);
        gfps_send_passkey(conn_id, gfps_db.gfps_id, p_link->gfps_link.le_bond_passkey);
    }
    else
    {
        le_bond_user_confirm(conn_id, GAP_CFM_CAUSE_REJECT);
    }
    p_link->gfps_link.le_bond_confirm_pending = false;
}

void app_gfps_send_legacy_bond_confirm(uint8_t conn_id)
{
    T_APP_LE_LINK *p_link;
    p_link = app_find_le_link_by_conn_id(conn_id);
    if (p_link == NULL)
    {
        return;
    }

    if (p_link->gfps_link.edr_bond_passkey == p_link->gfps_link.gfps_raw_passkey)
    {
        gap_br_user_cfm_req_cfm(p_link->gfps_link.edr_bond_bd_addr, GAP_CFM_CAUSE_ACCEPT);
        gfps_send_passkey(conn_id, gfps_db.gfps_id, p_link->gfps_link.edr_bond_passkey);
    }
    else
    {
        gap_br_user_cfm_req_cfm(p_link->gfps_link.edr_bond_bd_addr, GAP_CFM_CAUSE_REJECT);
    }
    p_link->gfps_link.edr_bond_confirm_pending = false;
}

static void app_gfps_set_receive_pair_req(bool received)
{
    gfps_db.receive_pair_req = received;
}

void app_gfps_handle_ble_user_confirm(uint8_t conn_id)
{
    T_APP_LE_LINK *p_link;
    p_link = app_find_le_link_by_conn_id(conn_id);

    if (p_link == NULL)
    {
        return;
    }

    uint32_t passkey = 0;
    le_bond_get_display_key(conn_id, &passkey);
    p_link->gfps_link.le_bond_passkey = passkey;

    APP_PRINT_INFO3("app_gfps_handle_ble_user_confirm: passkey %d, gfps_raw_passkey %d, is_gfps_pairing %d",
                    passkey, p_link->gfps_link.gfps_raw_passkey, p_link->gfps_link.is_gfps_pairing);

    if (p_link->gfps_link.is_gfps_pairing)
    {
        app_gfps_set_receive_pair_req(true);
        if (p_link->gfps_link.gfps_raw_passkey_received)
        {
            app_gfps_send_le_bond_confirm(conn_id);
            p_link->gfps_link.gfps_raw_passkey_received = false;
        }
        else
        {
            p_link->gfps_link.le_bond_confirm_pending = true;
            //wait up to 10 seconds for a write to the Passkey characteristic.
            gfps_db.receive_passkey = false;
            app_gfps_timer_start_check_receive_passkey();
        }
    }
    else
    {
        le_bond_user_confirm(conn_id, GAP_CFM_CAUSE_ACCEPT);
    }
}

void app_gfps_set_allow_write_account_key(bool allow)
{
    gfps_db.allow_write_account_key = allow;
}

void app_gfps_check_receive_passkey(void)
{
    T_APP_LE_LINK *p_le_link = app_find_le_link_by_conn_id(gfps_db.current_conn_id);
    if (!gfps_db.receive_passkey)
    {
        if (p_le_link)
        {
            app_disconnect_le_link(p_le_link, LE_LOCAL_DISC_CAUSE_GFPS_PAIR_NOT_RECEIVE_PASSKEY);
        }
    }
    APP_PRINT_INFO2("app_gfps_check_receive_passkey: receive_passkey %d, p_le_link 0x%x",
                    gfps_db.receive_passkey, p_le_link);
}

void app_gfps_check_receive_pair_req(void)
{
    T_APP_LE_LINK *p_le_link = app_find_le_link_by_conn_id(gfps_db.current_conn_id);

    if (!gfps_db.receive_pair_req)
    {
        if (p_le_link)
        {
            app_disconnect_le_link(p_le_link, LE_LOCAL_DISC_CAUSE_GFPS_PAIR_NOT_STARTED);
        }
    }
    APP_PRINT_INFO2("app_gfps_check_receive_pair_req: receive_pair_req %d, p_le_link 0x%x",
                    gfps_db.receive_pair_req, p_le_link);
}

void app_gfps_reset_account_key_write_counts(void)
{
    APP_PRINT_INFO0("app_gfps_reset_account_key_write_counts");
    gfps_db.account_key_write_counts = 0;
}

void app_gfps_reset_failure_count(void)
{
    APP_PRINT_INFO0("app_gfps_reset_failure_count");
    gfps_db.failure_count = 0;
}

static bool app_gfps_check_provider_addr(uint8_t *received_provider_addr)
{
    /*The value is decrypted successfully if the output matches the format in Table 1.2.1 or Table 1.2.2
     (that is, if it contains either the Fast Pair Provider's current BLE address,
     or the Fast Pair Provider's public address).*/
    uint8_t local_public_addr[6] = {0x00};
    gap_get_param(GAP_PARAM_BD_ADDR, local_public_addr);

    if (memcmp(received_provider_addr, gfps_db.random_address, 6) != 0 &&
        memcmp(received_provider_addr, local_public_addr, 6) != 0)
    {
        APP_PRINT_ERROR3("app_gfps_check_provider_addr: provider addr %s not right, random %s, public %s",
                         TRACE_BDADDR(received_provider_addr),
                         TRACE_BDADDR(gfps_db.random_address),
                         TRACE_BDADDR(local_public_addr));
        return false;
    }
    return true;
}

static bool app_gfps_check_failure_count(T_GFPS_KBP_WRITE_RESULT result)
{
    if (gfps_db.failure_count >= 10)
    {
        APP_PRINT_ERROR1("app_gfps_security_condition_check: failure_count %d", gfps_db.failure_count);
        return false;
    }

    /*Keep a count of these failures. When the failure count hits 10, fail all new requests immediately.
    Reset the failure count after 5 minutes, after power on, or after a success.*/
    if (result != GFPS_WRITE_RESULT_SUCCESS)
    {
        gfps_db.failure_count++;
        APP_PRINT_ERROR1("app_gfps_security_condition_check, failure_count %d", gfps_db.failure_count);
        if (gfps_db.failure_count == 10)
        {
            APP_PRINT_ERROR0("app_gfps_timer_start_reset_failure_count");
            app_gfps_timer_start_reset_failure_count();
        }
        return false;
    }
    else
    {
        gfps_db.failure_count = 0;
    }
    return true;
}

static T_APP_RESULT app_gfps_security_condition_check(T_APP_LE_LINK *p_le_link,
                                                      T_GFPS_CALLBACK_DATA *p_callback)
{
    /*Keep a count of these failures. When the failure count hits 10, fail all new requests immediately.
    Reset the failure count after 5 minutes, after power on, or after a success.*/
    if (!app_gfps_check_failure_count(p_callback->msg_data.kbp.result))
    {
        return APP_RESULT_REJECT;
    };

    /*The value is decrypted successfully if the output matches the format in Table 1.2.1 or Table 1.2.2
    (that is, if it contains either the Fast Pair Provider's current BLE address,
     or the Fast Pair Provider's public address).*/
    if (!app_gfps_check_provider_addr(p_callback->msg_data.kbp.provider_addr))
    {
        return APP_RESULT_REJECT;
    };

    /*If the optional Public Key field is present.*/
    if (p_callback->msg_data.kbp.pk_field_exist == 1)
    {
        //If the device is not in pairing mode, ignore the write and exit.
        if (!p_callback->msg_data.kbp.retroactively_account_key && !gfps_db.force_enter_pair_mode)
        {
            APP_PRINT_ERROR0("app_gfps_security_condition_check: not in pairing mode");
            return APP_RESULT_APP_ERR;
        }
    }
    return APP_RESULT_SUCCESS;
}

static void app_gfps_set_io_cap_display_yes_no(T_APP_LE_LINK *p_le_link)
{
    uint8_t io_cap = GAP_IO_CAP_DISPLAY_YES_NO;
    uint16_t auth_flags;
    p_le_link->gfps_link.is_gfps_pairing = true;

    gap_get_param(GAP_PARAM_BOND_IO_CAPABILITIES, &(p_le_link->gfps_link.io_cap));
    gap_get_param(GAP_PARAM_BOND_AUTHEN_REQUIREMENTS_FLAGS, &(p_le_link->gfps_link.auth_flags));
    p_le_link->gfps_link.auth_param_change = true;
    auth_flags = (p_le_link->gfps_link.auth_flags | GAP_AUTHEN_BIT_MITM_FLAG);
    gap_set_param(GAP_PARAM_BOND_IO_CAPABILITIES, sizeof(uint8_t), &io_cap);
    gap_set_param(GAP_PARAM_BOND_AUTHEN_REQUIREMENTS_FLAGS, sizeof(uint16_t), &auth_flags);

    gap_set_pairable_mode();
    APP_PRINT_INFO2("app_gfps_set_io_cap_display_yes_no: gap_set_pairable_mode, io_cap %d, authen_flag 0x%x",
                    io_cap, auth_flags);

}

T_APP_RESULT app_gfps_cb_handle_kbp_write_req(T_SERVER_ID service_id,
                                              T_GFPS_CALLBACK_DATA *p_callback)
{
    T_APP_RESULT  app_result = APP_RESULT_SUCCESS;
    T_APP_LE_LINK *p_le_link = app_find_le_link_by_conn_id(p_callback->conn_id);

    DBG_DIRECT("app_gfps_cb_handle_kbp_write_req conn_id = 0x%x, link = 0x%x", p_callback->conn_id,
               p_le_link);

    if (!p_le_link)
    {
        return APP_RESULT_APP_ERR;
    }
    else
    {
        app_gfps_ble_conn_info_init(p_callback->conn_id);

        APP_PRINT_INFO7("app_gfps_cb_handle_kbp_write_req: provider bond %d, notify name %d, "
                        "retroactively %d, pk exist %d, accountkey idx %d, provider addr %s, seek addr %s",
                        p_callback->msg_data.kbp.provider_init_bond,
                        p_callback->msg_data.kbp.notify_existing_name,
                        p_callback->msg_data.kbp.retroactively_account_key,
                        p_callback->msg_data.kbp.pk_field_exist,
                        p_callback->msg_data.kbp.account_key_idx,
                        TRACE_BDADDR(p_callback->msg_data.kbp.provider_addr),
                        TRACE_BDADDR(p_callback->msg_data.kbp.seek_br_edr_addr));

        T_APP_RESULT gfps_check_result = app_gfps_security_condition_check(p_le_link, p_callback);
        if (gfps_check_result != APP_RESULT_SUCCESS)
        {
#if CONFIG_REALTEK_GFPS_FINDER_SUPPORT
            if (app_gfps_cfg.gfps_finder_support &&
                (app_gfps_cfg.gfps_device_type == GFPS_LOCATOR_TRACKER))
            {
                APP_PRINT_INFO0("app_gfps_cb_handle_kbp_write_req: location tags ignore gfps_check_result");
            }
            else
#endif
            {
                APP_PRINT_ERROR1("app_gfps_security_condition_check: err result 0x%x", gfps_check_result);
                return gfps_check_result;
            }
        }
    }

    app_gfps_set_allow_write_account_key(true);

    if (p_callback->msg_data.kbp.retroactively_account_key == 1)
    {
        /*If the Provider is bonded without going through the Fast Pair flow,
        allow a new account key to be written through the Key-based Pairing method for up to one minute.
        Only accept one account key to be written during this time.*/
        gfps_db.is_gfps_retroactive_pairing = true;
        app_gfps_timer_retroactively_start_check_accountkey();

        /*If an Raw Request with Flags bit 3 set is received,
        the Provider should verify the bonded device's BR/EDR address is the same as what is included in the request.
        If not, reject the request.*/
        T_APP_LE_LINK *p_curr_le_link = app_find_le_link_by_addr(
                                            p_callback->msg_data.kbp.seek_br_edr_addr);

        APP_PRINT_INFO1("app_gfps_cb_handle_kbp_write_req: p_curr_le_link 0x%x",
                        p_curr_le_link);

        if (!p_curr_le_link)
        {
            gfps_db.allow_write_account_key = false;
            APP_PRINT_ERROR1("app_gfps_cb_handle_kbp_write_req: err addr %s, allow_write_account_key false",
                             TRACE_BDADDR(p_callback->msg_data.kbp.seek_br_edr_addr));
            return APP_RESULT_REJECT;
        }
        else
        {
            memcpy(p_le_link->gfps_link.edr_bond_bd_addr, p_callback->msg_data.kbp.seek_br_edr_addr, 6);
        }
    }
    else
    {
#if CONFIG_REALTEK_GFPS_FINDER_SUPPORT
        if (app_gfps_cfg.gfps_finder_support &&
            (app_gfps_cfg.gfps_device_type == GFPS_LOCATOR_TRACKER))
        {
            APP_PRINT_INFO0("app_gfps_cb_handle_kbp_write_req: location tags ignore pair procedure");
            return app_result;
        }
#endif

        app_gfps_set_io_cap_display_yes_no(p_le_link);

        if (p_callback->msg_data.kbp.provider_init_bond)
        {
            gap_br_pairing_req(p_callback->msg_data.kbp.seek_br_edr_addr);
        }
        else
        {
            /*Wait up to 10 seconds for a pairing request. If none is received, exit.*/
            app_gfps_set_receive_pair_req(false);
            app_gfps_timer_start_check_receive_pair_req();
        }

#if CONFIG_REALTEK_GFPS_LE_DEVICE_SUPPORT
        if (app_gfps_cfg.gfps_le_device_support)
        {
            T_GFPS_LE_DEVICE_MODE gfps_le_device_mode = gfps_le_get_device_mode();
            if (gfps_le_device_mode == GFPS_LE_DEVICE_MODE_LE_MODE_WITH_LEA ||
                gfps_le_device_mode == GFPS_LE_DEVICE_MODE_DUAL_MODE_WITH_LEA)
            {
                uint8_t change_io_cap = true;
            }
        }
#endif

#if GFPS_PERSONALIZED_NAME_SUPPORT
        //If the Request's Flags byte has bit 2 set to 1, notify the personalized name characteristic(p_callback->msg_data.kbp.notify_existing_name)
        if (p_callback->msg_data.kbp.notify_existing_name)
        {
            /*need get personalized name from ftl and send the name into gfps*/
            app_gfps_personalized_name_send(p_callback->conn_id, service_id);
        }
#endif

        if (p_callback->msg_data.kbp.pk_field_exist == 0)
        {
        }
    }
    return app_result;
}

T_APP_RESULT app_gfps_cb_handle_action_req(T_GFPS_CALLBACK_DATA *p_callback)
{
    T_APP_RESULT app_result = APP_RESULT_SUCCESS;

    if (p_callback->msg_data.action_req.result == GFPS_WRITE_RESULT_SUCCESS)
    {
        APP_PRINT_INFO5("app_gfps_cb_handle_action_req: pk_field_exist %d, account_key_idx %d, "
                        "device_action %d, additional_data %d, provider_addr %s",
                        p_callback->msg_data.action_req.pk_field_exist,
                        p_callback->msg_data.action_req.account_key_idx,
                        p_callback->msg_data.action_req.device_action,
                        p_callback->msg_data.action_req.additional_data,
                        TRACE_BDADDR(p_callback->msg_data.action_req.provider_addr)
                       );
    }
    else
    {
        APP_PRINT_ERROR1("app_gfps_cb_handle_action_req: failed, result %d",
                         p_callback->msg_data.action_req.result);
    }

    return app_result;
}

T_APP_RESULT app_gfps_cb_handle_passkey(T_GFPS_CALLBACK_DATA *p_callback)
{
    T_APP_LE_LINK *p_link;
    p_link = app_find_le_link_by_conn_id(p_callback->conn_id);
    if (p_link == NULL)
    {
        return APP_RESULT_APP_ERR;
    }

    gfps_db.receive_passkey = true;
    T_APP_RESULT app_result = APP_RESULT_SUCCESS;
    p_link->gfps_link.gfps_raw_passkey = p_callback->msg_data.passkey;
    p_link->gfps_link.gfps_raw_passkey_received = true;

    APP_PRINT_INFO3("app_gfps_cb_handle_passkey: gfps_raw_passkey %d ,le_bond_confirm_pending %d, edr_bond_confirm_pending %d",
                    p_link->gfps_link.gfps_raw_passkey,
                    p_link->gfps_link.le_bond_confirm_pending,
                    p_link->gfps_link.edr_bond_confirm_pending);

    if (p_link->gfps_link.le_bond_confirm_pending)
    {
        app_gfps_send_le_bond_confirm(p_callback->conn_id);
        p_link->gfps_link.gfps_raw_passkey_received = false;
    }
    else if (p_link->gfps_link.edr_bond_confirm_pending)
    {
        app_gfps_send_legacy_bond_confirm(p_callback->conn_id);
        p_link->gfps_link.gfps_raw_passkey_received = false;
    }

#if CONFIG_REALTEK_GFPS_LE_DEVICE_SUPPORT
    if (gfps_get_pairing_status() == GFPS_PAIRING_STATUS_SUBSEQUENT &&
        app_gfps_cfg.gfps_le_device_support)
    {
        T_GFPS_LE_DEVICE_MODE gfps_le_device_mode = gfps_le_get_device_mode();

        if ((gfps_le_device_mode == GFPS_LE_DEVICE_MODE_LE_MODE_WITH_LEA  ||
             gfps_le_device_mode == GFPS_LE_DEVICE_MODE_DUAL_MODE_WITH_LEA))
        {
            APP_PRINT_INFO0("app_gfps_cb_handle_passkey: start gfps and le audio adv for subsequent pairing");
            app_gfps_le_device_adv_start();
        }

        if (p_link->gfps_link.auth_param_change)
        {
            APP_PRINT_INFO2("app_gfps_cb_handle_passkey: gap_set_pairable_mode, io_cap %d, authen_flag 0x%x",
                            p_link->gfps_link.io_cap, p_link->gfps_link.auth_flags);
            //provider recover to original io capability
            gap_set_param(GAP_PARAM_BOND_IO_CAPABILITIES, sizeof(uint8_t), &(p_link->gfps_link.io_cap));
            gap_set_param(GAP_PARAM_BOND_AUTHEN_REQUIREMENTS_FLAGS, sizeof(uint16_t),
                          &(p_link->gfps_link.auth_flags));
            gap_set_pairable_mode();
            p_link->gfps_link.auth_param_change = false;
        }
    }
#endif

    return app_result;
}

T_APP_RESULT app_gfps_cb_handle_accountkey(T_GFPS_CALLBACK_DATA *p_callback)
{
    T_APP_LE_LINK *p_link;
    p_link = app_find_le_link_by_conn_id(p_callback->conn_id);

    if (p_link == NULL)
    {
        return APP_RESULT_APP_ERR;
    }

    /*If the Provider is bonded without going through the Fast Pair flow,
     allow a new account key to be written through the Key-based Pairing method for up to one minute.
     Only accept one account key to be written during this time.*/
    if (gfps_db.is_gfps_retroactive_pairing)
    {
        gfps_db.account_key_write_counts++;
        gfps_db.is_gfps_retroactive_pairing = false;

        if (gfps_db.account_key_write_counts > 1)
        {
            APP_PRINT_ERROR1("app_gfps_cb_handle_accountkey: account_key_write_counts %d",
                             gfps_db.account_key_write_counts);
            return APP_RESULT_VALUE_NOT_ALLOWED;
        }

        if (!gfps_db.allow_write_account_key)
        {
            APP_PRINT_ERROR1("app_gfps_cb_handle_accountkey: allow_write_account_key %d",
                             gfps_db.allow_write_account_key);
            return APP_RESULT_VALUE_NOT_ALLOWED;
        }
    }

    /*for initial pairing or retroactive write account key receive account key from seeker*/
    if (app_gfps_account_key_store(p_callback->msg_data.account_key,
                                   p_link->gfps_link.edr_bond_bd_addr))
    {
#if CONFIG_REALTEK_GFPS_LE_DEVICE_SUPPORT
        if (app_gfps_cfg.gfps_le_device_support)
        {
            app_gfps_le_device_adv_start();

            T_GFPS_LE_DEVICE_MODE gfps_le_device_mode = gfps_le_get_device_mode();

            if ((gfps_le_device_mode == GFPS_LE_DEVICE_MODE_LE_MODE_WITH_LEA  ||
                 gfps_le_device_mode == GFPS_LE_DEVICE_MODE_DUAL_MODE_WITH_LEA))
            {
            }
        }
#endif
    }

#if CONFIG_REALTEK_GFPS_LE_DEVICE_SUPPORT
    if (app_gfps_cfg.gfps_le_device_support)
    {
        if (p_link->gfps_link.auth_param_change)
        {
            APP_PRINT_INFO2("app_gfps_cb_handle_accountkey: gap_set_pairable_mode, io_cap %d, authen_flag 0x%x",
                            p_link->gfps_link.io_cap, p_link->gfps_link.auth_flags);
            //provider recover to original io capability;
            gap_set_param(GAP_PARAM_BOND_IO_CAPABILITIES, sizeof(uint8_t), &(p_link->gfps_link.io_cap));
            gap_set_param(GAP_PARAM_BOND_AUTHEN_REQUIREMENTS_FLAGS, sizeof(uint16_t),
                          &(p_link->gfps_link.auth_flags));
            gap_set_pairable_mode();
            p_link->gfps_link.auth_param_change = false;
        }
    }
#endif

    return APP_RESULT_SUCCESS;
}

T_APP_RESULT app_gfps_cb_handle_additional_data(T_GFPS_CALLBACK_DATA *p_callback)
{
    T_APP_RESULT app_result = APP_RESULT_SUCCESS;

    //personalized name need to be stored in ftl
#if GFPS_PERSONALIZED_NAME_SUPPORT
    if (p_callback->msg_data.additional_data.data_id == GFPS_DATA_ID_PERSONALIZED_NAME)
    {
        T_GFPS_PERSON_NAME_RST result = app_gfps_personalized_name_store(
                                            p_callback->msg_data.additional_data.p_data,
                                            p_callback->msg_data.additional_data.data_len);
        if (result == APP_GFPS_PERSONALIZED_NAME_SUCCESS)
        {
            app_gfps_personalized_name_remote_sync();
        }

        APP_PRINT_INFO4("app_gfps_cb_handle_additional_data: personalized name %s, result %d, data_id 0x%x, data_len %d",
                        TRACE_STRING(p_callback->msg_data.additional_data.p_data), result,
                        p_callback->msg_data.additional_data.data_id,
                        p_callback->msg_data.additional_data.data_len);
    }
#endif

    return app_result;
}

void app_gfps_set_ble_conn_param(uint8_t conn_id)
{
    uint16_t conn_interval_min = 0x06;
    uint16_t conn_interval_max = 0x06;
    uint16_t conn_latency = 0;
    uint16_t conn_supervision_timeout = 500;

    ble_set_prefer_conn_param(conn_id, conn_interval_min, conn_interval_max, conn_latency,
                              conn_supervision_timeout);
}

/*Fast pair service callback*/
static T_APP_RESULT app_gfps_cb(T_SERVER_ID service_id, void *p_data)
{
    uint8_t ret = 0;
    T_APP_RESULT app_result = APP_RESULT_SUCCESS;
    T_GFPS_CALLBACK_DATA *p_callback = (T_GFPS_CALLBACK_DATA *)p_data;
    DBG_DIRECT("app_gfps_cb msg_type = 0x%x", p_callback->msg_type);
    switch (p_callback->msg_type)
    {
    case GFPS_CALLBACK_TYPE_NOTIFICATION_ENABLE:
        {
            APP_PRINT_INFO1("app_gfps_cb: GFPS_CALLBACK_TYPE_NOTIFICATION_ENABLE, T_GFPS_NOTIFICATION_TYPE %d",
                            p_callback->msg_data.notify_type);
        }
        break;

    case GFPS_CALLBACK_TYPE_KBP_WRITE_REQ:
        {
            app_result = app_gfps_cb_handle_kbp_write_req(service_id, p_callback);
        }
        break;

    case GFPS_CALLBACK_TYPE_ACTION_REQ:
        {
            app_result = app_gfps_cb_handle_action_req(p_callback);
        }
        break;

    case GFPS_CALLBACK_TYPE_PASSKEY:
        {
            app_result = app_gfps_cb_handle_passkey(p_callback);
        }
        break;

    case GFPS_CALLBACK_TYPE_ACCOUNT_KEY:
        {
            app_result = app_gfps_cb_handle_accountkey(p_callback);

#if CONFIG_REALTEK_GFPS_FINDER_SUPPORT
            if (app_gfps_cfg.gfps_finder_support &&
                (app_gfps_cfg.gfps_device_type == GFPS_LOCATOR_TRACKER))
            {
                app_gfps_timer_start_tag_auto_reset();
            }
#endif
        }
        break;

#if GFPS_ADDTIONAL_DATA_SUPPORT
    case GFPS_CALLBACK_TYPE_ADDITIONAL_DATA:
        {
            app_result = app_gfps_cb_handle_additional_data(p_callback);
        }
        break;
#endif

#if CONFIG_REALTEK_GFPS_LE_DEVICE_SUPPORT
    case GFPS_CALLBACK_TYPE_ADDITIONAL_PASSKEY:
        {
            if (app_gfps_cfg.gfps_le_device_support)
            {
            }
        }
        break;
#endif

    default:
        break;
    }

    return app_result;
}

/**
 * @brief non discoverable mode has two submode:show pop up ui or hide pop up ui
 * hide pop up ui: we want to stop showing the subsequent pairing notification since that pairing could be rejected
 *
 */
void app_gfps_enter_nondiscoverable_mode()
{
    APP_PRINT_INFO0("app_gfps_enter_nondiscoverable_mode");
    app_gfps_next_action(GFPS_ACTION_ADV_NOT_DISCOVERABLE_MODE);
}

bool app_gfps_get_force_enter_pair_mode_state(void)
{
    return gfps_db.force_enter_pair_mode;
}

void app_gfps_force_enter_pairing_mode(uint8_t status)
{
    switch (status)
    {
    case GFPS_KEY_FORCE_ENTER_PAIR_MODE:
    case GFPS_LE_DISCONN_FORCE_ENTER_PAIR_MODE:
    case GFPS_BT_POLICY_FORCE_ENTER_PAIR_MODE:
        {
            gfps_db.force_enter_pair_mode = true;
        }
        break;

    case GFPS_EXIT_PAIR_MODE:
    case GFPS_BT_POLICY_FORCE_EXIT_PAIR_MODE:
        {
            gfps_db.force_enter_pair_mode = false;
        }
        break;

    default:
        break;
    }

#if CONFIG_REALTEK_GFPS_LE_DEVICE_SUPPORT
    if (app_gfps_cfg.gfps_le_device_support)
    {
        APP_PRINT_INFO2("app_gfps_force_enter_pairing_mode: gfps le enable, status %d, force_enter_pair_mode %d",
                        status, gfps_db.force_enter_pair_mode);
        app_gfps_le_device_adv_start();
    }
    else
#endif
    {
        if (gfps_db.force_enter_pair_mode)
        {
            APP_PRINT_INFO2("app_gfps_force_enter_pairing_mode: gfps legacy enable, status %d, force_enter_pair_mode %d",
                            status, gfps_db.force_enter_pair_mode);
            app_gfps_next_action(GFPS_ACTION_ADV_DISCOVERABLE_MODE_WITH_MODEL_ID);
        }
    }
}

#if CONFIG_REALTEK_GFPS_LE_DEVICE_SUPPORT
void app_gfps_le_device_adv_start(void)
{
    APP_PRINT_INFO1("app_gfps_le_device_adv_start: force enter pairing mode %d",
                    gfps_db.force_enter_pair_mode);

    uint8_t link_num = app_gfps_link_find_gfps_link();
    T_GFPS_LE_DEVICE_MODE gfps_le_device_mode = gfps_le_get_device_mode();

    if (link_num >= GFPS_LE_MAX_LINK_NUMBER)
    {
        app_gfps_next_action(GFPS_ACTION_ADV_NOT_DISCOVERABLE_MODE_HIDE_UI);
        return;
    }

    T_ACCOUNT_KEY *p_key_info = app_gfps_account_key_get_table();

    if (gfps_db.force_enter_pair_mode)
    {
#if CONFIG_REALTEK_GFPS_FINDER_SUPPORT
        if (app_gfps_cfg.gfps_finder_support &&
            (app_gfps_cfg.gfps_device_type == GFPS_LOCATOR_TRACKER) &&
            app_gfps_finder_provisoned())
        {
            //Discoverable Fast Pair frames shouldn't be advertised while the Provider is provisioned for FHN.
            APP_PRINT_ERROR0("app_gfps_le_device_adv_start: tag already provisioned, please do factory reset firstly and then enter pairing mode");
            return;
        }
        else
#endif
        {
            app_gfps_next_action(GFPS_ACTION_ADV_DISCOVERABLE_MODE_WITH_MODEL_ID);
        }
    }
    else
    {
#if CONFIG_REALTEK_GFPS_FINDER_SUPPORT
        if (app_gfps_cfg.gfps_finder_support &&
            (app_gfps_cfg.gfps_device_type == GFPS_LOCATOR_TRACKER))
        {
            //When advertising non-discoverable Fast Pair frames, UI indications shouldn't be enabled.
            app_gfps_next_action(GFPS_ACTION_ADV_NOT_DISCOVERABLE_MODE_HIDE_UI);
        }
        else
#endif
        {
            app_gfps_next_action(GFPS_ACTION_ADV_NOT_DISCOVERABLE_MODE);
        }
    }
}
#endif

void app_gfps_handle_b2s_ble_bonded(uint8_t conn_id, uint8_t *p_remote_identity_addr)
{
    T_APP_LE_LINK *p_link;
    p_link = app_find_le_link_by_conn_id(conn_id);

    if (p_link == NULL)
    {
        return;
    }

    if (p_link->gfps_link.is_gfps_pairing)
    {
        APP_PRINT_INFO2("app_gfps_handle_b2s_ble_bonded: is_gfps_pairing %d, remote identity address %b",
                        p_link->gfps_link.is_gfps_pairing, TRACE_BDADDR(p_remote_identity_addr));

        memcpy(p_link->gfps_link.edr_bond_bd_addr, p_remote_identity_addr, 6);

        T_GFPS_LE_DEVICE_MODE gfps_le_device_mode = gfps_le_get_device_mode();

        if ((gfps_le_device_mode == GFPS_LE_DEVICE_MODE_LE_MODE_WITH_LEA ||
             gfps_le_device_mode == GFPS_LE_DEVICE_MODE_LE_MODE_WITHOUT_LEA))
        {
            T_GFPS_PAIRING_STATUS gfps_pair_status = GFPS_PAIRING_STATUS_IDLE;
            gfps_set_pairing_status(gfps_pair_status);
            app_gfps_force_enter_pairing_mode(GFPS_EXIT_PAIR_MODE);
        }
    }
    else
    {
#if CONFIG_REALTEK_GFPS_LE_DEVICE_SUPPORT
        if (app_gfps_cfg.gfps_le_device_support)
        {
            app_gfps_le_device_adv_start();
        }
#endif
    }
}

static void gfps_module_init(void)
{
    app_gfps_cfg_init();
    if (app_gfps_cfg.gfps_support)
    {
        gfps_ecc_init_msg_queue(evt_queue_handle, io_queue_handle);
        app_gfps_msg_rfc_init();
        app_gfps_init();
        app_ble_gap_msg_handle_register(&gfps_adv);
    }
#if CONFIG_REALTEK_GFPS_FINDER_SUPPORT
    if (app_gfps_cfg.gfps_finder_support)
    {
        app_dult_device_init();
        app_dult_svc_init();
        app_ble_gap_msg_handle_register(&gfps_finder_adv);
    }
#endif
    event_bus_topic_register(EVENT_BUS_TOPIC_ECC_MSG_RELAY);
    event_bus_subscribe(&gfps_ecc_msg_relay_handle,
                        EVENT_BUS_TOPIC_ECC_MSG_RELAY,
                        app_ecc_handle_msg);
}
APP_MODULE_INIT(gfps_module_init);

APP_BLE_SERVICE_INFO(gfps, 2);

#endif
