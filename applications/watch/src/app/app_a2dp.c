/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <string.h>
#include "trace.h"
#include "btm.h"
#include "bt_a2dp.h"
#include "app_cfg.h"
#include "app_main.h"
#include "app_multilink.h"
#include "app_link_util.h"
#include "app_a2dp.h"
#include "app_timer.h"
#include "audio_type.h"
#include "app_linkback.h"
#include "app_task.h"
#include "os_sync.h"
#include "app_mmi.h"
#include "bt_hfp.h"
#include "bt_bond.h"
#include "bt_types.h"
#include "app_avrcp.h"
#include "audio_a2dp_src.h"
#include "app_audio_if.h"
#include "audio_a2dp_sink.h"


#define   SRC_A2DP_STREAM_MAX_CREDITS           8
static uint8_t src_a2dp_credits = SRC_A2DP_STREAM_MAX_CREDITS;
static uint8_t active_a2dp_index = 0;
static uint8_t app_a2dp_src_timer_id = 0;
static uint8_t timer_idx_a2dp_open_stream = 0;

static bool reconnect_cause_no_resource = false;
extern bool avrcp_play_status_changed_reg_req_flag;


void app_a2dp_set_active_idx(uint8_t idx)
{
    active_a2dp_index = idx;
}

uint8_t app_a2dp_get_active_idx(void)
{
    return active_a2dp_index;
}

uint8_t app_a2dp_get_src_credits(void)
{
    return src_a2dp_credits;
}

void app_a2dp_use_src_credits(void)
{
    if (src_a2dp_credits)
    {
        src_a2dp_credits--;
    }
}

static void app_a2dp_bt_cback(T_BT_EVENT event_type, void *event_buf, uint16_t buf_len)
{
    T_BT_EVENT_PARAM *param = event_buf;
    T_APP_BR_LINK *p_link;
    bool handle = true;

    switch (event_type)
    {
    case BT_EVENT_A2DP_CONN_IND:
        {
            p_link = app_find_br_link(param->a2dp_conn_ind.bd_addr);
            if (p_link != NULL)
            {
                bool is_a2dp_connected = false;

                if (app_connected_profile_link_num(A2DP_PROFILE_MASK) != 0)
                {
                    is_a2dp_connected = true;
                }

                if (is_a2dp_connected || (!app_db.a2dp_control_switch &&
                                          app_db.a2dp_cur_role == BT_A2DP_ROLE_SNK) || app_db.audio_play_mode == MODE_APP_PLAYBACK)
                {
                    APP_PRINT_INFO2("A2dp Connection is rejected: is_a2dp_connected %d, a2dp_control_switch %d",
                                    is_a2dp_connected, app_db.a2dp_control_switch);
                    bt_a2dp_connect_cfm(p_link->bd_addr, 0, false);
                }
                else
                {
                    bt_a2dp_connect_cfm(p_link->bd_addr, 0, true);
                }
            }
        }
        break;

    case BT_EVENT_A2DP_CONFIG_CMPL:
        {
            if (param->a2dp_config_cmpl.role == BT_A2DP_ROLE_SNK)
            {
                bt_a2dp_stream_delay_report_req(param->a2dp_config_cmpl.bd_addr, app_cfg_nv.audio_latency);
            }
        }
        break;

    case BT_EVENT_A2DP_CONN_CMPL:
        {
            p_link = app_find_br_link(param->a2dp_conn_cmpl.bd_addr);
            if (p_link == NULL)
            {
                APP_PRINT_ERROR0("ERROR: A2dp connect cmpl find link fail!");
                return;
            }
            //Both sink and src role will use this id for bt related actions.
            app_a2dp_set_active_idx(p_link->id);

            if (app_db.audio_play_mode == MODE_APP_A2DP_SRC)
            {
                app_db.a2dp_src_state = APP_A2DP_SRC_CONN;

                app_start_timer(&timer_idx_a2dp_open_stream, "a2dp_open_stream",
                                app_a2dp_src_timer_id, TIMER_ID_OPEN_A2DP_STREAM, 0, false,
                                1000);

                src_a2dp_credits = SRC_A2DP_STREAM_MAX_CREDITS;
            }
            else
            {
                if (app_db.a2dp_control_switch == false)
                {
                    linkback_profile_disconnect_start(param->a2dp_conn_cmpl.bd_addr, A2DP_PROFILE_MASK);
                }
            }
        }
        break;

    case BT_EVENT_A2DP_DISCONN_CMPL:
        {
            if (app_db.audio_play_mode == MODE_APP_A2DP_SRC)
            {
                app_db.a2dp_src_state = APP_A2DP_SRC_DISCONN;
                audio_a2dp_src_play_state_handle(AUDIO_A2DP_SRC_PROFILE_DISCON);

                if (reconnect_cause_no_resource)
                {
                    reconnect_cause_no_resource = false;
                    T_LINKBACK_RETRY_PARAM retry_param =
                    {
                        .conn_retry_timeout = 0,
                        .conn_retry_cnt = 0,
                        .prof_retry_timeout = 0,
                        .prof_retry_cnt = 0,
                        .delay_timeout = 0
                    };
                    linkback_create_connection(param->hfp_conn_ind.bd_addr, A2DP_PROFILE_MASK,
                                               T_DEVICE_TYPE_EARPHONE, retry_param);
                }
            }
        }
        break;

    case BT_EVENT_A2DP_STREAM_OPEN:
        {
            //In src role, we will need to open the stream and handle the response.
            if (app_db.audio_play_mode == MODE_APP_A2DP_SRC)
            {
                app_db.a2dp_src_state = APP_A2DP_SRC_STREAM_CONN;

                app_stop_timer(&timer_idx_a2dp_open_stream);

                audio_a2dp_src_init();
            }
        }
        break;

    case BT_EVENT_A2DP_STREAM_START_IND:
        {
            //In src role, we will suspend the stream opened by slave.
            //In sink role, we need to confirm the stream opened by master and set the config to audio module.
            if (app_db.audio_play_mode == MODE_APP_A2DP_SRC)
            {
                bt_a2dp_stream_start_cfm(param->a2dp_stream_start_ind.bd_addr, true);
                app_db.a2dp_src_state = APP_A2DP_SRC_STREAM_START;
                bt_sniff_mode_disable(param->a2dp_stream_start_ind.bd_addr);
                bt_link_qos_set(param->a2dp_stream_start_ind.bd_addr, BT_QOS_TYPE_GUARANTEED, 18);
                bt_a2dp_stream_suspend_req(param->a2dp_stream_start_ind.bd_addr);
            }
            else if (app_db.audio_play_mode == MODE_APP_A2DP_SNK)
            {
                bt_active_link_set(param->a2dp_stream_start_ind.bd_addr);
                audio_a2dp_sink_stream_start_handle(&param->a2dp_stream_start_ind);
            }
        }
        break;

    case BT_EVENT_A2DP_STREAM_START_RSP:
        {
            //In src role, we need to handle the stream start response.
            //After stream start, we can send audio stream to sink device.
            if (app_db.audio_play_mode == MODE_APP_A2DP_SRC)
            {
                app_db.a2dp_src_state = APP_A2DP_SRC_STREAM_START;
                bt_sniff_mode_disable(param->a2dp_stream_start_rsp.bd_addr);
                bt_link_qos_set(param->a2dp_stream_start_rsp.bd_addr, BT_QOS_TYPE_GUARANTEED, 18);
                if (avrcp_play_status_changed_reg_req_flag)
                {
                    avrcp_play_status_changed_reg_req_flag = 0;
                    bt_avrcp_play_status_change_req(param->a2dp_stream_start_rsp.bd_addr, BT_AVRCP_PLAY_STATUS_PLAYING);
                }
                if (playback_db.sd_play_state == APP_AUDIO_STATE_TRY_PLAYING)
                {
                    audio_a2dp_src_pipe_create();
                }
            }
        }
        break;

    case BT_EVENT_A2DP_STREAM_DATA_IND:
        {
            //In sink role, we receive the audio stream by this event and write the stream data to audio track.
            if (app_db.audio_play_mode == MODE_APP_A2DP_SNK)
            {
                audio_a2dp_sink_data_ind(param->a2dp_stream_data_ind.bd_addr,
                                         (uint8_t *) & (param->a2dp_stream_data_ind));
            }
        }
        break;

    case BT_EVENT_A2DP_STREAM_STOP:
        {
            //Both sink and src role need to handle stream stop event. Set play state, release audio track, etc.
            if (app_db.audio_play_mode == MODE_APP_A2DP_SRC)
            {
                app_db.a2dp_src_state = APP_A2DP_SRC_STREAM_STOP;
                bt_sniff_mode_enable(param->a2dp_stream_stop.bd_addr, 784, 816, 0, 0);
                bt_link_qos_set(param->a2dp_stream_stop.bd_addr, BT_QOS_TYPE_GUARANTEED, 38);
                if (avrcp_play_status_changed_reg_req_flag)
                {
                    avrcp_play_status_changed_reg_req_flag = 0;
                    bt_avrcp_play_status_change_req(param->a2dp_stream_stop.bd_addr, BT_AVRCP_PLAY_STATUS_STOPPED);
                }
                audio_a2dp_src_play_state_handle(AUDIO_A2DP_SRC_STREAM_STOP);
            }
            else if (app_db.audio_play_mode == MODE_APP_A2DP_SNK)
            {
                audio_a2dp_sink_track_release();
            }
        }
        break;

    case BT_EVENT_A2DP_STREAM_DATA_RSP:
        {
            //In src role, we will receive this event after send one stream packet.
            if (app_db.audio_play_mode == MODE_APP_A2DP_SRC)
            {
                src_a2dp_credits++;
                if (src_a2dp_credits > SRC_A2DP_STREAM_MAX_CREDITS)
                {
                    src_a2dp_credits = SRC_A2DP_STREAM_MAX_CREDITS;
                }
                APP_PRINT_INFO1("get a2dp credits = %d", src_a2dp_credits);
            }
        }
        break;

    case BT_EVENT_A2DP_STREAM_OPEN_FAIL:
        {
            if (app_db.audio_play_mode == MODE_APP_A2DP_SRC)
            {
                p_link = app_find_br_link(param->a2dp_stream_open_fail.bd_addr);
                APP_PRINT_INFO1("a2dp_stream_open_fail.cause = %d", param->a2dp_stream_open_fail.cause);
                if (p_link != NULL)
                {
                    bt_a2dp_disconnect_req(param->a2dp_stream_open_fail.bd_addr);//linkback_profile_disconnect_start
                    if (param->a2dp_stream_open_fail.cause == (L2C_ERR | L2C_CONN_RSP_NO_RESOURCE))
                    {
                        reconnect_cause_no_resource = true;
                    }
                }
            }
        }
        break;

    case BT_EVENT_A2DP_STREAM_CLOSE:
        {
            //Both sink and src role need to handle stream close event.
            if (app_db.audio_play_mode == MODE_APP_A2DP_SRC)
            {
                bt_link_qos_set(param->a2dp_stream_close.bd_addr, BT_QOS_TYPE_GUARANTEED, 38);
                app_mmi_handle_action(MMI_AV_PLAY_PAUSE);
            }
            else if (app_db.audio_play_mode == MODE_APP_A2DP_SNK)
            {
                audio_a2dp_sink_track_release();
            }

        }
        break;

    default:
        {
            handle = false;
        }
        break;
    }

    if (handle == true)
    {
        APP_PRINT_INFO1("app_a2dp_bt_cback: event_type 0x%04x", event_type);
    }
}

void app_a2dp_src_timeout_cb(uint8_t timer_id, uint16_t timer_chann)
{
    APP_PRINT_TRACE2("app_a2dp_src_timeout_cb: timer_id %d, timer_chann %d", timer_id, timer_chann);

    switch (timer_id)
    {
    case TIMER_ID_OPEN_A2DP_STREAM:
        {
            app_stop_timer(&timer_idx_a2dp_open_stream);
            uint8_t a2dp_idx = app_a2dp_get_active_idx();
            bt_a2dp_stream_open_req(app_db.br_link[a2dp_idx].bd_addr, BT_A2DP_ROLE_SNK);
        }
        break;

    default:
        break;
    }
}


void app_a2dp_init(void)
{
    if (app_cfg_const.supported_profile_mask & A2DP_PROFILE_MASK)
    {
        bt_a2dp_init(BT_A2DP_CAPABILITY_MEDIA_TRANSPORT | BT_A2DP_CAPABILITY_MEDIA_CODEC |
                     BT_A2DP_CAPABILITY_DELAY_REPORTING);
        app_db.a2dp_cur_role = BT_A2DP_ROLE_SRC;
        if (app_cfg_const.a2dp_codec_type_sbc)
        {
            T_BT_A2DP_STREAM_ENDPOINT sep;

            sep.role = BT_A2DP_ROLE_SRC;
            sep.codec_type = BT_A2DP_CODEC_TYPE_SBC;
            sep.u.codec_sbc.sampling_frequency_mask = BT_A2DP_SBC_SAMPLING_FREQUENCY_48KHZ;
            sep.u.codec_sbc.channel_mode_mask = app_cfg_const.sbc_channel_mode;
            sep.u.codec_sbc.subbands_mask = app_cfg_const.sbc_subbands;
            sep.u.codec_sbc.allocation_method_mask = app_cfg_const.sbc_allocation_method;
            bt_a2dp_stream_endpoint_add(sep);
        }
        if (app_cfg_const.a2dp_codec_type_aac)
        {
            T_BT_A2DP_STREAM_ENDPOINT sep;

            sep.codec_type = BT_A2DP_CODEC_TYPE_AAC;
            sep.u.codec_aac.object_type_mask = app_cfg_const.aac_object_type;
            sep.u.codec_aac.sampling_frequency_mask = app_cfg_const.aac_sampling_frequency;
            sep.u.codec_aac.channel_number_mask = app_cfg_const.aac_channel_number;
            sep.u.codec_aac.vbr_supported = app_cfg_const.aac_vbr_supported;
            sep.u.codec_aac.bit_rate = app_cfg_const.aac_bit_rate;
            bt_a2dp_stream_endpoint_add(sep);
        }
        bt_mgr_cback_register(app_a2dp_bt_cback);
        app_timer_reg_cb(app_a2dp_src_timeout_cb, &app_a2dp_src_timer_id);
    }
}

