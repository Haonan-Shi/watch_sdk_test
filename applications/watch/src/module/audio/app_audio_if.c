/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include "trace.h"
#include "app_mmi.h"
#include "app_main.h"
#include "app_cfg.h"
#include "audio_playback.h"
#include "app_a2dp.h"
#include "app_hfp.h"
#include "bt_bond.h"
// #include "map_mgr.h"
#include "app_audio_if.h"
#include "app_audio_policy.h"
#include "app_playback_update_file.h"
#include "app_task.h"
#include "app_audio_mode_switch.h"
#include "audio_record.h"
#include "audio_a2dp_sink.h"
#include "audio_a2dp_src.h"
#include "audio_hfp.h"
#include "playback_playlist.h"
#include "app_fs_if.h"
#if CONFIG_MUSIC_PLAYER
#include "app_music_player.h"
#endif

/*============================================================================*
 *                            Variables
 *============================================================================*/
static uint16_t play_fail_index = 0;

/*============================================================================*
 *                           Private Functions
 *============================================================================*/

/**
 * @brief Handles low-battery protection while local playback is active.
 */
static void app_audio_low_batt_cb(void)
{
    APP_PRINT_INFO0("battery low for playback");
    app_db.batt.allow_open.playback = false;

    T_IO_MSG play_msg;
    play_msg.type = IO_MSG_TYPE_WRISTBNAD;
    play_msg.subtype = IO_MSG_MMI;
    play_msg.u.param = MMI_AV_STOP;
    app_send_msg_to_apptask(&play_msg);
}

/**
 * @brief Restores local playback availability after battery level recovers.
 */
static void app_audio_high_batt_cb(void)
{
    APP_PRINT_INFO0("battery high for playback");
    app_db.batt.allow_open.playback = true;
}

/*============================================================================*
 *                           Public Functions
 *============================================================================*/

T_APP_AUDIO_ERROR app_audio_play_by_name(uint8_t *file_name, uint16_t length)
{
    if (app_fs_disk_power_down_check_idle())
    {
        app_fs_disk_power_down_disable(APP_DISK_CHECK_PLAYBACK);
        app_fs_disk_power_on();
    }

    if (app_db.audio_play_mode == MODE_APP_A2DP_SNK)
    {
        APP_PRINT_INFO0("not support play by name in sink mode");
        return APP_AUDIO_MODE_ERROR;
    }

    if ((playback_db.sd_play_state == APP_AUDIO_STATE_PAUSE) ||
        (playback_db.sd_play_state == APP_AUDIO_STATE_STOP))
    {
        if (g_curr_song != NULL)
        {
            Mp3_FreeHandle(g_curr_song);
            g_curr_song = NULL;
        }

        EMp3Res mp3_res;
        g_curr_song = Mp3_CreateHandle((TCHAR *)file_name, &mp3_res);

        if (g_curr_song == NULL || mp3_res != MP3RES_OK)
        {
            APP_PRINT_INFO0("Play failed! May be not a mp3 file.");
            play_fail_index ++;
            if (play_fail_index == playback_get_song_count())
            {
                play_fail_index = 0;
                app_audio_pause();
            }
            else
            {
                playback_play_next_music();
            }
            return APP_AUDIO_OPEN_FILE_ERROR;
        }
        else
        {
            play_fail_index = 0;
        }
        uint16_t res = app_audio_start();
        if (res == APP_AUDIO_SUCCESS)
        {
            playback_db.file_name = file_name;
            playback_db.name_length = length;
        }
        return (T_APP_AUDIO_ERROR)res;
    }
    else if (playback_db.sd_play_state == APP_AUDIO_STATE_PLAY)
    {
        playback_db.op_next_action = APP_AUDIO_STOPPED_SWITCH_BY_NAME;
        playback_db.file_name = file_name;
        playback_db.name_length = length;
        uint16_t res = app_audio_pause();

        if (res != APP_AUDIO_SUCCESS)
        {
            playback_db.op_next_action = APP_AUDIO_STOPPED_IDLE_ACTION;
        }
        return (T_APP_AUDIO_ERROR)res;
    }
    else  // Ignore the command because other cammand is on going
    {
        return APP_AUDIO_MODE_ERROR;
    }
}

T_APP_AUDIO_STATE app_audio_get_play_status(void)
{
    T_APP_AUDIO_STATE play_status = APP_AUDIO_STATE_STOP;

    if ((app_db.audio_play_mode == MODE_APP_PLAYBACK) || (app_db.audio_play_mode == MODE_APP_A2DP_SRC))
    {
        play_status = playback_db.sd_play_state;
    }
    else if (app_db.audio_play_mode == MODE_APP_A2DP_SNK)
    {
        uint8_t active_a2dp_idx = app_a2dp_get_active_idx();
        play_status = (T_APP_AUDIO_STATE)app_db.br_link[active_a2dp_idx].avrcp_play_status;
    }
    APP_PRINT_INFO1("app_audio_get_play_status play_status = 0x%02x", play_status);
    return play_status;
}

void app_volume_up(void)
{
    if (app_hfp_get_call_status() != APP_HFP_CALL_IDLE)
    {
        audio_hfp_volume_up();
    }
    else
    {
        if (app_db.audio_play_mode == MODE_APP_PLAYBACK)
        {
            audio_playback_volume_up();
        }
        else if (app_db.audio_play_mode == MODE_APP_A2DP_SRC)
        {
            audio_a2dp_src_volume_up();
        }
        else if (app_db.audio_play_mode == MODE_APP_A2DP_SNK)
        {
            audio_a2dp_sink_volume_up();
        }
    }

    uint8_t volume = app_audio_get_volume();
    event_bus_publish(EVENT_BUS_TOPIC_AUDIO_VOLUME_UP, &volume, sizeof(uint8_t));
}

void app_volume_down(void)
{
    if (app_hfp_get_call_status() != APP_HFP_CALL_IDLE)
    {
        audio_hfp_volume_down();
    }
    else
    {
        if (app_db.audio_play_mode == MODE_APP_PLAYBACK)
        {
            audio_playback_volume_down();
        }
        else if (app_db.audio_play_mode == MODE_APP_A2DP_SRC)
        {
            audio_a2dp_src_volume_down();
        }
        else if (app_db.audio_play_mode == MODE_APP_A2DP_SNK)
        {
            audio_a2dp_sink_volume_down();
        }
    }

    uint8_t volume = app_audio_get_volume();
    event_bus_publish(EVENT_BUS_TOPIC_AUDIO_VOLUME_DOWN, &volume, sizeof(uint8_t));
}

uint8_t app_audio_get_volume(void)
{
    if (app_hfp_get_call_status() != APP_HFP_CALL_IDLE)
    {
        uint8_t active_idx;
        uint8_t pair_idx;
        active_idx = app_hfp_get_active_hf_index();

        if (bt_bond_index_get(app_db.br_link[active_idx].bd_addr, &pair_idx) == false)
        {
            APP_PRINT_ERROR0("app_volume_get: find active hfp pair_index fail");
            return 0;
        }
        return app_cfg_nv.voice_gain_level[pair_idx];
    }
    else
    {
        if (app_db.audio_play_mode == MODE_APP_PLAYBACK)
        {
            return playback_db.volume;
        }
        else if (app_db.audio_play_mode == MODE_APP_A2DP_SRC)
        {
            uint8_t pair_idx;
            uint8_t a2dp_idx = app_a2dp_get_active_idx();
            if (bt_bond_index_get(app_db.br_link[a2dp_idx].bd_addr, &pair_idx) == false)
            {
                APP_PRINT_ERROR0("app_volume_get: find a2dp pair_index fail");
                return 0;
            }
            return app_cfg_nv.audio_gain_level[pair_idx];
        }
        else if (app_db.audio_play_mode == MODE_APP_A2DP_SNK)
        {
            uint8_t pair_idx;
            uint8_t a2dp_idx = app_a2dp_get_active_idx();
            if (bt_bond_index_get(app_db.br_link[a2dp_idx].bd_addr, &pair_idx) == false)
            {
                APP_PRINT_ERROR0("app_volume_up: find active a2dp pair_index fail");
                return 0;
            }
            return app_cfg_nv.audio_gain_level[pair_idx];
        }
        else
        {
            return 0;
        }
    }
}

uint16_t app_audio_start(void)
{
    uint16_t res = APP_AUDIO_SUCCESS;
    if (app_db.audio_play_mode == MODE_APP_PLAYBACK)
    {
        app_fs_disk_power_down_disable(APP_DISK_CHECK_PLAYBACK);
        app_fs_disk_power_on();
        res = audio_playback_start();
    }
    else if (app_db.audio_play_mode == MODE_APP_A2DP_SRC &&
             app_db.a2dp_src_state != APP_A2DP_SRC_DISCONN)
    {
        app_fs_disk_power_down_disable(APP_DISK_CHECK_PLAYBACK);
        app_fs_disk_power_on();
        audio_a2dp_src_start();//add return value
    }
    else if (app_db.audio_play_mode == MODE_APP_A2DP_SNK)
    {
        uint8_t active_a2dp_idx = app_a2dp_get_active_idx();
        if ((app_db.br_link[active_a2dp_idx].connected_profile & A2DP_PROFILE_MASK) &&
            (app_db.br_link[active_a2dp_idx].connected_profile & AVRCP_PROFILE_MASK))
        {
            if (app_db.br_link[active_a2dp_idx].avrcp_play_status != BT_AVRCP_PLAY_STATUS_PLAYING)
            {
                // Update play status after AVRCP play status event received
                bt_avrcp_play(app_db.br_link[active_a2dp_idx].bd_addr);
            }
        }
    }

    T_APP_AUDIO_STATE state = app_audio_get_play_status();
    event_bus_publish(EVENT_BUS_TOPIC_AUDIO_PLAY_STATUS_CHANGED, &state, sizeof(T_APP_AUDIO_STATE));

    return res;
}

uint16_t app_audio_pause(void)
{
    uint16_t res = APP_AUDIO_SUCCESS;
    if (app_db.audio_play_mode == MODE_APP_PLAYBACK)
    {
        res = audio_playback_stop();
        app_fs_disk_power_down_enable(APP_DISK_CHECK_PLAYBACK);
    }
    else if (app_db.audio_play_mode == MODE_APP_A2DP_SRC)
    {
        audio_a2dp_src_stop();//add return value
        app_fs_disk_power_down_enable(APP_DISK_CHECK_PLAYBACK);
    }
    else if (app_db.audio_play_mode == MODE_APP_A2DP_SNK)
    {
        uint8_t active_a2dp_idx = app_a2dp_get_active_idx();
        if ((app_db.br_link[active_a2dp_idx].connected_profile & A2DP_PROFILE_MASK) &&
            (app_db.br_link[active_a2dp_idx].connected_profile & AVRCP_PROFILE_MASK))
        {
            if (app_db.br_link[active_a2dp_idx].avrcp_play_status == BT_AVRCP_PLAY_STATUS_PLAYING)
            {
                // Update play status after AVRCP play status event received
                bt_avrcp_pause(app_db.br_link[active_a2dp_idx].bd_addr);
            }
        }
    }

    T_APP_AUDIO_STATE st = app_audio_get_play_status();
    event_bus_publish(EVENT_BUS_TOPIC_AUDIO_PLAY_STATUS_CHANGED, &st, sizeof(T_APP_AUDIO_STATE));
    return res;
}

uint16_t app_audio_stop(void)
{
    uint16_t res = APP_AUDIO_SUCCESS;
    if (app_db.audio_play_mode == MODE_APP_PLAYBACK)
    {
        if (app_audio_get_play_status() == APP_AUDIO_STATE_PLAY)
        {
            res = app_audio_pause();
        }
        res = audio_playback_close_flie();
        playback_db.sd_play_state = APP_AUDIO_STATE_STOP;
    }
    else if (app_db.audio_play_mode == MODE_APP_A2DP_SRC)
    {
        if (app_audio_get_play_status() == APP_AUDIO_STATE_PLAY)
        {
            res = app_audio_pause();
            playback_db.sd_play_state = APP_AUDIO_STATE_TRY_STOPPING;
        }
        else
        {
            playback_db.sd_play_state = APP_AUDIO_STATE_STOP;
        }
        res = audio_playback_close_flie();
    }
    else if (app_db.audio_play_mode == MODE_APP_A2DP_SNK)
    {
        uint8_t active_a2dp_idx = app_a2dp_get_active_idx();
        if ((app_db.br_link[active_a2dp_idx].connected_profile & A2DP_PROFILE_MASK) &&
            (app_db.br_link[active_a2dp_idx].connected_profile & AVRCP_PROFILE_MASK))
        {
            // Update play status after AVRCP play status event received
            bt_avrcp_stop(app_db.br_link[active_a2dp_idx].bd_addr);
        }
    }

    T_APP_AUDIO_STATE st = app_audio_get_play_status();
    event_bus_publish(EVENT_BUS_TOPIC_AUDIO_PLAY_STATUS_CHANGED, &st, sizeof(T_APP_AUDIO_STATE));

    return res;
}

uint16_t app_audio_next(void)
{
    uint16_t res = APP_AUDIO_SUCCESS;
    if (app_db.audio_play_mode == MODE_APP_PLAYBACK)
    {
        playback_play_next_music();
    }
    else if (app_db.audio_play_mode == MODE_APP_A2DP_SRC)
    {
        playback_play_next_music();
    }
    else if (app_db.audio_play_mode == MODE_APP_A2DP_SNK)
    {
        uint8_t active_a2dp_idx = app_a2dp_get_active_idx();
        if ((app_db.br_link[active_a2dp_idx].connected_profile & A2DP_PROFILE_MASK) &&
            (app_db.br_link[active_a2dp_idx].connected_profile & AVRCP_PROFILE_MASK))
        {
            // Update play status after AVRCP play status event received
            bt_avrcp_forward(app_db.br_link[active_a2dp_idx].bd_addr);
        }
    }

    if (app_db.audio_play_mode != MODE_APP_A2DP_SNK)
    {
        uint16_t index = playback_get_cur_play_index();
        event_bus_publish(EVENT_BUS_TOPIC_AUDIO_PLAY_INDEX_UPDATE, &index, sizeof(uint16_t));
    }

    return res;
}

uint16_t app_audio_prev(void)
{
    uint16_t res = APP_AUDIO_SUCCESS;
    if (app_db.audio_play_mode == MODE_APP_PLAYBACK)
    {
        playback_play_prev_music();
    }
    else if (app_db.audio_play_mode == MODE_APP_A2DP_SRC)
    {
        playback_play_prev_music();
    }
    else if (app_db.audio_play_mode == MODE_APP_A2DP_SNK)
    {
        uint8_t active_a2dp_idx = app_a2dp_get_active_idx();
        if ((app_db.br_link[active_a2dp_idx].connected_profile & A2DP_PROFILE_MASK) &&
            (app_db.br_link[active_a2dp_idx].connected_profile & AVRCP_PROFILE_MASK))
        {
            // Update play status after AVRCP play status event received
            bt_avrcp_backward(app_db.br_link[active_a2dp_idx].bd_addr);
        }
    }

    if (app_db.audio_play_mode != MODE_APP_A2DP_SNK)
    {
        uint16_t index = playback_get_cur_play_index();
        event_bus_publish(EVENT_BUS_TOPIC_AUDIO_PLAY_INDEX_UPDATE, &index, sizeof(uint16_t));
    }

    return res;
}

void app_audio_interface_init(void)
{
    if (app_audio_cfg.support_local_source)
    {
        if (!app_audio_fs_interface_init())
        {
            app_audio_mode_switch(MODE_APP_PLAYBACK);
            app_db.audio_fs_is_ready = true;
        }
    }
    else if (app_audio_cfg.support_sink)
    {
        app_audio_mode_switch(MODE_APP_A2DP_SNK);
    }
#if 0 // TODO: enable on ASIC
    internal_charger_register_cb(PLAYBACK_LOW_BATTERY_LEVEL, app_audio_low_batt_cb,
                                 PLAYBACK_HIGH_BATTERY_LEVEL, app_audio_high_batt_cb);
#endif
    event_bus_topic_register(EVENT_BUS_TOPIC_AUDIO_ALL_TOPIC);
}
