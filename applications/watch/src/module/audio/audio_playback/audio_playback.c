/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include <string.h>
#include "trace.h"
#include "os_mem.h"
#include "os_msg.h"
#include "os_task.h"
#include "audio_track.h"
#include "audio_type.h"
#include "btm.h"
#include "bt_avrcp.h"
#include "app_cfg.h"
#include "app_main.h"
#include "app_mmi.h"
#include "app_multilink.h"
#include "app_timer.h"
#include "audio.h"
#include "audio_playback.h"
#include "app_dlps.h"
#include "app_audio_if.h"
#include "app_eq.h"
#include "app_audio_policy.h"
#include "playback_playlist.h"

/*============================================================================*
 *                           Constants
 *============================================================================*/
#define PLAYBACK_FRAMES_NUM_2           5   //(PLAYBACK_FRAMES_NUM_2 + PLAYBACK_LOW_LEVEL) < LATENCY pkts
#define PLAYBACK_LOW_LEVEL              60  //ms
/* Max bit rate 320 kbps = 40 kB/s. */
/* Playback pool size time = playback pool size / 40960 * 1000 ms. */
/* If playback pool size = 10240 bytes, time = 250 ms, set 240 ms for safety. */
#define PLAYBACK_UPPER_LEVEL            150 //ms
#define PLAYBACK_LATENCY                200 //ms

#define PLAYBACK_PUT_DATA_PKT_NUM       4
// #define PLAYBACK_PUT_DATA_TIME_MS                   (PLAYBACK_PUT_DATA_PKT_NUM * 20)
//#define PLAYBACK_SINGLE_PREQ_PKTS(sf)           sf * (PLAYBACK_LATENCY + PLAYBACK_LOW_LEVEL) / 1000 / 1024 + 3
#define PLAYBACK_INTERVAL_TIMER_MS(pkt_num, sf)     pkt_num * 1000 * 1024 / sf

/*============================================================================*
 *                              Types
 *============================================================================*/

typedef enum
{
    APP_TIMER_PLAYBACK_PUT_DATA,
} T_APP_PLAYBACK_TIMER;

/*============================================================================*
 *                            Variables
 *============================================================================*/
static T_AUDIO_TRACK_HANDLE playback_track_handle;

static uint16_t g_playback_put_data_time_ms = 0;
static uint16_t g_playback_single_preq_pkts = 0;
static uint8_t timer_idx_playback_put_data = 0;
static uint8_t app_playback_time_id = 0;

Mp3Hdl_t g_curr_song;
T_APP_AUDIO_FS_DATA playback_db;

/*============================================================================*
 *                           Private Functions
 *============================================================================*/
static void playback_put_data_start_timer(uint16_t time_ms);
static void playback_get_audio_track_state_handle(void);


/**
 * @brief Reconfigures playback parameters according to the current MP3 stream.
 *
 * This routine parses MP3 metadata, updates playback timing parameters,
 * recreates the audio track, and applies EQ and latency configuration.
 *
 * @return Playback execution status.
 */
static uint8_t playback_parameter_recfg(void)
{
    ASSERT(g_curr_song != NULL);

    T_AUDIO_FORMAT_INFO format_info;
    format_info.frame_num = 1;
    format_info.type = AUDIO_FORMAT_TYPE_MP3;
    format_info.attr.mp3.sample_rate = Mp3_GetSamplingFrequency_Hz(g_curr_song);
    format_info.attr.mp3.bitrate = Mp3_GetBitRate_kbps(g_curr_song);
    format_info.attr.mp3.layer = Mp3_GetLayer(g_curr_song);
    format_info.attr.mp3.version = Mp3_GetVersion(g_curr_song);

    app_db.sampling_frequency = format_info.attr.mp3.sample_rate;
    switch (Mp3_GetChannelMode(g_curr_song))
    {
    case CHANNEL_STEREO:
    case CHANNEL_JOINT_STEREO:
    case CHANNEL_DOUBLE:
        format_info.attr.mp3.chann_mode = AUDIO_MP3_CHANNEL_MODE_DUAL;  // Stereo
        format_info.attr.mp3.chann_location = AUDIO_CHANNEL_LOCATION_FL | AUDIO_CHANNEL_LOCATION_FR;
        break;
    case CHANNEL_SINGLE:
        format_info.attr.mp3.chann_mode = AUDIO_MP3_CHANNEL_MODE_MONO;  // Mono
        format_info.attr.mp3.chann_location = AUDIO_CHANNEL_LOCATION_MONO;
        break;
    default:
        APP_PRINT_ERROR0("playback_parameter_recfg: channel not supprot");
        break;
    }

    APP_PRINT_TRACE5("playback_parameter_recfg: MP3, samplefrequency:%d, channel_mode:%d, "
                     "bitrate:%d, version: %d, layer:%d",
                     format_info.attr.mp3.sample_rate, format_info.attr.mp3.chann_mode,
                     format_info.attr.mp3.bitrate, format_info.attr.mp3.version,
                     format_info.attr.mp3.layer);

    g_playback_put_data_time_ms = PLAYBACK_PUT_DATA_PKT_NUM * Mp3_GetTimePerFrame_ms(g_curr_song);
    g_playback_single_preq_pkts = Mp3_GetSamplingFrequency_Hz(g_curr_song) *
                                  (PLAYBACK_LATENCY + PLAYBACK_LOW_LEVEL)
                                  / 1000 / Mp3_GetSamplePerFrame(g_curr_song);
    APP_PRINT_TRACE2("playback_parameter_recfg: g_playback_put_data_time_ms: %d, g_playback_single_preq_pkts: %d",
                     g_playback_put_data_time_ms, g_playback_single_preq_pkts);

    if (app_db.eq_instance != NULL)
    {
        eq_release(app_db.eq_instance);
        app_db.eq_instance = NULL;
    }

    playback_track_handle = audio_track_create(AUDIO_STREAM_TYPE_PLAYBACK, //stream_type
                                               AUDIO_STREAM_MODE_NORMAL, // mode
                                               AUDIO_STREAM_USAGE_SNOOP, // usage
                                               format_info, //format_info
                                               playback_db.volume, //volume
                                               0,
                                               AUDIO_DEVICE_OUT_SPK, // device
                                               NULL,
                                               NULL);
    playback_get_audio_track_state_handle();

    if (playback_track_handle != NULL)
    {
        app_db.eq_instance = app_eq_create(EQ_CONTENT_TYPE_AUDIO, SPK_SW_EQ,
                                           app_db.spk_eq_mode, app_cfg_nv.eq_idx);

        app_db.audio_eq_enabled = false;
        if (app_db.eq_instance != NULL)
        {
            app_eq_audio_eq_enable(&app_db.eq_instance, &app_db.audio_eq_enabled);

            audio_track_effect_attach(playback_track_handle, app_db.eq_instance);
        }

        audio_track_latency_set(playback_track_handle, PLAYBACK_LATENCY, true);
        audio_track_threshold_set(playback_track_handle, PLAYBACK_UPPER_LEVEL, PLAYBACK_LOW_LEVEL);
    }

    return APP_AUDIO_SUCCESS;
}

/**
 * @brief Reads MP3 frames and writes them into the playback track buffer.
 *
 * @param[in] pkt_num Number of frames to push during this refill cycle.
 */
static void playback_put_data(uint8_t pkt_num)
{
    uint16_t buf_level_ms;
    if (audio_track_buffer_level_get(playback_track_handle, &buf_level_ms))
    {

        APP_PRINT_TRACE2("playback_put_data: pkt_num: %d, buffer_level = %d ms",
                         pkt_num, buf_level_ms);
        // buf_level_after_write >= PLAYBACK_UPPER_LEVEL
        while (buf_level_ms + g_playback_put_data_time_ms / PLAYBACK_PUT_DATA_PKT_NUM * pkt_num
               >= PLAYBACK_UPPER_LEVEL)
        {
            --pkt_num;
            APP_PRINT_TRACE1("playback_put_data: buffer level high, pkt_num = %d", pkt_num);
        }
    }

    for (uint8_t i = 0; i < pkt_num; ++i)
    {
        uint8_t *pbuf; uint32_t buf_len; float pos_ms;
        EMp3Res res = Mp3_ReadNextFrame(g_curr_song, &pbuf, &buf_len, &pos_ms);
        if (res != MP3RES_OK) // MP3RES_FILE_ENDS
        {
            APP_PRINT_ERROR1("playback_put_data ERROR,RES:0x%x", res);
            APP_PRINT_WARN0("playback_buffer_low_handle,file end, and paly next song!!!");
            audio_playback_stop();
            playback_db.op_next_action = APP_AUDIO_STOPPED_FILE_END_TO_NEXT_ACTION;
            return;
        }

        uint16_t written_len;
        playback_db.seq_num++;
        if (audio_track_write(playback_track_handle,
                              0, // timestamp,
                              playback_db.seq_num,
                              AUDIO_STREAM_STATUS_CORRECT,
                              1, // frame_num,
                              pbuf,
                              buf_len,
                              &written_len) == false)
        {
            break;
        }
    }

    if (playback_db.sd_play_state == APP_AUDIO_STATE_PLAY)
    {
        // This maybe AUDIO_EVENT_TRACK_BUFFER_HIGH event
        uint16_t time_ms = (playback_db.local_buffer_state == APP_AUDIO_FS_BUF_HIGH) ?
                           g_playback_put_data_time_ms * 2 : g_playback_put_data_time_ms;
        APP_PRINT_INFO2("time_ms = %d, g_playback_put_data_time_ms = %d", time_ms,
                        g_playback_put_data_time_ms);
        playback_put_data_start_timer(time_ms);
    }
    playback_db.local_buffer_state = APP_AUDIO_FS_BUF_NORMAL;
}

/**
 * @brief Starts the timer used to schedule the next playback refill.
 *
 * @param[in] time_ms Timer period in milliseconds.
 */
static void playback_put_data_start_timer(uint16_t time_ms)
{
    app_start_timer(&timer_idx_playback_put_data, "playback_put_data",
                    app_playback_time_id, APP_TIMER_PLAYBACK_PUT_DATA, 0, false,
                    time_ms);
}

/**
 * @brief Stops the refill timer and pushes more data when playback is active.
 *
 * @param[in] pkt_num Number of frames to request immediately.
 */
static void playback_put_data_stop_timer(uint8_t pkt_num)
{
    APP_PRINT_TRACE0("playback_put_data_stop_timer");

    app_stop_timer(&timer_idx_playback_put_data);
    if (playback_db.sd_play_state != APP_AUDIO_STATE_PLAY)
    {
        return;
    }
    playback_put_data(pkt_num);
}

/**
 * @brief Handles playback timer events.
 *
 * @param[in] timer_evt Timer event identifier.
 * @param[in] param Reserved timer parameter.
 */
static void playback_timeout_cb(uint8_t timer_evt, uint16_t param)
{
    switch (timer_evt)
    {
    case APP_TIMER_PLAYBACK_PUT_DATA:
        {
            playback_put_data_stop_timer(PLAYBACK_PUT_DATA_PKT_NUM);
        }
        break;

    default:
        break;
    }
}

/**
 * @brief Handles low-buffer events by requesting extra audio data.
 */
static void playback_buffer_low_handle(void)
{
    playback_db.local_buffer_state = APP_AUDIO_FS_BUF_LOW;
    playback_put_data_stop_timer(PLAYBACK_PUT_DATA_PKT_NUM + 2);

}

/**
 * @brief Marks the local playback buffer as high.
 */
static void playback_buffer_high_handle(void)
{
//    app_stop_timer(&timer_idx_playback_put_data);
    playback_db.local_buffer_state = APP_AUDIO_FS_BUF_HIGH;
}

/**
 * @brief Refreshes the cached audio track state from the audio subsystem.
 */
static void playback_get_audio_track_state_handle(void)
{
    audio_track_state_get(playback_track_handle, (T_AUDIO_TRACK_STATE *)&playback_db.track_state);
    APP_PRINT_TRACE1("playback_get_audio_track_state_handle, track_state:%d",
                     playback_db.track_state);
}

/**
 * @brief Handles playback behavior after the audio track state changes.
 */
static void playback_track_state_changed_handle(void)
{
    APP_PRINT_TRACE3("playback_track_state_changed_handle, track_state:%d, bud_role:%d, connect:%d",
                     playback_db.track_state, app_cfg_nv.bud_role, remote_session_state_get());
    if (AUDIO_TRACK_STATE_STARTED == playback_db.track_state)
    {
        playback_put_data(g_playback_single_preq_pkts);
    }
    else if (AUDIO_TRACK_STATE_RELEASED == playback_db.track_state)
    {
        playback_track_handle = NULL;
        if (playback_db.op_next_action == APP_AUDIO_STOPPED_SWITCH_BY_NAME)
        {
            playback_db.op_next_action = APP_AUDIO_STOPPED_IDLE_ACTION;
            app_audio_play_by_name(playback_db.file_name, playback_db.name_length);
        }
        else if (playback_db.op_next_action == APP_AUDIO_STOPPED_FILE_END_TO_NEXT_ACTION)
        {
            playback_db.op_next_action = APP_AUDIO_STOPPED_IDLE_ACTION;
            playback_play_next_music();
        }
    }
}

/**
 * @brief Handles audio policy events related to the playback track.
 *
 * @param[in] event_type Audio event type.
 * @param[in] event_buf Event payload buffer.
 * @param[in] buf_len Event payload length.
 */
static void playback_audio_policy_cback(T_AUDIO_EVENT event_type, void *event_buf,
                                        uint16_t buf_len)
{
    T_AUDIO_EVENT_PARAM *param = event_buf;
    bool handle = true;

    if (param->track_state_changed.handle != playback_track_handle)
    {
        return;
    }

    switch (event_type)
    {
    case AUDIO_EVENT_TRACK_STATE_CHANGED:
        {
            playback_db.track_state = param->track_state_changed.state;
            playback_get_audio_track_state_handle();
            playback_track_state_changed_handle();
        }
        break;

    case AUDIO_EVENT_TRACK_BUFFER_LOW:
        {
            if (playback_db.sd_play_state == APP_AUDIO_STATE_PLAY)
            {
                playback_buffer_low_handle();
            }
        }
        break;

    case AUDIO_EVENT_TRACK_BUFFER_HIGH:
        {
            if (playback_db.sd_play_state == APP_AUDIO_STATE_PLAY)
            {
                playback_buffer_high_handle();
            }
        }
        break;

    default:
        handle = false;
        break;
    }

    if (handle == true)
    {
        APP_PRINT_TRACE1("playback_audio_policy_cback: event_type 0x%04x", event_type);
    }
}

/**
 * @brief Applies the cached playback volume to the audio track.
 */

static void playback_volume_set(void)
{
    APP_PRINT_TRACE1("playback_volume_set vol:%d", playback_db.volume);
    audio_track_volume_out_set(playback_track_handle, playback_db.volume);
}

/**
 * @brief Releases the current audio track and attached EQ resources.
 */
static void playback_audio_track_release(void)
{
    if (playback_track_handle != NULL)
    {
        playback_db.track_state = PLAYBACK_TRACK_STATE_CLEAR;
        audio_track_release(playback_track_handle);
    }

    if (app_db.eq_instance != NULL)
    {
        eq_release(app_db.eq_instance);
        app_db.eq_instance = NULL;
    }
}


/*============================================================================*
 *                           Public Functions
 *============================================================================*/

/**
 * @brief Starts local playback for the currently opened song.
 *
 * @return Playback execution status.
 */
uint8_t audio_playback_start(void)
{
    uint8_t res = APP_AUDIO_SUCCESS;

    APP_PRINT_TRACE0("audio_playback_start ++");
    app_dlps_disable(APP_DLPS_ENTER_CHECK_LOCAL_PLAYBACK);

    if ((res = playback_parameter_recfg()) != 0)
    {
        return res;
    }
    playback_volume_set();
    playback_db.sd_play_state = APP_AUDIO_STATE_PLAY;
    playback_db.op_next_action = APP_AUDIO_STOPPED_IDLE_ACTION;
    playback_db.seq_num = 0;
    playback_db.track_state = PLAYBACK_TRACK_STATE_CLEAR;
    playback_db.local_buffer_state = APP_AUDIO_FS_BUF_NORMAL;
    audio_track_start(playback_track_handle);
    //gui_update_by_event(GUI_EVENT_PLAYER, NULL, false);

    return res;
}

void audio_playback_volume_up(void)
{
    if (playback_db.volume < app_cfg_volume.playback_volume_max)
    {
        playback_db.volume++;
    }
    else
    {
        playback_db.volume = app_cfg_volume.playback_volume_max;
    }

    APP_PRINT_TRACE2("audio_playback_volume_up,volume:%d,max:%d", playback_db.volume,
                     app_cfg_volume.playback_volume_max);

    if (playback_track_handle != NULL)
    {
        audio_track_volume_out_set(playback_track_handle, playback_db.volume);
    }
    //gui_update_by_event(GUI_EVENT_PLAYER, NULL, false);
}

void audio_playback_volume_down(void)
{
    if (playback_db.volume > app_cfg_volume.playback_volume_min)
    {
        playback_db.volume--;
    }
    else
    {
        playback_db.volume = app_cfg_volume.playback_volume_min;
    }

    APP_PRINT_TRACE2("audio_playback_volume_down,volume:%d,min:%d", playback_db.volume,
                     app_cfg_volume.playback_volume_min);

    if (playback_track_handle != NULL)
    {
        audio_track_volume_out_set(playback_track_handle, playback_db.volume);
    }
    //gui_update_by_event(GUI_EVENT_PLAYER, NULL, false);
}

uint8_t audio_playback_stop(void)
{
    uint8_t res = APP_AUDIO_SUCCESS;

    app_stop_timer(&timer_idx_playback_put_data);
    if (playback_db.sd_play_state == APP_AUDIO_STATE_PLAY)
    {
        playback_db.sd_play_state = APP_AUDIO_STATE_PAUSE;
    }

    playback_audio_track_release();

    app_dlps_enable(APP_DLPS_ENTER_CHECK_LOCAL_PLAYBACK);
    //gui_update_by_event(GUI_EVENT_PLAYER, NULL, false);

    return res;
}

uint8_t audio_playback_close_flie(void)
{
    if (g_curr_song != NULL)
    {
        Mp3_FreeHandle(g_curr_song);
        g_curr_song = NULL;
        return APP_AUDIO_SUCCESS;
    }
    else
    {
        return APP_AUDIO_CLOSE_FILE_ERROR;
    }
}

bool playback_get_source_song_time(Mp3Hdl_t curr_song, uint32_t *pos_time_ms,
                                   uint32_t *total_time_ms)
{
    if (g_curr_song != NULL)
    {
        *pos_time_ms = curr_song->uReadSqnNum * (uint32_t)curr_song->fTimePerFrm_ms;
        *total_time_ms = (uint32_t)Mp3_GetTotalPlayTime_ms(curr_song);
        return true;
    }
    else
    {
        *pos_time_ms = 0;
        *total_time_ms = 0;
        return false;
    }
}

void audio_playback_init(void)
{
    memset(&playback_db, 0, sizeof(T_APP_AUDIO_FS_DATA));
    playback_db.volume = 8;
    audio_mgr_cback_register(playback_audio_policy_cback);
    app_timer_reg_cb(playback_timeout_cb, &app_playback_time_id);
}
