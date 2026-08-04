/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Includes ------------------------------------------------------------------*/
#include "app_msg_handle.h"
#include <os_msg.h>
#include <os_task.h>
#include "os_mem.h"
#include "os_sync.h"
#include "trace.h"
#include "communicate_protocol.h"
#include "gap.h"
#include "gap_br.h"
#include "app_mmi.h"
#include "app_link_util.h"
#include "app_main.h"
#include "app_audio_if.h"
#include "hub_usb.h"
#include "hub_task.h"
#include "app_linkback.h"
#include "app_hfp.h"
#include "playback_playlist.h"
#include "app_playback_update_file.h"
#include "system_status_api.h"
#include "app_bt_policy_int.h"
#include "app_bt_policy_api.h"
#include "os_timer.h"
#include "app_playback_update_file.h"
#include "app_timer.h"
#include "app_cmd.h"
#include "app_audio_mode_switch.h"
#include "app_sdp.h"
#include "bt_bond.h"
#include "app_gap.h"
#include "audio_resource.h"
#include "app_ble_adv.h"
#if (F_APP_AUTO_SUPPORT == 1)
#include "rtl876x_pinmux.h"
#include "module_gpio_button.h"
#include "app_dlps.h"
#endif
#if CONFIG_FINDMY
#include "fmna_connection.h"
#endif
static bool play_flag = false;
#if F_APP_AUTO_SUPPORT
extern uint8_t show_volume_bar_timer;
#endif
#include "event_bus.h"

void set_play_flag(bool play_fg)
{
    play_flag = play_fg;
}

bool get_play_flag(void)
{
    return play_flag;
}

void watch_handle_io_message(T_IO_MSG *p_watch_msg)
{
    uint8_t msg_type = p_watch_msg->subtype;
    APP_PRINT_INFO1("watch_handle_io_message  msg_type= 0x%x", msg_type);
    switch (msg_type)
    {
    case IO_MSG_TYPE_EVENT_BUS:
        {
            event_bus_async_dispatch((T_EVENT_BUS_ASYNC_EVENT *)p_watch_msg->u.buf);
        }
        break;
    case IO_MSG_BWPS_TX_VALUE:
        {
            uint16_t length = ((uint8_t *)(p_watch_msg->u.buf))[0];
            uint8_t *p_data = (uint8_t *)p_watch_msg->u.buf + 1;
            L1_receive_data(p_data, length);
        }
        break;

    case IO_MSG_INQUIRY_START:
        app_bt_inquiry_start();
        break;

    case IO_MSG_INQUIRY_STOP:
        app_bt_inquiry_stop();
        break;

    case IO_MSG_CONNECT_BREDR_DEVICE:
        app_bt_policy_connect_bredr(p_watch_msg->u.buf);
        break;

    case IO_MSG_MMI:
        {
            uint8_t action = p_watch_msg->u.param;
            app_mmi_handle_action(action);
        }
        break;

    case IO_MSG_SET_PLAY_MODE:
        {
            uint8_t mode = p_watch_msg->u.param;
            app_audio_mode_switch(mode);
        }
        break;

    case IO_MSG_PLAY_BY_NAME:
        {
//TODO: enable on ASIC
//            if (app_db.usb_status == USB_STOPPED && app_hfp_get_call_status() == APP_HFP_CALL_IDLE &&
//                app_db.batt.allow_open.playback && app_db.transfer_status == TRANSFER_STOPPED)
            {
                T_HEAD_INFO *play_name = (T_HEAD_INFO *)p_watch_msg->u.param;
                DBG_DIRECT("LENGTH %d, addr 0x%x, offset %u", play_name->length, MUSIC_NAME_BIN_ADDR,
                           play_name->offset);
                if (play_name->length <= FILE_NAME_LEN)
                {
                    APP_PRINT_INFO1("play music name = %b", TRACE_BINARY(play_name->length,
                                                                         (uint8_t *)(MUSIC_NAME_BIN_ADDR + play_name->offset)));

                    app_audio_play_by_name((uint8_t *)(MUSIC_NAME_BIN_ADDR + play_name->offset),
                                           play_name->length);
                }
                else
                {
                    if (flash_mutex_handle)
                    {
                        os_mutex_take(flash_mutex_handle, 0xffffffff);
                        playback_sync_playlist();
                        os_mutex_give(flash_mutex_handle);
                    }
                    else
                    {
                        playback_sync_playlist();
                    }

                    APP_PRINT_ERROR0("ERROR: play_name over");
                }

            }
        }
        break;

#if F_APP_SUPPORT_USB
    case IO_MSG_PREPARE_USB_ENVIRONMENT:
        {
            app_db.usb_status = USB_STARTED;
            app_mmi_handle_action(MMI_AV_STOP);
            app_playback_trans_cancel();
            app_fs_disk_power_down_disable(APP_DISK_CHECK_USB);
            //sd card power on, sdio clk may stop after mmi stop
            app_fs_disk_power_on();
            sys_hall_auto_sleep_in_idle(false);

            os_sem_give(usb_sem_handle);
        }
        break;

    case IO_MSG_HANDLE_USB_PLUG_OUT:
        {
            sys_hall_auto_sleep_in_idle(true);
            app_playback_trans_restore();
            bool playlist_changed;
            fs_scan_file_list(&scan_hdl, &playlist_changed);
            if (playlist_changed)
            {
                if (flash_mutex_handle)
                {
                    os_mutex_take(flash_mutex_handle, 0xffffffff);
                    playback_sync_playlist();
                    os_mutex_give(flash_mutex_handle);
                }
                else
                {
                    playback_sync_playlist();
                }

            }
            app_fs_disk_power_down_enable(APP_DISK_CHECK_USB);
            app_db.usb_status = USB_STOPPED;
            os_sem_give(usb_sem_handle);
        }
        break;
#endif

    case IO_MSG_PLAYBACK_TRANS_FILE_END:
        {
            if (flash_mutex_handle)
            {
                os_mutex_take(flash_mutex_handle, 0xffffffff);
                playback_sync_playlist();
                os_mutex_give(flash_mutex_handle);
            }
            else
            {
                playback_sync_playlist();
            }
            app_fs_disk_power_down_enable(APP_DISK_CHECK_TRANS_FILE);
            //gui_update_by_event(GUI_EVENT_PLAYER, NULL, false);
        }
        break;
    case IO_MSG_CONNECT_PHONE:
        app_bt_policy_connect_phone(p_watch_msg->u.buf);
        break;

    case IO_MSG_A2DP_CONTROL_SWITCH:
        {
//TODO: sync with RTL87x3EP watch release branch
            uint8_t app_bond_phone_index = app_bt_bond_get_active_num_by_type(T_DEVICE_TYPE_PHONE);

            if (p_watch_msg->u.param == A2DP_SWITCH_OFF)
            {
                linkback_profile_disconnect_start(app_db.bond_device[app_bond_phone_index].bd_addr,
                                                  A2DP_PROFILE_MASK | AVRCP_PROFILE_MASK);
            }
            else
            {
                T_LINKBACK_RETRY_PARAM retry_param =
                {
                    .conn_retry_timeout = 0,
                    .conn_retry_cnt = 0,
                    .prof_retry_timeout = 1000,
                    .prof_retry_cnt = 3,
                    .delay_timeout = 500,
                };

                linkback_create_connection(app_db.bond_device[app_bond_phone_index].bd_addr,
                                           A2DP_PROFILE_MASK | AVRCP_PROFILE_MASK, T_DEVICE_TYPE_PHONE, retry_param);
            }
        }
        break;
    case IO_MSG_CANCEL_LINKBACK:
        {
            T_CANCEL_LINKBACK_MSG *p_cancel_linkback_msg = p_watch_msg->u.buf;
            if (p_cancel_linkback_msg->mode == CANCEL_LINKBACK_BY_ADDR)
            {
                linkback_cancel_connection_by_addr(p_cancel_linkback_msg->u.addr);
            }
            else
            {
                linkback_cancel_connection_by_device_type(p_cancel_linkback_msg->u.type);
            }
            os_mem_free(p_watch_msg->u.buf);
        }
        break;
    case IO_MSG_DISCONNECT_BREDR_DEVICE:
        app_bt_policy_disconnect_bredr(p_watch_msg->u.buf);
        break;
    case IO_MSG_FACTORY_RESET:
        {
            /*delete bt bond info*/
            app_bt_policy_enter_state(STATE_INIT);//disconnect first
            for (uint8_t i = 0; i < MAX_BOND_INFO_NUM; i++)
            {
                if (app_db.bond_device[i].exist_addr_flag)
                {
                    app_bt_bond_del_bond_device(app_db.bond_device[i].bd_addr);
                    bt_bond_delete(app_db.bond_device[i].bd_addr);
                }
            }
            app_bt_bond_save_device_info_to_ftl(app_db.bond_device);
#if CONFIG_FINDMY
            fmna_connection_set_is_fmna_paired(false);
#endif
            /*delete user data*/
            if (app_audio_get_play_status() != APP_AUDIO_STATE_STOP)//stop current action
            {
                app_audio_stop();
            }
            if (fs_unlink_all_files(&scan_hdl) != 0)
            {
                APP_PRINT_ERROR0("fs_unlink_all_files fail");
            }
        }
        break;
#if F_APP_AUTO_SUPPORT
    case IO_MSG_GPIO_UART_WAKE_UP:
        {
            app_dlps_disable(APP_DLPS_ENTER_CHECK_BUTTON);
        }
        break;
#endif
    case IO_MSG_START_BLE_ADV:
        {
            app_ble_common_adv_start(p_watch_msg->u.param);
        }
        break;
    case IO_MSG_STOP_BLE_ADV:
        {
            app_ble_common_adv_stop(p_watch_msg->u.param);
        }
        break;

    case IO_MSG_BT_TOGGLE:
        app_bt_policy_set_enabled(p_watch_msg->u.param != 0);
        break;

    case IO_MSG_REMOVE_BOND_DEVICE:
        app_bt_bond_remove_by_index((uint8_t)p_watch_msg->u.param);
        break;

    default:
        break;
    }
}
