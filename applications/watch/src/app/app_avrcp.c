/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "trace.h"
#include "btm.h"
#include "bt_avrcp.h"
#include "bt_bond.h"
#include "app_cfg.h"
#include "app_link_util.h"
#include "app_avrcp.h"
#include "app_main.h"
#include "audio_volume.h"
#include "app_mmi.h"
#include "app_task.h"
#include "app_audio_if.h"
#include "audio_track.h"
#include "app_timer.h"
#include "app_linkback.h"
#include "audio_a2dp_src.h"
#include "audio_a2dp_sink.h"
#include "playback_playlist.h"

typedef enum
{
    APP_AVRCP_TIMER_IGNORE_TG_VOLUME_SETTING = 0x00,
} T_APP_AVRCP_TIMER;

static uint8_t avrcp_timer_id = 0;
static uint8_t timer_idx_ignore_tg_volume_setting = 0;
static bool ignore_tg_volume_setting_flag = false;
static bool app_support_abs = false;
bool avrcp_play_status_changed_reg_req_flag = false;
bool watch_supports_abs_vol_flag = true;

bool app_avrcp_get_abs_vol_support(void)
{
    return app_support_abs;
}

void app_avrcp_set_abs_vol_support(bool support_abs)
{
    app_support_abs = support_abs;
}

static void app_avrcp_bt_cback(T_BT_EVENT event_type, void *event_buf, uint16_t buf_len)
{
    T_BT_EVENT_PARAM *param = event_buf;
    T_APP_BR_LINK *p_link;
    bool handle = true;

    switch (event_type)
    {
    case BT_EVENT_AVRCP_CONN_IND:
        {
            p_link = app_find_br_link(param->avrcp_conn_ind.bd_addr);
            if (p_link != NULL)
            {
                bool is_avrcp_connected = false;

                if (app_connected_profile_link_num(AVRCP_PROFILE_MASK) != 0)
                {
                    is_avrcp_connected = true;
                }

                if (is_avrcp_connected || (!app_db.a2dp_control_switch && app_db.a2dp_cur_role == BT_A2DP_ROLE_SNK))
                {
                    APP_PRINT_INFO2("avrcp Connection is rejected: is_avrcp_connected %d, a2dp_control_switch %d",
                                    is_avrcp_connected, app_db.a2dp_control_switch);
                    bt_avrcp_connect_cfm(p_link->bd_addr, false);
                }
                else
                {
                    bt_avrcp_connect_cfm(p_link->bd_addr, true);
                }
            }
        }
        break;

    case BT_EVENT_AVRCP_GET_CAPABILITIES_RSP:
        {
            p_link = app_find_br_link(param->avrcp_browsing_conn_ind.bd_addr);
            if (p_link != NULL)
            {
                uint8_t  capability_count;
                uint8_t *capabilities;

                capability_count = param->avrcp_get_capabilities_rsp.capability_count;
                capabilities = param->avrcp_get_capabilities_rsp.capabilities;
                while (capability_count != 0)
                {
                    bt_avrcp_register_notification_req(p_link->bd_addr, *capabilities);
                    capability_count -= 1;
                    capabilities += 1;
                }
            }
        }
        break;

    case BT_EVENT_AVRCP_ABSOLUTE_VOLUME_SET:
        {
            if (app_db.audio_play_mode == MODE_APP_A2DP_SNK)
            {
                audio_a2dp_sink_set_volume(param->avrcp_absolute_volume_set.volume);
            }
        }
        break;

    case BT_EVENT_AVRCP_PLAY_STATUS_CHANGED:
        {
            if (app_db.audio_play_mode == MODE_APP_A2DP_SNK)
            {
                p_link = app_find_br_link(param->avrcp_play_status_changed.bd_addr);
                if (p_link != NULL)
                {
                    APP_PRINT_INFO1("BT_EVENT_AVRCP_PLAY_STATUS_CHANGED play status = 0x%x",
                                    param->avrcp_play_status_changed.play_status);
                    if ((param->avrcp_play_status_changed.play_status != BT_AVRCP_PLAY_STATUS_ERROR) && \
                        (p_link->avrcp_play_status != param->avrcp_play_status_changed.play_status))
                    {

                        p_link->avrcp_play_status = param->avrcp_play_status_changed.play_status;
                        //gui_update_by_event(GUI_EVENT_PLAYER, NULL, false);
                    }
                }
            }
        }
        break;

    case BT_EVENT_AVRCP_PLAY_STATUS_RSP:
        {
            if (app_db.audio_play_mode == MODE_APP_A2DP_SNK)
            {
                p_link = app_find_br_link(param->avrcp_play_status_rsp.bd_addr);
                if (p_link != NULL)
                {
                    APP_PRINT_INFO1("BT_EVENT_AVRCP_PLAY_STATUS_RSP play status = 0x%x",
                                    param->avrcp_play_status_rsp.play_status);
                    if (param->avrcp_play_status_rsp.play_status != BT_AVRCP_PLAY_STATUS_ERROR && \
                        (p_link->avrcp_play_status != param->avrcp_play_status_rsp.play_status))
                    {
                        p_link->avrcp_play_status = param->avrcp_play_status_rsp.play_status;
                        //gui_update_by_event(GUI_EVENT_PLAYER, NULL, false);
                    }
                }
            }
        }
        break;

    case BT_EVENT_AVRCP_VOLUME_CHANGED:
        {
            if (app_db.audio_play_mode == MODE_APP_A2DP_SRC)
            {
                app_avrcp_set_abs_vol_support(true);
                if (ignore_tg_volume_setting_flag)
                {
                    APP_PRINT_INFO0("ignore tg volume setting");
                }
                else if (watch_supports_abs_vol_flag)
                {
                    p_link = app_find_br_link(param->avrcp_volume_changed.bd_addr);
                    if (p_link != NULL)
                    {
                        uint8_t volume = (param->avrcp_volume_changed.volume *
                                          app_cfg_volume.playback_volume_max + 0x7F / 2) / 0x7F;

                        APP_PRINT_INFO1("TG_abs_vol = %x", param->avrcp_volume_changed.volume);
                        if (volume == 0 && param->avrcp_volume_changed.volume != 0)
                        {
                            audio_a2dp_src_set_volume(1);
                        }
                        else
                        {
                            audio_a2dp_src_set_volume(volume);
                        }
                        if (app_db.playback_muted)
                        {
                            app_db.playback_muted = false;
                        }
                    }
                }
            }
        }
        break;

    case BT_EVENT_AVRCP_ABSOLUTE_VOLUME_SET_RSP:
        {
            ignore_tg_volume_setting_flag = true;
            app_start_timer(&timer_idx_ignore_tg_volume_setting, "ignore_tg_volume_setting",
                            avrcp_timer_id,
                            APP_AVRCP_TIMER_IGNORE_TG_VOLUME_SETTING, 0, false, 100);
        }
        break;
    case BT_EVENT_AVRCP_VOLUME_UP:
        {

        }
        break;

    case BT_EVENT_AVRCP_VOLUME_DOWN:
        {

        }
        break;

    case BT_EVENT_AVRCP_REG_VOLUME_CHANGED:
        {
            p_link = app_find_br_link(param->avrcp_reg_volume_changed.bd_addr);
            if (p_link != NULL)
            {
                uint8_t pair_idx;
                uint8_t vol;

                if (bt_bond_index_get(p_link->bd_addr, &pair_idx) == false)
                {
                    APP_PRINT_ERROR0("avrcp reg volume change, get pair idx fail");
                    break;
                }

                vol = (app_cfg_nv.audio_gain_level[pair_idx] * 0x7F +
                       app_cfg_volume.playback_volume_max / 2) /
                      app_cfg_volume.playback_volume_max;
                bt_avrcp_volume_change_register_rsp(p_link->bd_addr, vol);
            }
        }
        break;

    case BT_EVENT_AVRCP_PLAY_STATUS_CHANGED_REG_REQ:
        {
            p_link = app_find_br_link(param->avrcp_reg_play_status_changed.bd_addr);
            if (p_link != NULL)
            {
                avrcp_play_status_changed_reg_req_flag = 1;
                if (app_db.a2dp_src_state == APP_A2DP_SRC_STREAM_START)
                    bt_avrcp_play_status_change_register_rsp(param->avrcp_reg_play_status_changed.bd_addr,
                                                             BT_AVRCP_PLAY_STATUS_PLAYING);
                else if (app_db.a2dp_src_state == APP_A2DP_SRC_STREAM_STOP)
                    bt_avrcp_play_status_change_register_rsp(param->avrcp_reg_play_status_changed.bd_addr,
                                                             BT_AVRCP_PLAY_STATUS_PAUSED);
            }
        }
        break;

    case BT_EVENT_AVRCP_PLAY:
        {
            if (app_db.audio_play_mode == MODE_APP_A2DP_SRC)
            {
                if (app_audio_get_play_status() == APP_AUDIO_STATE_PAUSE)
                {
                    T_IO_MSG play_msg;
                    play_msg.type = IO_MSG_TYPE_WRISTBNAD;
                    play_msg.subtype = IO_MSG_MMI;
                    play_msg.u.param = MMI_AV_PLAY_PAUSE;
                    app_send_msg_to_apptask(&play_msg);
                }
                else if (app_audio_get_play_status() == APP_AUDIO_STATE_STOP)
                {
                    playback_play_next_music();
                }
            }
        }
        break;

    case BT_EVENT_AVRCP_PAUSE:
        {
            if (app_db.audio_play_mode == MODE_APP_A2DP_SRC)
            {
                if (app_audio_get_play_status() == APP_AUDIO_STATE_PLAY)
                {
                    T_IO_MSG play_msg;
                    play_msg.type = IO_MSG_TYPE_WRISTBNAD;
                    play_msg.subtype = IO_MSG_MMI;
                    play_msg.u.param = MMI_AV_PLAY_PAUSE;
                    app_send_msg_to_apptask(&play_msg);
                }
            }
        }
        break;

    case BT_EVENT_AVRCP_STOP:
        {
            if (app_db.audio_play_mode == MODE_APP_A2DP_SRC)
            {
                if (app_audio_get_play_status() != APP_AUDIO_STATE_STOP)
                {
                    T_IO_MSG play_msg;
                    play_msg.type = IO_MSG_TYPE_WRISTBNAD;
                    play_msg.subtype = IO_MSG_MMI;
                    play_msg.u.param = MMI_AV_STOP;
                    app_send_msg_to_apptask(&play_msg);
                }
            }
        }
        break;

    case BT_EVENT_AVRCP_FORWARD:
        {
            if (app_db.audio_play_mode == MODE_APP_A2DP_SRC)
            {
                playback_play_next_music();
            }
        }
        break;

    case BT_EVENT_AVRCP_BACKWARD:
        {
            if (app_db.audio_play_mode == MODE_APP_A2DP_SRC)
            {
                playback_play_prev_music();
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
        APP_PRINT_INFO1("app_avrcp_bt_cback: event_type 0x%04x", event_type);
    }
}

void app_avrcp_support_absolute_volume(bool support)
{
    if (support)
    {
        if (app_audio_cfg.support_local_source)
        {
            /* source support abusolute volume: ct_features = BT_AVRCP_FEATURE_CATEGORY_2*/
            app_avrcp_feature_cfg.ct_features = BT_AVRCP_FEATURE_CATEGORY_2;
            app_avrcp_feature_cfg.tg_features = BT_AVRCP_FEATURE_CATEGORY_1;
        }
        if (app_audio_cfg.support_sink)
        {
            /* sink support abusolute volume: tg_features = BT_AVRCP_FEATURE_CATEGORY_2*/
            app_avrcp_feature_cfg.ct_features |= BT_AVRCP_FEATURE_CATEGORY_1;
            app_avrcp_feature_cfg.tg_features |= BT_AVRCP_FEATURE_CATEGORY_2;
        }
    }
    else
    {
        if (app_audio_cfg.support_local_source)
        {
            app_avrcp_feature_cfg.tg_features = BT_AVRCP_FEATURE_CATEGORY_1;
        }
        if (app_audio_cfg.support_sink)
        {
            app_avrcp_feature_cfg.ct_features = BT_AVRCP_FEATURE_CATEGORY_1;
        }
    }
    DBG_DIRECT("app_absolute_volume_cfg:ct_features=%d,tg_features=%d",
               app_avrcp_feature_cfg.ct_features, app_avrcp_feature_cfg.tg_features);
}

void app_avrcp_timeout_cb(uint8_t timer_id, uint16_t timer_chann)
{
    APP_PRINT_TRACE2("app_avrcp_timeout_cb: timer_id %d, timer_chann %d", timer_id, timer_chann);

    switch (timer_id)
    {
    case APP_AVRCP_TIMER_IGNORE_TG_VOLUME_SETTING:
        {
            app_stop_timer(&timer_idx_ignore_tg_volume_setting);
            ignore_tg_volume_setting_flag = false;
        }
        break;
    default:
        break;
    }
}

void app_avrcp_init(void)
{
    if (app_cfg_const.supported_profile_mask & AVRCP_PROFILE_MASK)
    {
        app_avrcp_support_absolute_volume(true);
        bt_avrcp_init(app_avrcp_feature_cfg.ct_features,
                      app_avrcp_feature_cfg.tg_features);
        app_timer_reg_cb(app_avrcp_timeout_cb, &avrcp_timer_id);
        bt_mgr_cback_register(app_avrcp_bt_cback);
    }
}
