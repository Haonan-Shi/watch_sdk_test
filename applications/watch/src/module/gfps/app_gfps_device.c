/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if CONFIG_REALTEK_GFPS_FEATURE_SUPPORT
#include "trace.h"
#include "app_gfps_device.h"
#include "app_gfps.h"
#include "app_main.h"
#include "app_gfps_account_key.h"
#include "app_gfps_personalized_name.h"
#include "app_gfps_cfg.h"
#include "app_bt_policy_api.h"
#include "bt_bond.h"
#include "app_bond.h"
#include "gap_bond_le.h"
#include "app_gfps_cfg.h"
#include "app_gfps_link.h"
#include "app_link_util.h"
#if CONFIG_REALTEK_GFPS_FINDER_SUPPORT
#include "app_gfps_finder.h"
#include "app_gfps_finder_adv.h"
#include "app_adv_stop_cause.h"
#endif
void app_gfps_device_handle_power_off(void)
{
    APP_PRINT_TRACE0("app_gfps_device_handle_power_off");

    app_gfps_next_action(GFPS_ACTION_IDLE);
    app_gfps_link_disconnect_all_gfps_link();

#if CONFIG_REALTEK_GFPS_FINDER_SUPPORT
    if (app_gfps_cfg.gfps_finder_support)
    {
        app_gfps_finder_handle_power_off();
    }
#endif
}

void app_gfps_device_handle_power_on(bool is_pairing)
{
    APP_PRINT_TRACE0("app_gfps_device_handle_power_on");

#if CONFIG_REALTEK_GFPS_FINDER_SUPPORT
    if (app_gfps_cfg.gfps_finder_support)
    {
        app_gfps_finder_set_ring_param();
    }
#endif

#if CONFIG_REALTEK_GFPS_FINDER_SUPPORT
    if (app_gfps_cfg.gfps_finder_support)
    {
        T_GFPS_FINDER_BEACON_STATE beacon_state = GFPS_FINDER_BEACON_STATE_ON;
        uint32_t adv_interval = app_gfps_cfg.gfps_power_on_finder_adv_interval;

        gfps_finder_adv_update_adv_interval(adv_interval);
        app_gfps_finder_enter_beacon_state(beacon_state);
    }
#endif

#if CONFIG_REALTEK_GFPS_LE_DEVICE_SUPPORT
    uint8_t bt_addr[6] = {0};
    gap_get_param(GAP_PARAM_BD_ADDR, bt_addr);
    gfps_set_identity_address(bt_addr, NULL, false);
    app_gfps_le_device_adv_start();


    uint8_t init_key = GFPS_LE_SMP_DIST_ENC_KEY | GFPS_LE_SMP_DIST_ID_KEY;
    uint8_t response_key = GFPS_LE_SMP_DIST_ENC_KEY | GFPS_LE_SMP_DIST_ID_KEY;
    le_bond_cfg_local_key_distribute(init_key, response_key);

    /*rws primary bud or single mode shall set GFPS_PSM_STATE_READY*/
    T_GFPS_PSM_RSP gfps_psm_rsp;
    gfps_psm_rsp.psm = GFPS_PSM_MSG_STREAM;
    gfps_psm_rsp.state = GFPS_PSM_STATE_READY;
    gfps_psm_rsp_data_set(&gfps_psm_rsp);
#endif
}
void app_gfps_device_handle_ble_link_disconnected(uint8_t local_disc_cause)
{
    APP_PRINT_TRACE2("app_gfps_device_handle_ble_link_disconnected: local_disc_cause %d, force enter pair mode %d",
                     local_disc_cause, app_gfps_cfg.gfps_le_disconn_force_enter_pairing_mode);

#if CONFIG_REALTEK_GFPS_LE_DEVICE_SUPPORT
    if (app_gfps_cfg.gfps_le_device_support)
    {
        if (app_gfps_cfg.gfps_le_disconn_force_enter_pairing_mode)
        {
            app_gfps_force_enter_pairing_mode(GFPS_LE_DISCONN_FORCE_ENTER_PAIR_MODE);
            app_gfps_le_device_adv_start();
        }
        else
        {
            app_gfps_le_device_adv_start();
        }
    }
#endif
}

void app_gfps_device_handle_factory_reset(void)
{
    APP_PRINT_TRACE0("app_gfps_device_handle_factory_reset");

    app_gfps_account_key_clear();

#if GFPS_PERSONALIZED_NAME_SUPPORT
    app_gfps_personalized_name_clear();
#endif

#if CONFIG_REALTEK_GFPS_FINDER_SUPPORT
    if (app_gfps_cfg.gfps_finder_support)
    {
        app_gfps_finder_handle_factory_reset();
    }
#endif
}
#endif
