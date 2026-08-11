/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if CONFIG_REALTEK_FINDMY_FEATURE_SUPPORT
#include "app_cmd.h"
#include "app_findmy.h"
#include "app_findmy_ble.h"
#include "app_mmi.h"
#include "fmna_connection_platform.h"
#include "fmna_state_machine.h"
#include "fmna_import_includes.h"
#include "fmna_motion_detection.h"
#include "fmna_crypto.h"
#include "fmna_connection.h"
#include "fmna_sound_platform.h"
#include "fmna_gatt_platform.h"
#include "trace.h"
#include "app_timer.h"
#include "fm-crypto.h"
#include "bt_bond_le.h"
#if CONFIG_REALTEK_FINDMY_SUPPORT_CUSTOMIZED_APP
#include "custom_app.h"
#endif

T_APP_GLOBAL_DATA app_global_data;

/*============================================================================*
 *                              Functions
 *============================================================================*/

void app_findmy_handle_cmd_set(uint8_t app_idx, uint8_t cmd_path, uint8_t *cmd_ptr,
                               uint16_t cmd_len, uint8_t *ack_pkt)
{
    uint16_t cmd_id = (uint16_t)(cmd_ptr[0] | (cmd_ptr[1] << 8));

    APP_PRINT_TRACE3("app_findmy_handle_cmd_set: cmd_id 0x%04x, cmd_len 0x%04x, cmd_path %u",
                     cmd_id, cmd_len, cmd_path);

    switch (cmd_id)
    {
    case CMD_FINDMY_FEATURE:
        {
            uint8_t action = cmd_ptr[2];

            if (action == FINDMY_ENTER_PAIRING)
            {
                app_findmy_enter_pair_mode();
            }
            else if (action == FINDMY_PUT_SERIAL_NUMBER)
            {
                app_mmi_handle_action(MMI_FINDMY_PUT_SERIAL_NUMBER_STATE);
            }
#if CONFIG_REALTEK_FINDMY_SUPPORT_CUSTOMIZED_APP
            else if (action == FINDMY_CUST_ADV_ENABLE)
            {
                cust_feature_enable();
            }
            else if (action == FINDMY_CUST_ADV_DISABLE)
            {
                cust_feature_disable();
            }
            else if (action == FINDMY_CUST_FACTORY_RESET)
            {
                cust_factory_reset();
            }
#endif
            else
            {
                ack_pkt[2] = CMD_SET_STATUS_PARAMETER_ERROR;
            }

            app_cmd_set_event_ack(cmd_path, app_idx, ack_pkt);
        }
        break;

    default:
        break;
    }
}

void fmna_bond_info_restore(void)
{
    uint8_t bond_idx_buffer[FTL_SAVE_BOND_IDX_SIZE];
    if (!ftl_load_from_storage(bond_idx_buffer, FTL_SAVE_BOND_IDX_ADDR, FTL_SAVE_BOND_IDX_SIZE))
    {
        memcpy(app_global_data.app_bond_idx, bond_idx_buffer, BLE_BOND_NUM);
        APP_PRINT_INFO1("fmna_bond_info_restore: bond_idx %#x", app_global_data.app_bond_idx[FINDMY_BOND]);

#if CONFIG_REALTEK_FINDMY_SUPPORT_CUSTOMIZED_APP
        if (app_global_data.app_bond_idx[CUST_BOND_1] != 0xFF ||
            app_global_data.app_bond_idx[CUST_BOND_2] != 0xFF)
        {
            cust_set_paired_flag(true);
        }
#endif
    }
    else
    {
        app_global_data.app_bond_idx[FINDMY_BOND] = 0xFF;
#if CONFIG_REALTEK_FINDMY_SUPPORT_CUSTOMIZED_APP
        app_global_data.app_bond_idx[CUST_BOND_1] = 0xFF;
        app_global_data.app_bond_idx[CUST_BOND_2] = 0xFF;
#endif
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

#if CONFIG_REALTEK_FINDMY_SUPPORT_CUSTOMIZED_APP
    cust_data_init();
#endif
}

void app_findmy_msg_bond_cb(uint8_t cb_type, void *p_cb_data)
{
    APP_PRINT_INFO1("app_findmy_msg_bond_cb: cb_type %d", cb_type);
    T_BT_LE_BOND_CB_DATA cb_data;
    memcpy(&cb_data, p_cb_data, sizeof(T_BT_LE_BOND_CB_DATA));

    T_APP_SELECT cur_bond_app = FINDMY_APP;

    switch (cb_type)
    {
    case BT_BOND_MSG_LE_BOND_ADD:
        {
            if (app_findmy_is_findmy_adv())
            {
                app_global_data.app_bond_idx[FINDMY_BOND] = cb_data.p_le_bond_add->p_entry->idx;
                APP_PRINT_INFO1("app_findmy_msg_bond_cb: add_idx %d", cb_data.p_le_bond_add->p_entry->idx);
            }
#if CONFIG_REALTEK_FINDMY_SUPPORT_CUSTOMIZED_APP
            else if (cust_app_is_cust_adv())
            {
                if ((app_global_data.app_bond_idx[CUST_BOND_1] != cb_data.p_le_bond_add->p_entry->idx) &&
                    (app_global_data.app_bond_idx[CUST_BOND_2] != cb_data.p_le_bond_add->p_entry->idx))
                {
                    if (app_global_data.app_bond_idx[CUST_BOND_1] == 0xFF)
                    {
                        app_global_data.app_bond_idx[CUST_BOND_1] = cb_data.p_le_bond_add->p_entry->idx;
                    }
                    else if (app_global_data.app_bond_idx[CUST_BOND_2] == 0xFF)
                    {
                        app_global_data.app_bond_idx[CUST_BOND_2] = cb_data.p_le_bond_add->p_entry->idx;
                    }
                    else
                    {
                        APP_PRINT_ERROR0("app_findmy_msg_bond_cb: CUST BOND error");
                    }
                }
            }
#endif

            APP_PRINT_INFO3("app_findmy_msg_bond_cb: add_idx %d, remote_bd_type %d, remote_bd %s",
                            cb_data.p_le_bond_add->p_entry->idx,
                            cb_data.p_le_bond_add->p_entry->remote_bd_type,
                            TRACE_BDADDR(cb_data.p_le_bond_add->p_entry->remote_bd));
        }
        break;

    case BT_BOND_MSG_LE_BOND_REMOVE:
        {
            if (app_global_data.app_bond_idx[FINDMY_BOND] == cb_data.p_le_bond_remove->p_entry->idx)
            {
                app_global_data.app_bond_idx[FINDMY_BOND] = 0xFF;
            }
#if CONFIG_REALTEK_FINDMY_SUPPORT_CUSTOMIZED_APP
            else if (app_global_data.app_bond_idx[CUST_BOND_1] == cb_data.p_le_bond_remove->p_entry->idx)
            {
                app_global_data.app_bond_idx[CUST_BOND_1] = 0xFF;
            }
            else if (app_global_data.app_bond_idx[CUST_BOND_2] == cb_data.p_le_bond_remove->p_entry->idx)
            {
                app_global_data.app_bond_idx[CUST_BOND_2] = 0xFF;
            }
#endif

            APP_PRINT_INFO2("app_findmy_msg_bond_cb: remote_bd_type %d, remote_bd %s",
                            cb_data.p_le_bond_remove->p_entry->remote_bd_type,
                            TRACE_BDADDR(cb_data.p_le_bond_remove->p_entry->remote_bd));
        }
        break;

    case BT_BOND_MSG_LE_BOND_CLEAR:
        {
            for (uint8_t i = 0; i < BLE_BOND_NUM; i++)
            {
                app_global_data.app_bond_idx[i] = 0xFF;
            }
        }
        break;

    case BT_BOND_MSG_LE_BOND_UPDATE:
        {
            if (app_findmy_is_findmy_adv())
            {
                app_global_data.app_bond_idx[FINDMY_BOND] = cb_data.p_le_bond_update->p_entry->idx;
                APP_PRINT_INFO1("app_findmy_msg_bond_cb: update_idx=%d", cb_data.p_le_bond_update->p_entry->idx);
            }
#if CONFIG_REALTEK_FINDMY_SUPPORT_CUSTOMIZED_APP
            else if (cust_app_is_cust_adv())
            {
                if ((app_global_data.app_bond_idx[CUST_BOND_1] != cb_data.p_le_bond_update->p_entry->idx) &&
                    (app_global_data.app_bond_idx[CUST_BOND_2] != cb_data.p_le_bond_update->p_entry->idx))
                {
                    if (app_global_data.app_bond_idx[CUST_BOND_1] == 0xFF)
                    {
                        app_global_data.app_bond_idx[CUST_BOND_1] = cb_data.p_le_bond_update->p_entry->idx;
                    }
                    else if (app_global_data.app_bond_idx[CUST_BOND_2] == 0xFF)
                    {
                        app_global_data.app_bond_idx[CUST_BOND_2] = cb_data.p_le_bond_update->p_entry->idx;
                    }
                    else
                    {
                        APP_PRINT_ERROR0("LE_BOND_UPDATE CUST BOND error");
                    }
                }
            }
#endif
        }
        break;

    default:
        break;
    }

    uint8_t bond_idx_buffer[FTL_SAVE_BOND_IDX_SIZE];
    memcpy(bond_idx_buffer, app_global_data.app_bond_idx, BLE_BOND_NUM);
    if (ftl_save_to_storage(bond_idx_buffer, FTL_SAVE_BOND_IDX_ADDR, FTL_SAVE_BOND_IDX_SIZE))
    {
        APP_PRINT_ERROR0("app_findmy_msg_bond_cb :FTL save failed!");
    }

    /* if findmy app bonded, set to high priority */
    if (app_global_data.app_bond_idx[FINDMY_BOND] != 0xFF)
    {
        T_LE_BOND_ENTRY *findmy_key_entry = NULL;
        findmy_key_entry = bt_le_find_key_entry_by_idx(app_global_data.app_bond_idx[FINDMY_BOND]);
        bt_le_set_high_priority_bond(findmy_key_entry);
    }

#if CONFIG_REALTEK_FINDMY_SUPPORT_CUSTOMIZED_APP
    APP_PRINT_INFO3("app_findmy_msg_bond_cb: findmy_app_idx %d, CUST_BOND_1 idx=%d, CUST_BOND_2 idx=%d",
                    app_global_data.app_bond_idx[FINDMY_BOND], app_global_data.app_bond_idx[CUST_BOND_1],
                    app_global_data.app_bond_idx[CUST_BOND_2]);
    if (app_global_data.app_bond_idx[CUST_BOND_1] != 0xFF ||
        app_global_data.app_bond_idx[CUST_BOND_2] != 0xFF)
    {
        cust_set_paired_flag(true);
    }
    else
    {
        cust_set_paired_flag(false);
    }
#endif
}

void app_findmy_crypto_init(void)
{
    fm_crypto_init();
}

void app_findmy_init(void)
{
    fmna_connection_pair_info_restore();
    app_global_data_init();
    fmna_version_init();
    fmna_gatt_platform_init();
    fmna_motion_detection_init();
    fmna_sound_platform_init();
    fmna_timer_init();
    bt_bond_register_app_cb(app_findmy_msg_bond_cb);
}

#endif
