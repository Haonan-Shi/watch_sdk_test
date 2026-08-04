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
#include "app_a2dp.h"
#include "audio_type.h"
#include "app_task.h"
#include "os_sync.h"
#include "app_mmi.h"
#include "app_dlps.h"
#include "bt_hfp.h"
#include "bt_bond.h"
#include "hw_tim.h"
#include "app_msg_handle.h"
#include "bt_types.h"
#include "app_a2dp_enc.h"
#include "audio_pipe.h"
#include "app_audio_if.h"
#include "app_audio_policy.h"
#include "audio_a2dp_src.h"
#include "app_avrcp.h"
#include "stdlib.h"
#include "playback_playlist.h"

T_APP_AUDIO_FS_DATA   a2dp_src_db;
void *p_snk_data_buf = NULL;
static T_AUDIO_PIPE_HANDLE audio_pipe_handle = NULL;

static const uint16_t a2dp_gain_table[] =
{
    0x8001, 0xeb00, 0xec80, 0xee00, 0xef80, 0xf100, 0xf280, 0xf400,
    0xf580, 0xf700, 0xf880, 0xfa00, 0xfb80, 0xfd00, 0xfe80, 0x0000
};
static uint8_t  cur_pair_idx = 0;
static const uint8_t  min_gain_level = 0;
static const uint8_t  max_gain_level = 15;
static uint8_t  a2dp_src_bitpool = 0x22;
static float    a2dp_src_timer = 21.333;
static float    a2dp_sbc_time = 2.6666;
static bool     dsp_tx_ack = false;

static bool adjust_audio_pipe_vol_flag = false;
extern bool watch_supports_abs_vol_flag;

static uint32_t src_timestamp = 0;
static uint16_t src_seq_num = 0;

uint32_t pre_timestamp = 0;
uint32_t cur_timestamp = 0;
int32_t timestamp_total = 0;
int32_t hw_period = 0;
T_HW_TIMER_HANDLE a2dp_timer_handle = NULL;
extern uint32_t sys_timestamp_get_from_hw_timer_us(void);

static void audio_a2dp_src_set_pipe_vol_0db(void)
{
    int16_t left_gain = 0, right_gain = 0;
    bool res = audio_pipe_gain_get(audio_pipe_handle, &left_gain, &right_gain);
    APP_PRINT_INFO3("audio_pipe_gain_get:res= %d, left = 0x%x, right = 0x%x", res, left_gain,
                    right_gain);
    if (res == true && (left_gain != 0 || right_gain != 0))
    {
        audio_pipe_gain_set(audio_pipe_handle, 0, 0);
        adjust_audio_pipe_vol_flag = false;
    }
}

/* send MP3 data to dsp, get SBC data */
static bool audio_a2dp_src_codec_callback(T_AUDIO_PIPE_HANDLE handle, T_AUDIO_PIPE_EVENT  event,
                                          uint32_t  param)
{
    //APP_PRINT_TRACE1("carol - audio_codec_callback,event: %x", event);
    T_IO_MSG a2dp_msg;
    bool res = true;

    a2dp_msg.type = IO_MSG_TYPE_A2DP_SRC;
    a2dp_msg.subtype = event;
    a2dp_msg.u.param = param;
    if (app_send_msg_to_apptask(&a2dp_msg) == false)
    {
        APP_PRINT_ERROR0("audio_codec_callback msg send fail");
        res = false;
    }
    return res;
}

static uint16_t audio_a2dp_src_get_header_info_from_fs(void)
{
    ASSERT(g_curr_song != NULL);

    T_AUDIO_FORMAT_INFO src_info;
    src_info.frame_num = 1;
    src_info.type = AUDIO_FORMAT_TYPE_MP3;
    src_info.attr.mp3.sample_rate = Mp3_GetSamplingFrequency_Hz(g_curr_song);
    src_info.attr.mp3.bitrate = Mp3_GetBitRate_kbps(g_curr_song) * 1000;
    src_info.attr.mp3.layer = Mp3_GetLayer(g_curr_song);
    src_info.attr.mp3.version = Mp3_GetVersion(g_curr_song);
    switch (Mp3_GetChannelMode(g_curr_song))
    {
    case CHANNEL_STEREO:
    case CHANNEL_JOINT_STEREO:
    case CHANNEL_DOUBLE:
        src_info.attr.mp3.chann_mode = AUDIO_MP3_CHANNEL_MODE_DUAL;  // Stereo
        src_info.attr.mp3.chann_location = AUDIO_CHANNEL_LOCATION_FL | AUDIO_CHANNEL_LOCATION_FR;
        break;
    case CHANNEL_SINGLE:
        src_info.attr.mp3.chann_mode = AUDIO_MP3_CHANNEL_MODE_MONO;  // Mono
        src_info.attr.mp3.chann_location = AUDIO_CHANNEL_LOCATION_MONO;
        break;
    default:
        APP_PRINT_ERROR0("audio_a2dp_src_get_header_info_from_fs: channel not supprot");
        break;
    }
    APP_PRINT_TRACE2("audio_a2dp_src_get_header_info_from_fs: MP3, sample_rate:%d, channel_mode:%d",
                     src_info.attr.mp3.sample_rate, src_info.attr.mp3.chann_mode);

    T_AUDIO_FORMAT_INFO snk_info;
    snk_info.frame_num = 6;
    snk_info.type = AUDIO_FORMAT_TYPE_SBC;
    snk_info.attr.sbc.subband_num = 8;
    snk_info.attr.sbc.bitpool = a2dp_src_bitpool;          //change to be same with min bitpool
    snk_info.attr.sbc.sample_rate = 48000;
    snk_info.attr.sbc.block_length = 16;
    snk_info.attr.sbc.chann_mode = AUDIO_SBC_CHANNEL_MODE_JOINT_STEREO;
    snk_info.attr.sbc.allocation_method = 0;
    snk_info.attr.sbc.chann_location = AUDIO_CHANNEL_LOCATION_FL | AUDIO_CHANNEL_LOCATION_FR;

    float sbc_block = snk_info.attr.sbc.block_length;
    float sbc_subband = snk_info.attr.sbc.subband_num;
    a2dp_sbc_time = (sbc_block * sbc_subband) / 48;
    if (audio_pipe_handle == NULL)
    {
        if (app_avrcp_get_abs_vol_support() && watch_supports_abs_vol_flag)
        {
            audio_pipe_handle = audio_pipe_create(AUDIO_STREAM_MODE_NORMAL, src_info, snk_info,
                                                  0,
                                                  audio_a2dp_src_codec_callback);
            APP_PRINT_TRACE0("abs vol is supported, audio pipe gain is 0db");
        }
        else
        {
            audio_pipe_handle = audio_pipe_create(AUDIO_STREAM_MODE_NORMAL, src_info, snk_info,
                                                  a2dp_gain_table[app_cfg_nv.audio_gain_level[cur_pair_idx]],
                                                  audio_a2dp_src_codec_callback);
            if (watch_supports_abs_vol_flag)
            {
                adjust_audio_pipe_vol_flag = true;
            }
        }
    }

    return APP_AUDIO_SUCCESS;
}

static uint16_t audio_a2dp_src_get_data_from_fs(void)
{
    static uint8_t seq_num = 0;
    if (g_curr_song == NULL || audio_pipe_handle == NULL)
    {
        APP_PRINT_WARN2("audio_a2dp_src_get_data_from_fs, NULL handle. g_curr_song: %p, audio_pipe_handle: %p",
                        g_curr_song, audio_pipe_handle);
        return APP_AUDIO_SUCCESS;
    }

    uint8_t *pbuf; uint32_t buf_len; float pos_ms;
    EMp3Res res = Mp3_ReadNextFrame(g_curr_song, &pbuf, &buf_len, &pos_ms);
    if (res != MP3RES_OK)
    {
        APP_PRINT_WARN0("audio_a2dp_src_get_data_from_fs, file end, and paly next song!!!");
        playback_db.op_next_action = APP_AUDIO_STOPPED_FILE_END_TO_NEXT_ACTION;
        audio_a2dp_src_stop();
        return APP_AUDIO_END_OF_FILE;
    }

    APP_PRINT_TRACE2("audio_a2dp_src_get_data_from_fs. frame length: 0x%x, pos: %u ms", buf_len,
                     (uint32_t)pos_ms);
    if (!audio_pipe_fill(audio_pipe_handle, 0, seq_num, AUDIO_STREAM_STATUS_CORRECT, 1, pbuf, buf_len))
    {
        return APP_AUDIO_PIPE_FILL_ERROR;
    }
    else
    {
        seq_num++;
    }

    return APP_AUDIO_SUCCESS;
}

static uint16_t audio_a2dp_src_data_ind(void)
{
    uint16_t res = APP_AUDIO_SUCCESS;
    uint16_t data_len = 0;
    uint32_t timestamp = 0;
    uint16_t seq = 0;
    T_AUDIO_STREAM_STATUS status;
    uint8_t  frame_number = 0;

    audio_pipe_drain(audio_pipe_handle,
                     &timestamp,
                     &seq,
                     &status,
                     &frame_number,
                     p_snk_data_buf,
                     &data_len);
    if (data_len == 0)
    {
        res = APP_AUDIO_PIPE_DRAIN_ERROR;
    }
    else
    {
        if (!a2dp_enc_data_rx((uint8_t *)p_snk_data_buf, data_len, frame_number))
        {
            res = APP_AUDIO_A2DP_ENC_RX_ERROR;
        }
        if ((a2dp_enc_queue.count >= A2DP_ENC_WATER_LEVEL_H) &&
            (playback_db.source_buffer_state == APP_AUDIO_FS_BUF_LOW))
        {
            bool timer_active = false;
            hw_timer_is_active(a2dp_timer_handle, &timer_active);
            if (timer_active == false)
            {
                T_A2DP_ENC_MEIDAHEAD *da_pkt;
                da_pkt = a2dp_enc_audio_peek(0);
                if (da_pkt != NULL)
                {
                    pre_timestamp = sys_timestamp_get_from_hw_timer_us();
                    a2dp_src_timer = (float)da_pkt->frame_num * a2dp_sbc_time;
                    hw_timer_restart(a2dp_timer_handle, a2dp_src_timer * 1000);
                    hw_period = a2dp_src_timer * 1000;
                    T_IO_MSG gpio_msg;
                    gpio_msg.type = IO_MSG_TYPE_A2DP_SRC;
                    gpio_msg.subtype = AUDIO_A2DP_SRC_EVENT_DATA_SEND;
                    if (app_send_msg_to_apptask(&gpio_msg) == false)
                    {
                        APP_PRINT_ERROR0("a2dp hw timer: msg send fail");
                    }
                }
            }
            playback_db.source_buffer_state = APP_AUDIO_FS_BUF_HIGH;
        }
    }
    APP_PRINT_TRACE5("audio_a2dp_src_data_ind: res:0x%x, data_len: 0x%x, frame_number: 0x%x, source_buffer_state: 0x%x, enc_sbc_count: 0x%x,",
                     res, data_len, frame_number, playback_db.source_buffer_state, a2dp_enc_queue.count);
    return res;
}

static uint16_t audio_a2dp_src_fill_data(void)
{
    uint16_t res = APP_AUDIO_SUCCESS;
    if (playback_db.source_buffer_state == APP_AUDIO_FS_BUF_LOW)
    {
        res = audio_a2dp_src_get_data_from_fs();
    }
    else
    {
        dsp_tx_ack = true;
    }
    APP_PRINT_TRACE3("audio_a2dp_src_fill_data: res:0x%x, source_buffer_state:0x%x, dsp_tx_ack:0x%x",
                     res,
                     playback_db.source_buffer_state, dsp_tx_ack);
    return res;
}

static uint16_t audio_a2dp_src_send_data(void)
{
    uint16_t res = APP_AUDIO_SUCCESS;

    if (app_db.a2dp_src_state != APP_A2DP_SRC_STREAM_START)
    {
        return APP_AUDIO_MODE_ERROR;
    }

    T_A2DP_ENC_MEIDAHEAD *da_pkt;
    da_pkt = a2dp_enc_audio_peek(0);
    if (da_pkt == NULL)
    {
        if (dsp_tx_ack)
        {
            dsp_tx_ack = false;
            res = audio_a2dp_src_get_data_from_fs();
        }
        else
        {
            APP_PRINT_ERROR0("The dsp_tx_ack is not received");
        }
    }
    else
    {

        uint8_t a2dp_idx = app_a2dp_get_active_idx();
        uint8_t a2dp_credits = app_a2dp_get_src_credits();
        APP_PRINT_INFO1("audio_a2dp_src_send_data: src_a2dp_credits = %d", a2dp_credits);

        if (a2dp_credits)
        {
            if (bt_a2dp_stream_data_send(app_db.br_link[a2dp_idx].bd_addr, src_seq_num, src_timestamp,
                                         (uint8_t)da_pkt->frame_num, da_pkt->p_data,
                                         da_pkt->payload_length, false))
            {
                app_a2dp_use_src_credits();
            }
            else
            {
                res = APP_AUDIO_A2DP_DATA_SEND;
            }
        }
        else
        {
            APP_PRINT_TRACE0("audio_a2dp_src_send_data no reason need to send");
        }

        src_timestamp += da_pkt->frame_num * 128;
        src_seq_num++;
        if (a2dp_enc_queue.count > 1)
        {
            a2dp_enc_audio_flush(1);
        }
        else
        {
            APP_PRINT_TRACE0("audio_a2dp_src_send_data queue count 2");
        }
        if (a2dp_enc_queue.count <= A2DP_ENC_WATER_LEVEL_L && dsp_tx_ack)
        {
            dsp_tx_ack = false;
            res = audio_a2dp_src_get_data_from_fs();
        }
    }
    APP_PRINT_TRACE4("audio_a2dp_src_send_data, res:0x%x, da_pkt:0x%x, dsp_tx_ack:0x%x, frameCount:0x%x",
                     res, da_pkt, dsp_tx_ack, a2dp_enc_queue.count);
    return res;
}

void audio_a2dp_src_handle_msg(T_IO_MSG *msg)
{
    uint16_t subtype = msg->subtype;

    APP_PRINT_TRACE1("audio_a2dp_src_handle_msg: subtype: (0x%x)", subtype);
    switch (subtype)
    {
    case AUDIO_PIPE_EVENT_CREATED:
        {
            //uint32_t snk_buf_size = msg->u.param;
            uint32_t snk_buf_size = 1024; // crb14 app need set this para as 1024
            app_audio_pipe_chann_set(audio_pipe_handle);
            audio_pipe_start(audio_pipe_handle);

            p_snk_data_buf = malloc(snk_buf_size);
            app_db.audio_pipe_create = true;
            audio_a2dp_src_play_state_handle(AUDIO_A2DP_SRC_PIPE_CREATE);
        }
        break;

    case AUDIO_PIPE_EVENT_STARTED:
        {
            audio_a2dp_src_get_data_from_fs();
        }
        break;

    case AUDIO_PIPE_EVENT_DATA_IND:
        {
            if (app_avrcp_get_abs_vol_support() && watch_supports_abs_vol_flag && adjust_audio_pipe_vol_flag)
            {
                audio_a2dp_src_set_pipe_vol_0db();
            }
            if (playback_db.sd_play_state == APP_AUDIO_STATE_PLAY)
            {
                audio_a2dp_src_data_ind();
            }
        }
        break;

    case AUDIO_PIPE_EVENT_DATA_FILLED:
        {
            if (playback_db.sd_play_state == APP_AUDIO_STATE_PLAY)
            {
                audio_a2dp_src_fill_data();
            }
        }
        break;

    case AUDIO_PIPE_EVENT_RELEASED:
        {
            if (p_snk_data_buf != NULL)
            {
                free(p_snk_data_buf);
                p_snk_data_buf = NULL;
            }
            app_db.audio_pipe_create = false;
            audio_a2dp_src_play_state_handle(AUDIO_A2DP_SRC_PIPE_RELEASE);

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
        break;

    case AUDIO_A2DP_SRC_EVENT_DATA_SEND:
        {
            audio_a2dp_src_send_data();
        }
        break;

    default:
        break;
    }
}

void audio_a2dp_src_hw_timer_callback(T_HW_TIMER_HANDLE handle)
{
    T_A2DP_ENC_MEIDAHEAD *da_pkt;
    da_pkt = a2dp_enc_audio_peek(0);
    if (da_pkt != NULL)
    {
        cur_timestamp = sys_timestamp_get_from_hw_timer_us();
        if (cur_timestamp > pre_timestamp)
        {
            timestamp_total = cur_timestamp - pre_timestamp;
        }
        else
        {
            timestamp_total = 0xFFFFFFFF - pre_timestamp + cur_timestamp;
        }
        int32_t diff_time = timestamp_total - hw_period;
        a2dp_src_timer = (float)da_pkt->frame_num * a2dp_sbc_time;
        uint32_t restart_time = (uint32_t)(a2dp_src_timer * 1000) - diff_time;
        hw_timer_restart(a2dp_timer_handle, a2dp_src_timer * 1000 - diff_time);

        APP_PRINT_INFO7("a2dp hw timer: pre timestamp = %u, cur timestamp = %u, total timestamp = %d, hw period = %d, diff_time_us %d, a2dp_src_timer %d, restart_time %d",
                        \
                        pre_timestamp, cur_timestamp, timestamp_total, hw_period, diff_time, a2dp_src_timer, restart_time);

        if (hw_period >= 30 * 60 * 1000 * 1000)
        {
            hw_period -= 30 * 60 * 1000 * 1000;
            pre_timestamp += 30 * 60 * 1000 * 1000;
        }
        hw_period += a2dp_src_timer * 1000;

        T_IO_MSG a2dp_src_msg;
        a2dp_src_msg.type = IO_MSG_TYPE_A2DP_SRC;
        a2dp_src_msg.subtype = AUDIO_A2DP_SRC_EVENT_DATA_SEND;
        if (app_send_msg_to_apptask(&a2dp_src_msg) == false)
        {
            APP_PRINT_ERROR0("a2dp hw timer: msg send fail");
        }
    }
}

void audio_a2dp_src_hw_timer_init(void)
{
    if (a2dp_timer_handle == NULL)
    {
        a2dp_timer_handle = hw_timer_create("a2dp_hw_timer", 21333, true, audio_a2dp_src_hw_timer_callback);
        if (a2dp_timer_handle == NULL)
        {
            DBG_DIRECT("fail to create a2dp hw timer, check hw timer usage!!");
        }
        else
        {
            DBG_DIRECT("create a2dp hw timer instance successfully, id %d",
                       hw_timer_get_id(a2dp_timer_handle));
        }
    }
}


void audio_a2dp_src_volume_up(void)
{
    if (app_cfg_nv.audio_gain_level[cur_pair_idx] < max_gain_level)
    {
        app_cfg_nv.audio_gain_level[cur_pair_idx]++;
    }
    else
    {
        app_cfg_nv.audio_gain_level[cur_pair_idx] = max_gain_level;
    }

    APP_PRINT_TRACE2("audio_a2dp_src_volume_up,volume:%d,max:%d",
                     app_cfg_nv.audio_gain_level[cur_pair_idx], max_gain_level);

    if (app_avrcp_get_abs_vol_support() && watch_supports_abs_vol_flag)
    {
        uint8_t vol = app_cfg_nv.audio_gain_level[cur_pair_idx] * 0x7F /
                      app_cfg_volume.playback_volume_max;
        uint8_t a2dp_idx = app_a2dp_get_active_idx();
        bt_avrcp_absolute_volume_set(app_db.br_link[a2dp_idx].bd_addr, vol);
    }
    else if (audio_pipe_handle != NULL)
    {
        audio_pipe_gain_set(audio_pipe_handle, a2dp_gain_table[app_cfg_nv.audio_gain_level[cur_pair_idx]],
                            a2dp_gain_table[app_cfg_nv.audio_gain_level[cur_pair_idx]]);
    }
    //gui_update_by_event(GUI_EVENT_PLAYER, NULL, false);
}

void audio_a2dp_src_volume_down(void)
{
    if (app_cfg_nv.audio_gain_level[cur_pair_idx] > min_gain_level)
    {
        app_cfg_nv.audio_gain_level[cur_pair_idx]--;
    }
    else
    {
        app_cfg_nv.audio_gain_level[cur_pair_idx] = min_gain_level;
    }

    APP_PRINT_TRACE2("audio_a2dp_src_volume_down,volume:%d,min:%d",
                     app_cfg_nv.audio_gain_level[cur_pair_idx], min_gain_level);

    if (app_avrcp_get_abs_vol_support() && watch_supports_abs_vol_flag)
    {
        uint8_t vol = app_cfg_nv.audio_gain_level[cur_pair_idx] * 0x7F /
                      app_cfg_volume.playback_volume_max;
        uint8_t a2dp_idx = app_a2dp_get_active_idx();
        bt_avrcp_absolute_volume_set(app_db.br_link[a2dp_idx].bd_addr, vol);
    }
    else if (audio_pipe_handle != NULL)
    {
        audio_pipe_gain_set(audio_pipe_handle, a2dp_gain_table[app_cfg_nv.audio_gain_level[cur_pair_idx]],
                            a2dp_gain_table[app_cfg_nv.audio_gain_level[cur_pair_idx]]);
    }
    //gui_update_by_event(GUI_EVENT_PLAYER, NULL, false);
}

void audio_a2dp_src_set_volume(uint8_t volume)
{
    if (volume > max_gain_level)
    {
        volume = max_gain_level;
    }

    APP_PRINT_TRACE3("audio_a2dp_src_set_volume,volume:%d,min:%d,max:%d", volume, min_gain_level,
                     max_gain_level);

    app_cfg_nv.audio_gain_level[cur_pair_idx] = volume;
    //gui_update_by_event(GUI_EVENT_PLAYER, NULL, false);
}

void audio_a2dp_src_stop(void)
{
    if ((app_db.a2dp_src_state == APP_A2DP_SRC_STREAM_START ||
         app_db.a2dp_src_state == APP_A2DP_SRC_DISCONN) &&
        (playback_db.op_next_action == APP_AUDIO_STOPPED_IDLE_ACTION))
    {
        hw_timer_stop(a2dp_timer_handle);
        uint8_t a2dp_idx = app_a2dp_get_active_idx();
        bt_a2dp_stream_suspend_req(app_db.br_link[a2dp_idx].bd_addr);
        a2dp_enc_audio_flush(a2dp_enc_queue.count);
        os_queue_init(&a2dp_enc_queue);
        playback_db.source_buffer_state = APP_AUDIO_FS_BUF_LOW;
    }

    audio_a2dp_src_play_state_handle(AUDIO_A2DP_SRC_STOP);

    if (audio_pipe_handle != NULL)
    {
        audio_pipe_release(audio_pipe_handle);
        audio_pipe_handle = NULL;
    }
    app_dlps_enable(APP_DLPS_ENTER_CHECK_A2DP);
}

void audio_a2dp_src_start(void)
{
    APP_PRINT_TRACE0("audio_a2dp_src_start ++");
    if ((playback_db.sd_play_state != APP_AUDIO_STATE_STOP) && \
        (playback_db.sd_play_state != APP_AUDIO_STATE_PAUSE))
    {
        APP_PRINT_ERROR0("audio_a2dp_src_start play status error!");
        return;
    }

    audio_a2dp_src_play_state_handle(AUDIO_A2DP_SRC_START);
    playback_db.op_next_action = APP_AUDIO_STOPPED_IDLE_ACTION;
    if (app_db.a2dp_src_state != APP_A2DP_SRC_STREAM_START)
    {
        uint8_t a2dp_idx = app_a2dp_get_active_idx();
        bt_a2dp_stream_start_req(app_db.br_link[a2dp_idx].bd_addr);
    }
    else
    {
        audio_a2dp_src_pipe_create();
    }
}

void audio_a2dp_src_pipe_create(void)
{
    if (audio_a2dp_src_get_header_info_from_fs() == APP_AUDIO_SUCCESS)
    {
        app_dlps_disable(APP_DLPS_ENTER_CHECK_A2DP);
    }
    else
    {
        APP_PRINT_TRACE0("audio_a2dp_src_start fail, get mp3 header fail");
    }
}

void audio_a2dp_src_init(void)
{
    src_timestamp = 0;
    src_seq_num = 0;
    uint8_t a2dp_idx = app_a2dp_get_active_idx();
    if (bt_bond_index_get(app_db.br_link[a2dp_idx].bd_addr, &cur_pair_idx) == false)
    {
        APP_PRINT_ERROR0("audio_a2dp_src_init, get pair idx fail");
    }
    APP_PRINT_INFO2("pair idx = %d, vol level = %d", cur_pair_idx,
                    app_cfg_nv.audio_gain_level[cur_pair_idx]);

    audio_a2dp_src_hw_timer_init();
}

void audio_a2dp_src_play_state_handle(T_AUDIO_A2DP_SRC_PLAY_STATE_EVENT event)
{
    T_APP_AUDIO_STATE  cur_state = playback_db.sd_play_state;
    switch (event)
    {
    case AUDIO_A2DP_SRC_START:
        {
            playback_db.sd_play_state = APP_AUDIO_STATE_TRY_PLAYING;
        }
        break;
    case AUDIO_A2DP_SRC_STOP:
        {
            if (playback_db.sd_play_state == APP_AUDIO_STATE_PLAY)
            {
                playback_db.sd_play_state = APP_AUDIO_STATE_TRY_PAUSING;
            }
        }
        break;
    case AUDIO_A2DP_SRC_PIPE_CREATE:
        {
            //stream start -> pipe create -> play state
            playback_db.sd_play_state = APP_AUDIO_STATE_PLAY;
            //gui_update_by_event(GUI_EVENT_PLAYER, NULL, false);
        }
        break;
    case AUDIO_A2DP_SRC_PIPE_RELEASE:
        {
            //              -> pipe release }
            //a2dp src stop ->              |  -> pause/stop
            //              -> stream close }
            //in current process, stream will keep while switch songs. So in this case, we don't check steam state.
            if (playback_db.op_next_action == APP_AUDIO_STOPPED_IDLE_ACTION)
            {
                if (app_db.a2dp_src_state == APP_A2DP_SRC_STREAM_START)
                {
                    break;
                }
            }

            if (playback_db.sd_play_state == APP_AUDIO_STATE_TRY_STOPPING)
            {
                playback_db.sd_play_state = APP_AUDIO_STATE_STOP;
            }
            else
            {
                playback_db.sd_play_state = APP_AUDIO_STATE_PAUSE;
            }
            //gui_update_by_event(GUI_EVENT_PLAYER, NULL, false);
        }
        break;
    case AUDIO_A2DP_SRC_STREAM_STOP:
        {
            //pipe release and stream stop, set play state to stop or pause
            if (!app_db.audio_pipe_create)
            {
                if (playback_db.sd_play_state == APP_AUDIO_STATE_TRY_STOPPING)
                {
                    playback_db.sd_play_state = APP_AUDIO_STATE_STOP;
                }
                else
                {
                    playback_db.sd_play_state = APP_AUDIO_STATE_PAUSE;
                }
                //gui_update_by_event(GUI_EVENT_PLAYER, NULL, false);
            }
        }
        break;
    case AUDIO_A2DP_SRC_PROFILE_DISCON:
        {
            if (playback_db.sd_play_state >= APP_AUDIO_STATE_TRY_STOPPING)
            {
                playback_db.sd_play_state = APP_AUDIO_STATE_STOP;
                //gui_update_by_event(GUI_EVENT_PLAYER, NULL, false);
            }
        }
        break;
    default:
        break;
    }

    APP_PRINT_INFO3("a2dp src play state cur = %d, new = %d, event = %d", cur_state, \
                    playback_db.sd_play_state, event);
}
