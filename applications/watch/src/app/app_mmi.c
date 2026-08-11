/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "trace.h"
#include "test_mode.h"
#include "bt_avrcp.h"
#include "bt_bond.h"
#include "eq.h"
#include "eq_utils.h"
#include "audio_passthrough.h"
#include "audio.h"
#include "audio_volume.h"
#include "vad.h"
#include "remote.h"
#include "bt_bond.h"
#include "app_mmi.h"
#include "app_main.h"
#include "app_cfg.h"
#include "sysm.h"
#include "app_transfer.h"
#include "app_hfp.h"
#include "audio_hfp.h"
#include "app_link_util.h"
#include "rtl876x_pinmux.h"
#include "app_audio_policy.h"
#include "audio_playback.h"
#include "audio_record.h"
#include "app_audio_if.h"
#include "app_playback_update_file.h"
#include "app_cmd.h"

#if F_APP_HID_SUPPORT
#include "bt_hid.h"
#endif

void app_mmi_handle_action(uint8_t action)
{
    APP_PRINT_TRACE1("app_mmi_handle_action: action 0x%02x", action);

    uint8_t app_idx = app_hfp_get_active_hf_index();

    switch (action)
    {
    case MMI_HF_ANSWER_CALL:
        {
            if (app_idx < MAX_BR_LINK_NUM && app_db.br_link[app_idx].used)
            {
                app_hfp_stop_ring();
                bt_hfp_call_answer_req(app_db.br_link[app_idx].bd_addr);
            }
        }
        break;

    case MMI_HF_REJECT_CALL:
        {
            if (app_idx < MAX_BR_LINK_NUM && app_db.br_link[app_idx].used)
            {
                app_hfp_stop_ring();
                bt_hfp_call_terminate_req(app_db.br_link[app_idx].bd_addr);
            }
        }
        break;

    case MMI_HF_END_ACTIVE_CALL:
        {
            if (app_idx < MAX_BR_LINK_NUM && app_db.br_link[app_idx].used)
            {
                bt_hfp_call_terminate_req(app_db.br_link[app_idx].bd_addr);
            }
        }
        break;

    case MMI_HF_END_OUTGOING_CALL:
        {
            if (app_idx < MAX_BR_LINK_NUM && app_db.br_link[app_idx].used)
            {
                bt_hfp_call_terminate_req(app_db.br_link[app_idx].bd_addr);
            }
        }
        break;
    case MMI_HF_OUTGOING_CALL:
        {
            // Validate app_idx before use
            if (app_idx >= MAX_BR_LINK_NUM || !app_db.br_link[app_idx].used)
            {
                APP_PRINT_WARN0("MMI_HF_OUTGO_CALL: invalid app_idx");
                break;
            }

            // Get dial number from app_hfp (set by GUI before calling this MMI)
            const char *dial_num = app_hfp_get_current_call_number();
            if (dial_num != NULL && strlen(dial_num) > 0)
            {
                bt_hfp_dial_with_number_req(app_db.br_link[app_idx].bd_addr, dial_num);
            }
        }
        break;

#if (F_APP_AUTO_SUPPORT == 1)
    case MMI_HF_LAST_NUMBER_REDIAL:
        {
            uint8_t app_idx;
            app_idx = app_hfp_get_active_hf_index();

            if (app_db.br_link[app_idx].app_hf_state == APP_HF_STATE_CONNECTED)
            {
                if (app_hfp_get_call_status() == APP_HFP_CALL_IDLE)
                {
                    APP_PRINT_INFO1("link_bd_addr = %s", TRACE_BDADDR(app_db.br_link[app_idx].bd_addr));
                    bt_hfp_dial_last_number_req(app_db.br_link[app_idx].bd_addr);
                }
            }
        }
        break;
#endif

    case MMI_HF_TRANSFER_CALL:
        {
            if (app_db.br_link[app_idx].sco_handle)// to phone
            {
                bt_hfp_audio_disconnect_req(app_db.br_link[app_idx].bd_addr);
            }
            else //to bud
            {
                bt_hfp_audio_connect_req(app_db.br_link[app_idx].bd_addr);
            }
        }
        break;

    case MMI_HF_INITIATE_VOICE_DIAL:
        {
            if (app_db.br_link[app_idx].connected_profile & (HFP_PROFILE_MASK | HSP_PROFILE_MASK))
            {
                bt_hfp_voice_recognition_enable_req(app_db.br_link[app_idx].bd_addr);
            }
            else
            {
                //may connect phone here
            }
        }
        break;

    case MMI_HF_CANCEL_VOICE_DIAL:
        {
            bt_hfp_voice_recognition_disable_req(app_db.br_link[app_idx].bd_addr);
        }
        break;

    case MMI_HF_RELEASE_HELD_OR_WAITING_CALL:
        {
            bt_hfp_release_held_or_waiting_call_req(app_db.br_link[app_idx].bd_addr);
        }
        break;

    case MMI_HF_RELEASE_ACTIVE_CALL_ACCEPT_HELD_OR_WAITING_CALL:
        {
            bt_hfp_release_active_call_accept_held_or_waiting_call_req(app_db.br_link[app_idx].bd_addr);
        }
        break;

    case MMI_HF_SWITCH_TO_SECOND_CALL:
        {
            bt_hfp_hold_active_call_accept_held_or_waiting_call_req(app_db.br_link[app_idx].bd_addr);
        }
        break;

    case MMI_HF_JOIN_TWO_CALLS:
        {
            bt_hfp_join_two_calls_req(app_db.br_link[app_idx].bd_addr);
        }
        break;

    case MMI_DEV_SPK_VOL_UP:
        {
            app_volume_up();
        }
        break;

    case MMI_DEV_SPK_VOL_DOWN:
        {
            app_volume_down();
        }
        break;

    case MMI_DEV_MIC_MUTE:
        {
            app_mmi_mic_mute_set();
        }
        break;

    case MMI_DEV_MIC_UNMUTE:
        {
            app_mmi_mic_unmute_set();
        }
        break;

    case MMI_DEV_MIC_MUTE_UNMUTE:
        {
            if (audio_hfp_check_mic_mute_enable() == true)
            {
                audio_hfp_mute_ctrl();
            }
        }
        break;

    case MMI_AUDIO_EQ_SWITCH:
        {
            uint8_t eq_num = eq_utils_num_get(SPK_SW_EQ, NORMAL_MODE);

            if (eq_num != 0)
            {
                app_cfg_nv.eq_idx++;

                if (app_cfg_nv.eq_idx >= eq_num)
                {
                    app_cfg_nv.eq_idx = 0;
                }

                app_eq_index_set(SPK_SW_EQ, NORMAL_MODE, app_cfg_nv.eq_idx);
            }
            else
            {
                APP_PRINT_ERROR0("app_mmi_handle_action: eq need to enable");
            }
        }
        break;

    case MMI_AV_PLAY_PAUSE:
        {
            APP_PRINT_INFO4("call status = %d, allow playback = %d, playstatus = %d, transfer_status = %d",
                            app_hfp_get_call_status(),
                            \
                            app_db.batt.allow_open.playback, app_audio_get_play_status(), app_db.transfer_status);
            if ((app_hfp_get_call_status() == APP_HFP_CALL_IDLE && app_db.batt.allow_open.playback &&
                 app_db.transfer_status == TRANSFER_STOPPED) ||
                app_audio_get_play_status() == APP_AUDIO_STATE_PLAY)
            {
                if (app_audio_get_play_status() == APP_AUDIO_STATE_PLAY)
                {
#if (F_APP_AUTO_SUPPORT == 1)
                    DBG_DIRECT("Play pause");
#endif
                    app_audio_pause();
                }
                else if (app_audio_get_play_status() == APP_AUDIO_STATE_PAUSE)
                {
#if (F_APP_AUTO_SUPPORT == 1)
                    DBG_DIRECT("Play start");
#endif
                    app_audio_start();
                }
                else if (app_audio_get_play_status() == APP_AUDIO_STATE_STOP)
                {
                    app_audio_next();
                }
            }
            else
            {
                APP_PRINT_ERROR0("call active or low battery level or song is transferring,, do not play music!");
            }
        }
        break;

    case MMI_AV_STOP:
        {
            if (app_audio_get_play_status() != APP_AUDIO_STATE_STOP)
            {
                app_audio_stop();
            }
        }
        break;

    case MMI_AV_FWD:
        {
            APP_PRINT_INFO3("mmi fwd call status = %d, allow playback = %d, playstatus = %d",
                            app_hfp_get_call_status(),
                            \
                            app_db.batt.allow_open.playback, app_audio_get_play_status());
            if (app_hfp_get_call_status() == APP_HFP_CALL_IDLE && app_db.batt.allow_open.playback)
            {
                app_audio_next();
            }
            else
            {
                APP_PRINT_ERROR0("call active or low battery level, do not play music!");
            }
        }
        break;

    case MMI_AV_BWD:
        {
            APP_PRINT_INFO3("mmi bwd call status = %d, allow playback = %d, playstatus = %d",
                            app_hfp_get_call_status(),
                            \
                            app_db.batt.allow_open.playback, app_audio_get_play_status());
            if ((app_hfp_get_call_status() == APP_HFP_CALL_IDLE && app_db.batt.allow_open.playback) ||
                app_audio_get_play_status() == APP_AUDIO_STATE_PLAY)
            {
                app_audio_prev();
            }
            else
            {
                APP_PRINT_ERROR0("call active or low battery level, do not play music!");
            }
        }
        break;

    case MMI_SD_PLAYBACK_SWITCH:
        {
        }
        break;

    case MMI_SD_A2DP_SRC_SWITCH:
        {
        }
        break;

    case MMI_SD_A2DP_SNK_SWITCH:
        {
        }
        break;

    case MMI_RECORD_START:
        {
            audio_record_init_recorder(AUDIO_RECORD_SAVE_FS);
            audio_record_start_recording();
        }
        break;

    case MMI_RECORD_STOP:
        {
            audio_record_stop_recording();
        }
        break;

    case MMI_RECORD_PLAY_START:
        {
            audio_record_init_player(AUDIO_RECORD_SAVE_FS, NULL);
        }
        break;

    case MMI_RECORD_PLAY_STOP:
        {
            audio_record_stop_playing();
        }
        break;

#if F_APP_HID_SUPPORT
    case MMI_TAKE_PICTURE:
        {
            uint8_t keyboard_vol_up[5] = {0x02, 0, 0, 0x80, 0};
            uint8_t keyboard_release[5] = {0x02, 0, 0, 0, 0};

            bt_hid_interrupt_data_send(app_db.br_link[app_idx].bd_addr, BT_HID_REPORT_TYPE_INPUT,
                                       keyboard_vol_up, sizeof(keyboard_vol_up));
            bt_hid_interrupt_data_send(app_db.br_link[app_idx].bd_addr, BT_HID_REPORT_TYPE_INPUT,
                                       keyboard_release, sizeof(keyboard_release));
        }
        break;
#endif

    case MMI_RECORD_RECORD_AND_PLAY_START:
        {
            audio_record_init_recorder(AUDIO_RECORD_LOOP_BACK);
            audio_record_start_recording();
            audio_record_init_player(AUDIO_RECORD_LOOP_BACK, NULL);
        }
        break;

    case MMI_RECORD_RECORD_AND_PLAY_STOP:
        {
            audio_record_stop_playing();
            audio_record_stop_recording();
        }
        break;

    default:
        break;
    }
}
