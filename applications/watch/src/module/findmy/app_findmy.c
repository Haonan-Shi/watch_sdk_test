/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include "app_cmd.h"
#include "app_findmy.h"
#include "app_findmy_ble.h"
#include "app_module_init.h"
#include "app_ble_service_info.h"
#include "app_mmi.h"
#include "fmna_connection_platform.h"
#include "fmna_state_machine.h"
#include "fmna_import_includes.h"
#include "fmna_motion_detection.h"
#include "fmna_crypto.h"
#include "fmna_connection.h"
#include "fmna_sound_platform.h"
#include "trace.h"
#include "app_timer.h"
#include "crypto/fm-crypto.h"
#include "bt_bond_le.h"
#include "bt_bond_le_sync.h"
#include "app_ble_gap.h"
#include "app_findmy_task.h"

T_APP_GLOBAL_DATA app_global_data;
static T_EVENT_BUS_SUBSCRIBER_HANDLE findmy_app_event_async_handle;

/*============================================================================*
 *                              Functions
 *============================================================================*/

void fmna_bond_info_restore(void)
{
    uint8_t bond_idx_buffer[FTL_SAVE_BOND_IDX_SIZE];
    if (!ftl_load_from_storage(bond_idx_buffer, FTL_SAVE_BOND_IDX_ADDR, FTL_SAVE_BOND_IDX_SIZE))
    {
        memcpy(app_global_data.app_bond_idx, bond_idx_buffer, BLE_BOND_NUM);
        APP_PRINT_INFO1("fmna_bond_info_restore: bond_idx %#x", app_global_data.app_bond_idx[FINDMY_APP]);
    }
    else
    {
        app_global_data.app_bond_idx[FINDMY_APP] = 0xFF;
        APP_PRINT_WARN0("fmna_bond_info_restore: failed!");
    }
}

void app_global_data_init(void)
{
    memset(&app_global_data, 0, sizeof(app_global_data));
    for (uint8_t i = 0; i < BLE_BOND_NUM; i++)
    {
        app_global_data.app_bond_idx[i] = 0xFF;
    }
}

void app_findmy_msg_bond_cb(uint8_t cb_type, void *p_cb_data)
{
    T_BT_LE_BOND_CB_DATA cb_data;
    memcpy(&cb_data, p_cb_data, sizeof(T_BT_LE_BOND_CB_DATA));

    T_APP_SELECT cur_bond_app = FINDMY_APP;

    switch (cb_type)
    {
    case BT_BOND_MSG_LE_BOND_ADD:
        {
            if (cb_data.p_le_bond_add->p_entry->bond_info.local_bd_type == GAP_LOCAL_ADDR_LE_RANDOM)
            {
                cur_bond_app = FINDMY_APP;
            }
#if SUPPORT_CUSTOMIZED_APP
            else if (cb_data.p_le_bond_add->p_entry->bond_info.local_bd_type == GAP_LOCAL_ADDR_LE_PUBLIC)
            {
                cur_bond_app = CUSTOMIZED_APP;
            }
#endif

            app_global_data.app_bond_idx[cur_bond_app] = cb_data.p_le_bond_add->p_entry->idx;

            /* if findmy app bonded, set to high priority */
            if (app_global_data.app_bond_idx[FINDMY_APP] != 0xFF)
            {
                T_LE_BOND_ENTRY *findmy_key_entry = NULL;
                findmy_key_entry = bt_le_find_key_entry_by_idx(app_global_data.app_bond_idx[FINDMY_APP]);
                bt_le_set_high_priority_bond(findmy_key_entry);
            }
            ftl_save_to_storage(app_global_data.app_bond_idx, FTL_SAVE_BOND_IDX_ADDR, FTL_SAVE_BOND_IDX_SIZE);

            APP_PRINT_INFO3("app_findmy_msg_bond_cb: modify_flags 0x%x, remote_bd_type %d, remote_bd %s",
                            cb_data.p_le_bond_add->modify_flags,
                            cb_data.p_le_bond_add->p_entry->remote_bd_type,
                            TRACE_BDADDR(cb_data.p_le_bond_add->p_entry->remote_bd));
        }
        break;

    case BT_BOND_MSG_LE_BOND_REMOVE:
        {
            if (cb_data.p_le_bond_remove->p_entry->bond_info.local_bd_type == GAP_LOCAL_ADDR_LE_RANDOM)
            {
                cur_bond_app = FINDMY_APP;
            }
#if SUPPORT_CUSTOMIZED_APP
            else if (cb_data.p_le_bond_remove->p_entry->bond_info.local_bd_type ==  GAP_LOCAL_ADDR_LE_PUBLIC)
            {
                cur_bond_app = CUSTOMIZED_APP;
            }
#endif

            app_global_data.app_bond_idx[cur_bond_app] = 0xFF;
            ftl_save_to_storage(app_global_data.app_bond_idx, FTL_SAVE_BOND_IDX_ADDR, FTL_SAVE_BOND_IDX_SIZE);
            APP_PRINT_INFO2("app_findmy_msg_bond_cb: remote_bd_type %d, remote_bd %s",
                            cb_data.p_le_bond_remove->p_entry->remote_bd_type,
                            TRACE_BDADDR(cb_data.p_le_bond_remove->p_entry->remote_bd));
        }
        break;

    case BT_BOND_MSG_LE_BOND_CLEAR:
        {
            APP_PRINT_INFO0("BT_BOND_MSG_LE_BOND_CLEAR");
            for (uint8_t i = 0; i < BLE_BOND_NUM; i++)
            {
                app_global_data.app_bond_idx[i] = 0xFF;
            }
            ftl_save_to_storage(app_global_data.app_bond_idx, FTL_SAVE_BOND_IDX_ADDR, FTL_SAVE_BOND_IDX_SIZE);
        }
        break;

    case BT_BOND_MSG_LE_BOND_UPDATE:
        {
            if (cb_data.p_le_bond_add->p_entry->bond_info.local_bd_type == GAP_LOCAL_ADDR_LE_RANDOM)
            {
                cur_bond_app = FINDMY_APP;
            }
#if SUPPORT_CUSTOMIZED_APP
            else if (cb_data.p_le_bond_add->p_entry->bond_info.local_bd_type == GAP_LOCAL_ADDR_LE_PUBLIC)
            {
                cur_bond_app = CUSTOMIZED_APP;
            }
#endif

            app_global_data.app_bond_idx[cur_bond_app] = cb_data.p_le_bond_update->p_entry->idx;

            /* if findmy app bonded, set to high priority */
            if (app_global_data.app_bond_idx[FINDMY_APP] != 0xFF)
            {
                T_LE_BOND_ENTRY *findmy_key_entry = NULL;
                findmy_key_entry = bt_le_find_key_entry_by_idx(app_global_data.app_bond_idx[FINDMY_APP]);
                bt_le_set_high_priority_bond(findmy_key_entry);
            }
            ftl_save_to_storage(app_global_data.app_bond_idx, FTL_SAVE_BOND_IDX_ADDR, FTL_SAVE_BOND_IDX_SIZE);

            APP_PRINT_INFO3("app_findmy_msg_bond_cb: modify_flags 0x%x, remote_bd_type %d, remote_bd %s",
                            cb_data.p_le_bond_add->modify_flags,
                            cb_data.p_le_bond_add->p_entry->remote_bd_type,
                            TRACE_BDADDR(cb_data.p_le_bond_add->p_entry->remote_bd));
        }
        break;

    default:
        break;
    }
    return;
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

void app_findmy_init(void)
{
    app_global_data_init();
    fmna_version_init();
    fmna_gatt_init();
    fmna_motion_detection_init();
    fmna_sound_platform_init();
    fmna_timer_init();
    fm_crypto_init();
    bt_bond_register_app_cb(app_findmy_msg_bond_cb);
    bt_bond_register_bond_get_cb(app_findmy_ble_bond_get_cb);
    app_ble_gap_msg_handle_register(&le_findmy_adv);

    event_bus_topic_register(EVENT_BUS_TOPIC_FINDMY_STATE_MACHINE_ALL_TOPIC);
    event_bus_subscribe_async(&findmy_app_event_async_handle,
                              EVENT_BUS_TOPIC_FINDMY_STATE_MACHINE_ALL_TOPIC,
                              event_bus_async_send_to_apptask,
                              NULL,
                              app_findmy_task_msg_handler);

}

static void findmy_module_init(void)
{
    app_findmy_init();
}
APP_MODULE_INIT(findmy_module_init);

APP_BLE_SERVICE_INFO(findmy, 3);

