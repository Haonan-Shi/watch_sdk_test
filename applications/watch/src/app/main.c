/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <zephyr/kernel.h>
#include <os_sched.h>
#include <stdio.h>
#include <trace.h>
#include "hub_task.h"
#include "app_task.h"
#include "gap_br.h"
#include "app_cfg.h"
#include "sysm.h"
#include "remote.h"
#include "btm.h"
#include "audio.h"
#include "app_gap.h"
#include "app_ble_gap.h"
#include "app_main.h"
#include "app_ble_service.h"
#include <gap_bond_le.h>
#include "communicate_task.h"
#include "app_avrcp.h"
#include "app_hfp.h"
#include "app_pbap.h"
#include "app_spp.h"
#include "app_sdp.h"
#include "app_audio_policy.h"
#include "app_bt_policy_api.h"
#include "app_multilink.h"
#include "clock_manager.h"
#include "fmc_api.h"
#include "system_status_api.h"
#include "os_mem.h"
#include "os_heap.h"
#include "app_a2dp.h"
#include "fmc_api_ext.h"
#include "app_dlps.h"
#include "app_amp.h"
#include "app_audio_if.h"
#include "app_playback_update_file.h"
#include "console_uart.h"
#include "auto_k_rf.h"
#include "app_transfer.h"
#include "dma_channel.h"
#include "app_cmd.h"
#include <rtl876x_gpio.h>
#include <rtl876x_pinmux.h>
#include "app_cfg.h"
#include <rtl876x_rcc.h>
#include <fmc_api.h>
#include "app_cmd.h"
#include "app_audio_mode_switch.h"
#include "audio_record.h"
#include "app_bt_policy_api.h"
#include "app_util.h"
#include "app_lower_init.h"
#include "bt_bond_api.h"
#include "app_ble_adv.h"
#include "app_module_init.h"
#include "event_bus.h"
#if CONFIG_DFU_NORMAL_OTA
#include "dfu_common.h"
#include "dfu_main.h"
#endif
#if CONFIG_REALTEK_HONEYGUI
#include "gui_server.h"
#endif
#if CONFIG_REALTEK_BUILD_GUI_410_502_DEMO
#include "gui_app_port.h"
#endif

#if CONFIG_REALTEK_BUILD_LVGL && CONFIG_REALTEK_BUILD_LVGL_SIMPLE_DEMO
#include "app_rtk_port.h"
#endif
#if CONFIG_SC_KEY_DERIVE
#include "app_ble_sc_key_derive.h"
#endif
/** @brief  Default minimum advertising interval when device is discoverable (units of 625us, 160=100ms) */
#define DEFAULT_PAGESCAN_WINDOW                     0x12
#define DEFAULT_PAGESCAN_INTERVAL                   0x800 //0x800
#define DEFAULT_PAGE_TIMEOUT                        0x2000
#define DEFAULT_SUPVISIONTIMEOUT                    0x1f40 //0x7D00
#define DEFAULT_INQUIRYSCAN_WINDOW                  0x12
#define DEFAULT_INQUIRYSCAN_INTERVAL                0x800 //0x1000

T_APP_DB app_db;
bool normal_ota_mode_check = false;

void app_le_gap_init(void)
{
    /* Device name and device appearance */
    uint16_t appearance = GAP_GATT_APPEARANCE_UNKNOWN;
    uint8_t  slave_init_mtu_req = true;

    /* GAP Bond Manager parameters */
    uint8_t  auth_use_fix_passkey = false;
    uint32_t auth_fix_passkey = 0;
#if CONFIG_REALTEK_SUBSYS_GATT_PROFILE_ANCS_CLIENT
    uint8_t  auth_sec_req_enable = true;
#else
    uint8_t  auth_sec_req_enable = false;
#endif
    uint16_t auth_sec_req_flags = GAP_AUTHEN_BIT_BONDING_FLAG;

    le_gap_init(CONFIG_BLE_LINKS);

    le_set_gap_param(GAP_PARAM_APPEARANCE, sizeof(appearance), &appearance);
    le_set_gap_param(GAP_PARAM_SLAVE_INIT_GATT_MTU_REQ, sizeof(slave_init_mtu_req),
                     &slave_init_mtu_req);


    /* Setup the GAP Bond Manager */
    le_bond_set_param(GAP_PARAM_BOND_FIXED_PASSKEY, sizeof(auth_fix_passkey), &auth_fix_passkey);
    le_bond_set_param(GAP_PARAM_BOND_FIXED_PASSKEY_ENABLE, sizeof(auth_use_fix_passkey),
                      &auth_use_fix_passkey);
    le_bond_set_param(GAP_PARAM_BOND_SEC_REQ_ENABLE, sizeof(auth_sec_req_enable), &auth_sec_req_enable);
    le_bond_set_param(GAP_PARAM_BOND_SEC_REQ_REQUIREMENT, sizeof(auth_sec_req_flags),
                      &auth_sec_req_flags);

    /* register gap message callback */
    le_register_app_cb(app_gap_callback);

#if CONFIG_SC_KEY_DERIVE
    app_ble_key_derive_init();
#endif
}
static void app_bt_gap_init(void)
{
    uint32_t class_of_device = 0x280708;
    uint16_t supervision_timeout = DEFAULT_SUPVISIONTIMEOUT;
    uint16_t link_policy = GAP_LINK_POLICY_ROLE_SWITCH | GAP_LINK_POLICY_SNIFF_MODE;

    uint8_t radio_mode = GAP_RADIO_MODE_NONE_DISCOVERABLE;//GAP_RADIO_MODE_NONE_DISCOVERABLE;
    bool limited_discoverable = false;
    bool auto_accept_acl = false;

    uint8_t pagescan_type = GAP_PAGE_SCAN_TYPE_STANDARD;
    uint16_t pagescan_interval = DEFAULT_PAGESCAN_INTERVAL;
    uint16_t pagescan_window = DEFAULT_PAGESCAN_WINDOW;
    uint16_t page_timeout = DEFAULT_PAGE_TIMEOUT;

    uint8_t inquiryscan_type = GAP_INQUIRY_SCAN_TYPE_STANDARD;
    uint16_t inquiryscan_window = DEFAULT_INQUIRYSCAN_WINDOW;
    uint16_t inquiryscan_interval = DEFAULT_INQUIRYSCAN_INTERVAL;
    uint8_t inquiry_mode = GAP_INQUIRY_MODE_EXTENDED_RESULT;

    uint8_t pair_mode = GAP_PAIRING_MODE_PAIRABLE;
    uint16_t auth_flags = GAP_AUTHEN_BIT_GENERAL_BONDING_FLAG | GAP_AUTHEN_BIT_SC_FLAG;
    uint8_t io_cap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT;
    uint8_t oob_enable = false;
    uint8_t bt_mode = GAP_BT_MODE_21ENABLED;

    gap_lib_init();

    //0: to be master
    gap_br_cfg_accept_role(1);

    gap_br_set_param(GAP_BR_PARAM_NAME, GAP_DEVICE_NAME_LEN, app_cfg_nv.device_name_legacy);

    gap_set_param(GAP_PARAM_BOND_PAIRING_MODE, sizeof(uint8_t), &pair_mode);
    gap_set_param(GAP_PARAM_BOND_AUTHEN_REQUIREMENTS_FLAGS, sizeof(uint16_t), &auth_flags);
    gap_set_param(GAP_PARAM_BOND_IO_CAPABILITIES, sizeof(uint8_t), &io_cap);
    gap_set_param(GAP_PARAM_BOND_OOB_ENABLED, sizeof(uint8_t), &oob_enable);

    gap_br_set_param(GAP_BR_PARAM_BT_MODE, sizeof(uint8_t), &bt_mode);
    gap_br_set_param(GAP_BR_PARAM_COD, sizeof(uint32_t), &class_of_device);
    gap_br_set_param(GAP_BR_PARAM_LINK_POLICY, sizeof(uint16_t), &link_policy);
    gap_br_set_param(GAP_BR_PARAM_SUPV_TOUT, sizeof(uint16_t), &supervision_timeout);
    gap_br_set_param(GAP_BR_PARAM_AUTO_ACCEPT_ACL, sizeof(bool), &auto_accept_acl);

    gap_br_set_param(GAP_BR_PARAM_RADIO_MODE, sizeof(uint8_t), &radio_mode);
    gap_br_set_param(GAP_BR_PARAM_LIMIT_DISCOV, sizeof(bool), &limited_discoverable);

    gap_br_set_param(GAP_BR_PARAM_PAGE_SCAN_TYPE, sizeof(uint8_t), &pagescan_type);
    gap_br_set_param(GAP_BR_PARAM_PAGE_SCAN_INTERVAL, sizeof(uint16_t), &pagescan_interval);
    gap_br_set_param(GAP_BR_PARAM_PAGE_SCAN_WINDOW, sizeof(uint16_t), &pagescan_window);
    gap_br_set_param(GAP_BR_PARAM_PAGE_TIMEOUT, sizeof(uint16_t), &page_timeout);

    gap_br_set_param(GAP_BR_PARAM_INQUIRY_SCAN_TYPE, sizeof(uint8_t), &inquiryscan_type);
    gap_br_set_param(GAP_BR_PARAM_INQUIRY_SCAN_INTERVAL, sizeof(uint16_t), &inquiryscan_interval);
    gap_br_set_param(GAP_BR_PARAM_INQUIRY_SCAN_WINDOW, sizeof(uint16_t), &inquiryscan_window);
    gap_br_set_param(GAP_BR_PARAM_INQUIRY_MODE, sizeof(uint8_t), &inquiry_mode);

}

static void framework_init(void)
{
    /* Device Manager */
    sys_mgr_init(evt_queue_handle);

    /* RemoteController Manager */
    remote_mgr_init((T_REMOTE_SESSION_ROLE)app_cfg_nv.bud_role);
    remote_local_addr_set(app_cfg_nv.bud_local_addr);
    remote_peer_addr_set(app_cfg_nv.bud_peer_addr);
    /* Bluetooth Manager */
    bt_mgr_init();

    /* Audio Manager */
    audio_mgr_init(PLAYBACK_POOL_SIZE, VOICE_POOL_SIZE, RECORD_POOL_SIZE, NOTIFICATION_POOL_SIZE);
}

static void task_init(void)
{
    app_task_init();
    communicate_task_init();
    hub_task_init();
#if CONFIG_REALTEK_HONEYGUI
    gui_server_init();
#endif
#if CONFIG_REALTEK_BUILD_GUI_410_502_DEMO
    gui_app_port_audio_init();
#endif
#if CONFIG_REALTEK_BUILD_LVGL && CONFIG_REALTEK_BUILD_LVGL_SIMPLE_DEMO
    WDG_Disable();
    rt_lvgl_demo_init();
#endif
}

void dfu_spp_init(void)
{
    normal_ota_mode_check = true;
    app_cfg_init();
    memcpy(&app_cfg_nv.device_name_legacy[0], app_cfg_const.device_name_legacy_default, 40);
    memcpy(&app_cfg_nv.device_name_le[0], app_cfg_const.device_name_le_default, 40);

    bt_bond_init();
    app_bt_gap_init();
    framework_init();
    app_gap_init();
    app_bt_policy_init();
    app_spp_init();
    app_sdp_init();
    linkback_init();
}

uint8_t mp_mode = 0;
int main(void)
{
    /* Dlps init and disable should invoke before any other initialization,
       or cpu may enter low power state unexpectedly */
    app_dlps_init();
    app_dlps_disable(APP_DLPS_ENTER_CHECK_INIT);

    app_system_lower_init();

    memset(&app_db, 0, sizeof(T_APP_DB));
    app_main_task_queue_create();
    event_bus_init();

#if CONFIG_DFU_NORMAL_OTA
    if (dfu_check_ota_mode_flag())
    {
        DBG_DIRECT("DFU TASK");
        if (dfu_check_ota_transport_spp())
        {
            dfu_spp_init();
            dfu_set_ota_transport_spp(false);
        }
        else
        {
            dfu_ble_init();
        }
        dfu_main();
        dfu_set_ota_mode_flag(false);
        return 0;
    }
#endif

    dfu_print_active_bootpatch_banknum();
    dfu_print_all_images_version();

    if (mp_mode)
    {
        framework_init();
        app_transfer_init();
#if (F_APP_AUTO_SUPPORT == 0)
        app_console_init();
#endif
        app_task_init();
        mp_hci_task_init();
        app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
    }
    else
    {
        app_cfg_init();
        bt_bond_init();
        app_ble_gap_ble_mgr_init();
        app_bt_gap_init();
        app_le_gap_init();
        framework_init();
        app_gap_init();
        app_ble_service_init();
        app_bt_policy_init();
        app_hfp_init();
        app_avrcp_init();
        app_pbap_init();
        app_spp_init();
        app_sdp_init();
        app_a2dp_init();
        linkback_init();

        app_audio_init();
        app_transfer_init();

        app_fs_init();
        app_audio_mode_switch_init();

        app_audio_interface_init();
        audio_record_init();

        app_external_amp_init();
        //TODO: remove on ASIC
        app_db.batt.allow_open.playback = true;

        app_module_init_all();
        task_init();

        app_util_print_all_tasks_info();
        app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);

        //Remove after UI linkage is completed
        app_audio_mode_switch(MODE_APP_A2DP_SNK);

    }
    return 0;
}
