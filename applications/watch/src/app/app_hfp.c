/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "trace.h"
#include "bt_types.h"
#include "app_timer.h"
#include "btm.h"
#include "remote.h"
#include "ringtone.h"
#include "bt_bond.h"
#include "app_cfg.h"
#include "app_main.h"
#include "app_report.h"
#include "app_link_util.h"
#include "app_sdp.h"
#include "app_hfp.h"
#include "app_audio_policy.h"
#include "audio_volume.h"
#include "app_bond.h"
#include "os_timer.h"
#include "app_linkback.h"
#include "audio_track.h"
#include "app_cmd.h"
#include "app_pbap.h"
#include "app_bt_policy_api.h"
#include "audio_hfp.h"
#include "event_bus.h"

#if F_APP_HID_SUPPORT
#include "app_hid.h"
#endif
#if CONFIG_WECHAT_CALL_ON_PHONE_OUTPUT_ONLY
#include "app_mmi.h"
#endif

typedef enum
{
    APP_HFP_TIMER_RING             = 0x00,
#if CONFIG_WECHAT_CALL_ON_PHONE_OUTPUT_ONLY
    APP_HFP_TIMER_WECHAT           = 0x01,
#endif
} T_APP_HFP_TIMER;

#if CONFIG_WECHAT_CALL_ON_PHONE_OUTPUT_ONLY
static bool is_wechat_call = false;
static uint8_t sco_conn_ind_cnt = 0;
static uint8_t timer_handle_hfp_wechat = 0;
#endif
static uint8_t hfp_timer_id = 0;
static uint8_t timer_handle_hfp_ring = 0;
static bool hf_ring_active = false;
static uint8_t active_hf_index = 0;
uint8_t  call_num_hfp[21];
bool always_play_hf_incoming_tone = false;

#if CONFIG_WECHAT_CALL_ON_PHONE_OUTPUT_ONLY
void app_audio_set_sco_conn_cnt(uint8_t sco_conn_cnt)
{
    sco_conn_ind_cnt = sco_conn_cnt;
}
bool app_hfp_get_wechat_status(void)
{
    return is_wechat_call;
}

void app_hfp_wechat_timer_start(uint32_t time)
{
    app_stop_timer(&timer_handle_hfp_wechat);
    app_start_timer(&timer_handle_hfp_wechat, "hfp_wechat",
                    hfp_timer_id, APP_HFP_TIMER_WECHAT, 0, false, time);
}

void app_hfp_wechat_transfer_handle(uint8_t *bd_addr)
{
    T_APP_BR_LINK *p_link;

    p_link = app_find_br_link(bd_addr);
    if (p_link)
    {
        APP_PRINT_INFO2("app_hfp_wechat_transfer_handle: call_status = %x, is_wechat_call = %d",
                        p_link->call_status, is_wechat_call);

        if (p_link->call_status != APP_HFP_CALL_IDLE)
        {
            is_wechat_call = false;
        }

        if (p_link->call_status == APP_HFP_CALL_IDLE)
        {
            if (is_wechat_call)
            {
                app_audio_set_sco_conn_cnt(1);
                app_hfp_wechat_timer_start(2000);
            }
            else
            {
                if (!timer_handle_hfp_wechat)
                {
                    app_audio_set_sco_conn_cnt(0);
                }
            }
            is_wechat_call = false;
            audio_track_volume_out_unmute(p_link->sco_track_handle);
        }
        else if (p_link->call_status == APP_HFP_CALL_OUTGOING)
        {
            app_hfp_wechat_timer_start(2200);
        }
        else if (p_link->call_status == APP_HFP_CALL_ACTIVE)
        {
            if (timer_handle_hfp_wechat)
            {
                is_wechat_call = true;
                app_hfp_wechat_timer_start(500);
                audio_track_volume_out_mute(p_link->sco_track_handle);
            }
        }
    }
}
#endif

void app_hfp_timeout_cb(uint8_t timer_evt, uint16_t param)
{
    APP_PRINT_TRACE2("app_hfp_timeout_cb: timer_id %d, param %d", timer_evt, param);

    switch (timer_evt)
    {
    case APP_HFP_TIMER_RING:
        {
            if (timer_handle_hfp_ring != 0)
            {
                app_stop_timer(&timer_handle_hfp_ring);
                app_hfp_ring_alert(param);
            }
        }
        break;
#if CONFIG_WECHAT_CALL_ON_PHONE_OUTPUT_ONLY
    case APP_HFP_TIMER_WECHAT:
        {
            app_stop_timer(&timer_handle_hfp_wechat);
            if (is_wechat_call)
            {
                uint8_t app_idx = app_hfp_get_active_hf_index();
                if (app_db.br_link[app_idx].sco_handle)// to phone
                {
                    app_mmi_handle_action(MMI_HF_TRANSFER_CALL);
                }
            }
            else
            {
                app_audio_set_sco_conn_cnt(0);
            }
        }
        break;
#endif
    default:
        break;
    }
}

void app_hfp_stop_ring(void)
{
    if (hf_ring_active)
    {
        app_stop_timer(&timer_handle_hfp_ring);
        hf_ring_active = false;
        voice_prompt_flush(false);
        voice_prompt_stop();
    }
}

void app_hfp_ring_alert(uint8_t link_id)
{
    hf_ring_active = true;
    app_audio_tone_play(0x8d, false, false);//incoming call vp index
    app_start_timer(&timer_handle_hfp_ring, "hfp_ring",
                    hfp_timer_id, APP_HFP_TIMER_RING, link_id, true,
                    3 * 1000);
}


T_APP_HFP_CALL_STATUS app_hfp_get_call_status(void)
{
    return app_db.br_link[active_hf_index].call_status;
}

uint8_t app_hfp_get_active_hf_index(void)
{
    return active_hf_index;
}

bool app_hfp_set_active_hf_index(uint8_t *bd_addr)
{
    T_APP_BR_LINK *p_link;

    p_link = app_find_br_link(bd_addr);
    if (p_link)
    {
        active_hf_index = p_link->id;
        return bt_sco_link_switch(bd_addr);
    }
    return false;
}

void app_hfp_call_status_update(uint8_t *bd_addr, T_BT_HFP_CALL_STATUS bt_hfp_status)
{
    T_APP_BR_LINK *p_link;
    p_link = app_find_br_link(bd_addr);
    if (p_link != NULL)
    {
        T_APP_HFP_CALL_STATUS call_status_old = app_hfp_get_call_status();
        switch (bt_hfp_status)
        {
        case BT_HFP_CALL_IDLE:
            {
                p_link->call_status = APP_HFP_CALL_IDLE;
                // Clear call number when call ends
                call_num_hfp[0] = 0;
                app_pbap_clear_caller_name();
                // Notify phone UI that call has ended
                event_bus_publish(EVENT_BUS_TOPIC_HFP_ENDED, NULL, 0);
            }
            break;

        case BT_HFP_CALL_INCOMING:
            {
                p_link->call_status = APP_HFP_CALL_INCOMING;
                APP_PRINT_INFO0("app_hfp: incoming call detected");

                // Clear previous caller data for new incoming call
                call_num_hfp[0] = 0;
                app_pbap_clear_caller_name();

                // Notify phone UI about incoming call with phone number
                const char *incoming_num = app_hfp_get_current_call_number();
                if (incoming_num != NULL)
                {
                    event_bus_publish(EVENT_BUS_TOPIC_HFP_INCOMING, (void *)incoming_num, strlen(incoming_num) + 1);
                }
                else
                {
                    event_bus_publish(EVENT_BUS_TOPIC_HFP_INCOMING, NULL, 0);
                }
            }
            break;

        case BT_HFP_CALL_OUTGOING:
            {
                p_link->call_status = APP_HFP_CALL_OUTGOING;

                // Clear previous caller data for new outgoing call
                call_num_hfp[0] = 0;
                app_pbap_clear_caller_name();

                // Clear call_id_type flags to prevent old PBAP data from being used
                p_link->call_id_type_check = false;
                p_link->call_id_type_num = false;
            }
            break;

        case BT_HFP_CALL_ACTIVE:
            {
                p_link->call_status = APP_HFP_CALL_ACTIVE;
                APP_PRINT_INFO0("app_hfp: BT_HFP_CALL_ACTIVE received");
                // Notify phone UI about call answered with phone number
                const char *active_num = app_hfp_get_current_call_number();
                if (active_num != NULL)
                {
                    event_bus_publish(EVENT_BUS_TOPIC_HFP_ANSWERED, (void *)active_num, strlen(active_num) + 1);
                }
                else
                {
                    event_bus_publish(EVENT_BUS_TOPIC_HFP_ANSWERED, NULL, 0);
                }
            }
            break;

        case BT_HFP_CALL_HELD:
            {
                //p_link->call_status = APP_HFP_CALL_HELD;
            }
            break;

        case BT_HFP_CALL_ACTIVE_WITH_CALL_WAITING:
            {
                p_link->call_status = APP_HFP_CALL_ACTIVE_WITH_CALL_WAITING;
            }
            break;

        case BT_HFP_CALL_ACTIVE_WITH_CALL_HELD:
            {
                p_link->call_status = APP_HFP_CALL_ACTIVE_WITH_CALL_HELD;
            }
            break;

        default:
            break;
        }

        APP_PRINT_INFO2("app_hfp_update_call_status: call_status_old 0x%04x, call_status_new 0x%04x",
                        call_status_old, p_link->call_status);
    }
}

void app_hfp_call_number_handle(uint8_t *bd_addr, char *number)
{
    T_APP_BR_LINK *p_link;
    p_link = app_find_br_link(bd_addr);
    if (p_link != NULL)
    {
        APP_PRINT_INFO3("app_hfp_call_number_handle: call_id_type_check=%d, call_id_type_num=%d, number=%s",
                        p_link->call_id_type_check, p_link->call_id_type_num, TRACE_STRING((const uint8_t *)number));

        if (p_link->call_id_type_check == true)
        {
            // Clear entire array first to remove old data
            memset(call_num_hfp, 0, sizeof(call_num_hfp));

            uint8_t number_len = strlen(number);
            if (number_len > 20)
            {
                number_len = 20;
            }
            call_num_hfp[0] = number_len;
            memcpy(call_num_hfp + 1, number, number_len);

            if (p_link->connected_profile & PBAP_PROFILE_MASK)
            {
                APP_PRINT_INFO0("app_hfp_call_number_handle: PBAP connected, pulling vcard");
                if (bt_pbap_vcard_listing_by_number_pull(p_link->bd_addr, number) == false)
                {
                    APP_PRINT_INFO0("app_hfp_call_number_handle: vcard pull failed, setting call_id_type_check=false");
                    p_link->call_id_type_check = false;
                    p_link->call_id_type_num = true;
                }
            }
            else
            {
                APP_PRINT_INFO0("app_hfp_call_number_handle: PBAP not connected");
                p_link->call_id_type_check = false;
                p_link->call_id_type_num = true;
            }
        }

        if (p_link->call_id_type_check == false)
        {
            if (p_link->call_id_type_num == true)
            {
                // set_dial_num(number, strlen(number));
                // Note: phone number is already published via hfp/incoming event
                p_link->call_id_type_num = false;
            }
            else
            {
                if (app_hfp_get_call_status() == APP_HFP_CALL_OUTGOING)
                {
                    if (call_num_hfp[0] == 0 && number != NULL && strlen(number) > 0)
                    {
                        uint8_t number_len = strlen(number);
                        if (number_len > 20)
                        {
                            number_len = 20;
                        }
                        call_num_hfp[0] = number_len;
                        memcpy(call_num_hfp + 1, number, number_len);
                        APP_PRINT_INFO1("app_hfp_call_number_handle: saved number to call_num_hfp, number=%s",
                                        TRACE_STRING((const uint8_t *)number));
                    }
                    // Note: phone number is already published via hfp/incoming event
                }
            }
        }
    }
}

static void app_hfp_bt_cback(T_BT_EVENT event_type, void *event_buf, uint16_t buf_len)
{
    T_BT_EVENT_PARAM *param = event_buf;
    bool handle = true;

    switch (event_type)
    {
    case BT_EVENT_HFP_CONN_IND:
        {
            T_APP_BR_LINK *p_link = app_find_br_link(param->hfp_conn_ind.bd_addr);
            if (p_link == NULL)
            {
                APP_PRINT_ERROR0("app_hfp_bt_cback: no acl link found");
                return;
            }

            bool is_hfp_connected = false;
            for (uint8_t i = 0; i < MAX_BR_LINK_NUM; ++i)
            {
                if (app_db.br_link[i].used &&
                    (app_db.br_link[i].connected_profile & HFP_PROFILE_MASK))
                {
                    is_hfp_connected = true;
                    break;
                }
            }
            if (is_hfp_connected)
            {
                APP_PRINT_INFO0("HFP is already connected. Connection is rejected.");
                bt_hfp_connect_cfm(p_link->bd_addr, false);
                break; // Jump over handling.
            }

            uint8_t temp_index = app_bt_bond_get_index_by_addr(param->hfp_conn_ind.bd_addr);
            if (temp_index < MAX_BOND_INFO_NUM)
            {
                if (app_db.bond_device[temp_index].device_type == T_DEVICE_TYPE_EARPHONE)
                {
                    APP_PRINT_ERROR0("app_hfp_bt_cback: special ear device cfm false");
                    bt_hfp_connect_cfm(p_link->bd_addr, false);

                    T_LINKBACK_RETRY_PARAM retry_param =
                    {
                        .conn_retry_timeout = 0,
                        .conn_retry_cnt = 0,
                        .prof_retry_timeout = 1000,
                        .prof_retry_cnt = 3,
                        .delay_timeout = 1000
                    };
                    linkback_create_connection(param->hfp_conn_ind.bd_addr, A2DP_PROFILE_MASK | AVRCP_PROFILE_MASK,
                                               T_DEVICE_TYPE_EARPHONE, retry_param);
                }
                else
                {
                    bt_hfp_connect_cfm(p_link->bd_addr, true);
                }
            }
        }
        break;

    case BT_EVENT_HFP_CONN_CMPL:
        {
            T_APP_BR_LINK *p_link;

            p_link = app_find_br_link(param->hfp_conn_cmpl.bd_addr);
            if (p_link != NULL)
            {
                uint8_t pair_idx;

                p_link->call_id_type_check = true;
                p_link->call_id_type_num = false;

                if (bt_bond_index_get(p_link->bd_addr, &pair_idx) == false)
                {
                    APP_PRINT_ERROR0("hfp conn cmpl: get pair idx fail");
                    break;
                }
                audio_hfp_speaker_gain_level_report(p_link->bd_addr, app_cfg_nv.voice_gain_level[pair_idx]);
                bt_hfp_microphone_gain_level_report(p_link->bd_addr, 0x0a);
                bt_hfp_batt_level_report(p_link->bd_addr, app_db.local_batt_level);
                bt_hfp_nrec_disable(p_link->bd_addr);

                audio_volume_out_set(AUDIO_STREAM_TYPE_VOICE, app_cfg_nv.voice_gain_level[pair_idx]);
                app_hfp_set_active_hf_index(p_link->bd_addr);
                app_bond_set_priority(p_link->bd_addr);

                p_link->app_hf_state = APP_HF_STATE_CONNECTED;

                if (app_db.audio_play_mode == MODE_APP_A2DP_SNK && app_db.a2dp_control_switch == true)
                {
                    T_LINKBACK_RETRY_PARAM retry_param =
                    {
                        .conn_retry_timeout = 0,
                        .conn_retry_cnt = 0,
                        .prof_retry_timeout = 1000,
                        .prof_retry_cnt = 3,
                        .delay_timeout = 0
                    };
                    uint32_t plan_profs = (A2DP_PROFILE_MASK | AVRCP_PROFILE_MASK | PBAP_PROFILE_MASK);
                    linkback_create_connection(param->hfp_conn_cmpl.bd_addr, plan_profs, T_DEVICE_TYPE_PHONE,
                                               retry_param);
                }
                else if ((p_link->connected_profile & PBAP_PROFILE_MASK) == 0)
                {
                    T_LINKBACK_RETRY_PARAM retry_param =
                    {
                        .conn_retry_timeout = 0,
                        .conn_retry_cnt = 0,
                        .prof_retry_timeout = 3000,
                        .prof_retry_cnt = 3,
                        .delay_timeout = 1000
                    };
                    linkback_create_connection(param->hfp_conn_cmpl.bd_addr, PBAP_PROFILE_MASK, T_DEVICE_TYPE_PHONE,
                                               retry_param);
                }

#if F_APP_HID_SUPPORT
                T_LINKBACK_RETRY_PARAM retry_param =
                {
                    .conn_retry_timeout = 0,
                    .conn_retry_cnt = 0,
                    .prof_retry_timeout = 0,
                    .prof_retry_cnt = 0,
                    .delay_timeout = 5000
                };
                linkback_create_connection(p_link->bd_addr, HID_PROFILE_MASK, T_DEVICE_TYPE_PHONE, retry_param);
#endif
            }
        }
        break;

    case BT_EVENT_HFP_CALL_STATUS:
        {
            T_APP_BR_LINK *p_link;
            p_link = app_find_br_link(param->hfp_call_status.bd_addr);
            if (p_link != NULL)
            {
                uint8_t call_status_old = app_hfp_get_call_status();
                app_hfp_call_status_update(param->hfp_call_status.bd_addr, \
                                           param->hfp_call_status.curr_status);
                if (p_link->call_status != APP_HFP_CALL_IDLE)
                {
                    if ((p_link->connected_profile & PBAP_PROFILE_MASK) == 0)
                    {
                        T_LINKBACK_RETRY_PARAM retry_param =
                        {
                            .conn_retry_timeout = 0,
                            .conn_retry_cnt = 0,
                            .prof_retry_timeout = 3000,
                            .prof_retry_cnt = 3,
                            .delay_timeout = 0
                        };
                        linkback_create_connection(p_link->bd_addr, PBAP_PROFILE_MASK, T_DEVICE_TYPE_PHONE,
                                                   retry_param);
                    }
                }

                if (p_link->call_status == APP_HFP_CALL_OUTGOING
                    || p_link->call_status == APP_HFP_CALL_INCOMING)
                {
                    bt_hfp_current_call_list_req(p_link->bd_addr);
                }

                if (p_link->call_status == APP_HFP_CALL_IDLE)
                {
                    p_link->call_id_type_check = true;
                    p_link->call_id_type_num = false;
                    //gui_update_by_event(GUI_EVENT_CALL_END, NULL, true);
                }
                else if (p_link->call_status == APP_HFP_CALL_ACTIVE || p_link->call_status == APP_HFP_CALL_OUTGOING)
                {
                    if (p_link->is_inband_ring && always_play_hf_incoming_tone)
                    {
                        audio_track_volume_out_unmute(p_link->sco_track_handle);
                    }
                    if (p_link->call_status == APP_HFP_CALL_ACTIVE)
                    {
                        //gui_update_by_event(GUI_EVENT_CALL_ACTIVE, NULL, true);
                    }
                }

                if (call_status_old != app_hfp_get_call_status())
                {
                    if ((call_status_old == APP_HFP_CALL_INCOMING) ||
                        (call_status_old == APP_HFP_CALL_ACTIVE_WITH_CALL_WAITING) ||
                        (call_status_old == APP_HFP_MULTILINK_CALL_ACTIVE_WITH_CALL_WAIT))
                    {
                        app_hfp_stop_ring();
                    }
                }
#if CONFIG_WECHAT_CALL_ON_PHONE_OUTPUT_ONLY
                app_hfp_wechat_transfer_handle(p_link->bd_addr);
#endif
                audio_coexist_handle(AUDIO_COEXIST_CALL_STATUS_UPDATE);
            }
        }
        break;

    case BT_EVENT_HFP_CALLER_ID_IND:
    case BT_EVENT_HFP_CALL_WAITING_IND:
    case BT_EVENT_HFP_CURRENT_CALL_LIST_IND:
        {
            uint8_t *bd_addr = NULL;
            char *number = NULL;

            if (event_type == BT_EVENT_HFP_CALLER_ID_IND)
            {
                bd_addr = param->hfp_caller_id_ind.bd_addr;
                number = (char *)param->hfp_caller_id_ind.number;
            }
            else if (event_type == BT_EVENT_HFP_CALL_WAITING_IND)
            {
                bd_addr = param->hfp_call_waiting_ind.bd_addr;
                number = (char *)param->hfp_call_waiting_ind.number;
            }
            else if (event_type == BT_EVENT_HFP_CURRENT_CALL_LIST_IND)
            {
                bd_addr = param->hfp_current_call_list_ind.bd_addr;
                number = (char *)param->hfp_current_call_list_ind.number;
            }

            app_hfp_call_number_handle(bd_addr, number);
        }
        break;

    case BT_EVENT_HFP_RING_ALERT:
        {
            T_APP_BR_LINK *p_link;

            p_link = app_find_br_link(param->hfp_ring_alert.bd_addr);
            if (p_link != NULL)
            {
                p_link->is_inband_ring = param->hfp_ring_alert.is_inband;

                if (p_link->is_inband_ring == false ||
                    p_link->id != active_hf_index) /* TODO check active sco link */
                {
                    if (app_db.audio_pipe_create == false && hf_ring_active == false &&
                        p_link->call_status == APP_HFP_CALL_INCOMING)
                    {
//ring alert by ringtone
//TODO: enable on ASIC
//                        app_hfp_ring_alert(p_link->id);
                    }
                }
            }
        }
        break;

    case BT_EVENT_HFP_SPK_VOLUME_CHANGED:
        {
            if (app_cfg_const.hfp_brsf_capability & BT_HFP_HF_LOCAL_REMOTE_VOLUME_CONTROL)
            {
                audio_hfp_set_volume(param->hfp_spk_volume_changed.volume);
            }
        }
        break;

    case BT_EVENT_HFP_MIC_VOLUME_CHANGED:
        {

        }
        break;

    case BT_EVENT_HFP_DISCONN_CMPL:
        {
            T_APP_BR_LINK *p_link;

            p_link = app_find_br_link(param->hfp_disconn_cmpl.bd_addr);
            if (p_link != NULL)
            {
                if (param->hfp_disconn_cmpl.cause == (HCI_ERR | HCI_ERR_CONN_ROLESWAP))
                {
                    //do nothing
                }
                else
                {
                    p_link->call_status = APP_HFP_CALL_IDLE;
                    p_link->app_hf_state = APP_HF_STATE_STANDBY;
                }
            }
        }
        break;

    case BT_EVENT_HFP_VOICE_RECOGNITION_ACTIVATION:
        {
            T_APP_BR_LINK *p_link;

            if (param->hfp_voice_recognition_activation.result == BT_HFP_CMD_OK)
            {
                p_link = app_find_br_link(param->hfp_voice_recognition_activation.bd_addr);
                if (p_link != NULL)
                {
                    if (p_link->call_status == APP_HFP_CALL_IDLE)
                    {
                        p_link->call_status = APP_HFP_VOICE_ACTIVATION_ONGOING;
                    }
                }
            }
        }
        break;

    case BT_EVENT_HFP_VOICE_RECOGNITION_DEACTIVATION:
        {
            T_APP_BR_LINK *p_link;

            if (param->hfp_voice_recognition_deactivation.result == BT_HFP_CMD_OK)
            {
                p_link = app_find_br_link(param->hfp_voice_recognition_deactivation.bd_addr);
                if (p_link != NULL)
                {
                    if (p_link->call_status == APP_HFP_VOICE_ACTIVATION_ONGOING)
                    {
                        p_link->call_status = APP_HFP_CALL_IDLE;
                    }
                }
            }
        }
        break;

    case BT_EVENT_SCO_CONN_IND:
        {
            bool need_cfm = true;
            T_APP_BR_LINK *p_link;
            p_link = app_find_br_link(param->sco_conn_ind.bd_addr);
            if (p_link != NULL)
            {
#if CONFIG_WECHAT_CALL_ON_PHONE_OUTPUT_ONLY
                if (app_hfp_get_wechat_status() || (sco_conn_ind_cnt != 0))
                {
                    sco_conn_ind_cnt++;
                    if (sco_conn_ind_cnt > 1)
                    {
                        need_cfm = false;
                    }
                }
#endif
                audio_coexist_handle(AUDIO_COEXIST_SCO_CONN_IND);
                bt_hfp_audio_connect_cfm(param->sco_conn_ind.bd_addr, need_cfm);
            }
        }
        break;

    case BT_EVENT_SCO_CONN_CMPL:
        {
            T_APP_BR_LINK *p_link;
            p_link = app_find_br_link(param->sco_conn_cmpl.bd_addr);
            if (p_link != NULL)
            {
                bt_active_link_set(param->sco_conn_cmpl.bd_addr);
                audio_sco_conn_cmpl_handle(&param->sco_conn_cmpl);
                if ((p_link->call_status == APP_HFP_CALL_INCOMING) && always_play_hf_incoming_tone)
                {
                    audio_track_volume_out_mute(p_link->sco_track_handle);
                    app_hfp_ring_alert(p_link->id);
                }
#if CONFIG_WECHAT_CALL_ON_PHONE_OUTPUT_ONLY
                if (app_hfp_get_wechat_status())
                {
                    audio_track_volume_out_mute(p_link->sco_track_handle);
                }
#endif
            }
        }
        break;

    case BT_EVENT_SCO_DISCONNECTED:
        {
#if CONFIG_WECHAT_CALL_ON_PHONE_OUTPUT_ONLY
            T_APP_BR_LINK *p_link;
            p_link = app_find_br_link(param->sco_disconnected.bd_addr);
            if (p_link != NULL)
            {
                audio_track_volume_out_unmute(p_link->sco_track_handle);
            }
#endif
            audio_sco_discon_handle(&param->sco_disconnected);
        }
        break;

    case BT_EVENT_SCO_DATA_IND:
        {
            audio_sco_data_ind(&param->sco_data_ind);
        }
        break;

    default:
        handle = false;
        break;
    }

    if (handle == true)
    {
        APP_PRINT_INFO1("app_hfp_bt_cback: event_type 0x%04x", event_type);
    }
}


void app_hfp_init(void)
{
    if (app_cfg_const.supported_profile_mask & (HFP_PROFILE_MASK | HSP_PROFILE_MASK))
    {
        bt_hfp_init(RFC_HFP_CHANN_NUM, RFC_HSP_CHANN_NUM, app_cfg_const.hfp_brsf_capability,
                    BT_HFP_HF_CODEC_TYPE_CVSD | BT_HFP_HF_CODEC_TYPE_MSBC);
        bt_mgr_cback_register(app_hfp_bt_cback);
        app_timer_reg_cb(app_hfp_timeout_cb, &hfp_timer_id);

        /* Note: event bus topics are registered in bridge_phone_call_init() */
    }
    audio_hfp_init();
}

/**
 * @brief Get incoming call phone number
 * @return pointer to the phone number string (empty string if no number)
 */
const char *app_hfp_get_current_call_number(void)
{
    if (call_num_hfp[0] > 0 && call_num_hfp[0] <= 20)
    {
        return (const char *)(call_num_hfp + 1);
    }
    return "";
}

/**
 * @brief Clear stored call number (should be called when new call starts)
 */
void app_hfp_clear_call_number(void)
{
    call_num_hfp[0] = 0;
    app_pbap_clear_caller_name();
}

/**
 * @brief Set dial number for outgoing call (used by GUI to pass number to MMI)
 */
void app_hfp_set_dial_number(const char *number, uint8_t len)
{
    if (number == NULL || len == 0 || len > 20)
    {
        return;
    }
    // Clear old data before setting new number
    memset(call_num_hfp, 0, sizeof(call_num_hfp));
    call_num_hfp[0] = len;
    memcpy(call_num_hfp + 1, number, len);
    APP_PRINT_INFO1("app_hfp_set_dial_number: number=%s", TRACE_STRING((const uint8_t *)number));
}
