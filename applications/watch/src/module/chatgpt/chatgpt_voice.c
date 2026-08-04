/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "chatgpt_queue.h"
#include "chatgpt_port.h"
#include "chatgpt_voice.h"
#include "rtk_opus.h"
#include "audio_type.h"
#include "audio_track.h"
#include "audio.h"
#include "app_timer.h"

/*============================================================================*
 *                              Macros
 *============================================================================*/
#define  CHATGPT_VOICE_LEVEL_HIGH      (300)  //high level  ms
#define  CHATGPT_VOICE_LEVEL_LOW       (250)  //low  level  ms

typedef enum
{
    CHATGPT_PLAY_WRITE        = 0x00,
} T_CHATGPT_VOICE_TIMER;

/*============================================================================*
 *                              Variable
 *============================================================================*/
static void *enc_handle  = NULL;
static rtk_opus_param_t param;
static bool chatgpt_record_start = false;
T_AUDIO_TRACK_HANDLE chatgpt_record_handle = NULL;
T_AUDIO_TRACK_HANDLE chatgpt_play_handle = NULL;
static uint8_t chatgpt_voice_timer_id = 0;
static uint8_t timer_idx_play_write = 0;
static bool chatgpt_play_buf_high = false;
static bool chatgpt_voice_recv_complete = false;
static _chatgpt_scene_thread *p_record_secne = NULL;
static _chatgpt_scene_thread *p_play_secne = NULL;

/*****************************************************************************
                                  Opus encoder
******************************************************************************/
void chatgpt_opus_encoder_create(void)
{
    if (enc_handle)
    {
        APP_PRINT_INFO0("opus encoder already exist");
        return;
    }
    // Set encoding parameters
    rtk_opus_param_set_default(&param);
    param.sampling_rate = 16000;
    param.channels = 1;
    param.bitrate_bps = 16000;
    param.application = RTK_OPUS_APP_VOIP;
    param.bandwidth = RTK_OPUS_BW_AUTO;
    param.frame_size_type = RTK_OPUS_FS_20MS;
    param.frame_size = param.sampling_rate / 50;
    param.complexity = 3;
    param.use_vbr = 0;
    param.use_inbandfec = 0;
    param.use_dtx = 0;
    param.forcechannels = RTK_OPUS_FORCE_AUTO;
    param.max_payload_bytes = 1500;
    param.final_range_mode = RTK_OPUS_FINAL_RANGE_NONE;
    param.final_range = 0;

    enc_handle = rtk_celt_encoder_init(&param);
    if (!enc_handle)
    {
        DBG_DIRECT("rtk_encoder_init failed!\n");
        return;
    }
}

int chatgpt_opus_encoder(uint16_t *pcm_in, uint8_t *out, int *out_len)
{
    int res = 0;
    if (enc_handle)
    {
        res = rtk_celt_encode(enc_handle, &param, pcm_in, out, out_len);
    }

    return res;
}

void chatgpt_opus_encoder_release(void)
{
    if (enc_handle)
    {
        rtk_celt_encoder_destroy(enc_handle);
        enc_handle = NULL;
    }
}

/*****************************************************************************
                                  voice record
******************************************************************************/
bool chatgpt_record_read_cb(T_AUDIO_TRACK_HANDLE  handle,
                            uint32_t             *timestamp,
                            uint16_t             *seq_num,
                            T_AUDIO_STREAM_STATUS *status,
                            uint8_t              *frame_num,
                            void                 *buf,
                            uint16_t              required_len,
                            uint16_t             *actual_len)
{
    APP_PRINT_TRACE2("[ChatGPT] chatgpt_record_read_cb: buf 0x%08x, required_len %d", buf,
                     required_len);

    static uint16_t counter = 0;
    static uint8_t sendbuf[4 + 4 * 40] = {0};
    {

        if (chatgpt_record_start)
        {
            uint8_t out[45];
            int out_len = 0;
            chatgpt_opus_encoder(buf, &out[0], &out_len);
            sendbuf[counter * 41] = out_len;
            memcpy(sendbuf + counter * 41 + 1, out, out_len);
            counter ++;
            if (counter >= 4)
            {
                extern uint8_t chatgpt_voice_data_send(_chatgpt_scene_thread * p_scene, uint8_t *pbuf,
                                                       uint16_t payload_length);
                chatgpt_voice_data_send(p_record_secne, sendbuf, 164);
                counter = 0;
            }
        }
    }

    *actual_len = required_len;

    return true;
}


void chatgpt_start_record(void)
{
    if (chatgpt_record_start != false) /*g_voice_data.is_voice_start == false8*/
    {
        APP_PRINT_ERROR0("chatgpt_start_record: already started");
        return;
    }

    APP_PRINT_TRACE0("[chatgpt] chatgpt_start_record");
    audio_track_start(chatgpt_record_handle);
    chatgpt_record_start = true;
}


void chatgpt_record_init(_chatgpt_scene_thread *p_scene)
{
    APP_PRINT_TRACE0("[ChatGPT] chatgpt_record_init");
    T_AUDIO_FORMAT_INFO format_info;
    format_info.type = AUDIO_FORMAT_TYPE_PCM;
    format_info.attr.pcm.sample_rate = 16 * 1000;
    format_info.attr.pcm.bit_width = 16;
    format_info.attr.pcm.chann_num = 1;
    format_info.attr.pcm.frame_length = 640;
    format_info.attr.pcm.chann_location = AUDIO_CHANNEL_LOCATION_MONO;

    chatgpt_record_handle = audio_track_create(AUDIO_STREAM_TYPE_RECORD,
                                               AUDIO_STREAM_MODE_NORMAL,
                                               AUDIO_STREAM_USAGE_LOCAL,
                                               format_info,
                                               0,
                                               15,
                                               AUDIO_DEVICE_IN_MIC,
                                               NULL,
                                               chatgpt_record_read_cb);

    if (chatgpt_record_handle == NULL)
    {
        APP_PRINT_ERROR0("chatgpt_record_init: handle is NULL");
    }
    else
    {
        chatgpt_opus_encoder_create();
    }
    p_record_secne = p_scene;
}

void chatgpt_stop_record(void)
{
    if (chatgpt_record_start != true)
    {
        APP_PRINT_ERROR0("chatgpt_stop_record: already stopped!");
        return;
    }

    APP_PRINT_TRACE0("chatgpt_stop_record");
    audio_track_release(chatgpt_record_handle);
    chatgpt_record_handle = NULL;
    chatgpt_record_start = false;
    chatgpt_opus_encoder_release();
}

/*****************************************************************************
                                  voice play
******************************************************************************/
void chatgpt_set_voice_recv_status(bool recv_complete)
{
    chatgpt_voice_recv_complete = recv_complete;
}

static void chatgpt_play_cback(T_AUDIO_EVENT event_type, void *event_buf, uint16_t buf_len)
{
    T_AUDIO_EVENT_PARAM *param = event_buf;
    bool handle = true;

    if (param->track_state_changed.handle != chatgpt_play_handle)
    {
        return;
    }

    chatgpt_log("[ChatGPT] chatgpt_voice_dsp_cbackevent_type %d", event_type);
    switch (event_type)
    {
    case AUDIO_EVENT_TRACK_STATE_CHANGED:
        {
            chatgpt_log("[ChatGPT] track_state_changed.state %d",
                        param->track_state_changed.state);

            switch (param->track_state_changed.state)
            {
            case AUDIO_TRACK_STATE_STARTED:
                {
                    chatgpt_play_timer_start();
                }
                break;

            case AUDIO_TRACK_STATE_STOPPED:
                break;

            default:
                break;
            }
        }
        break;

    case AUDIO_EVENT_TRACK_BUFFER_LOW:
        {
            chatgpt_play_buf_high = false;
            APP_PRINT_TRACE0("[ChatGPT] chatgpt_play_cback: AUDIO_EVENT_TRACK_BUFFER_LOW");
        }
        break;

    case AUDIO_EVENT_TRACK_BUFFER_HIGH:
        {
            chatgpt_play_buf_high = true;
            APP_PRINT_TRACE0("[ChatGPT] chatgpt_play_cback: AUDIO_EVENT_TRACK_BUFFER_HIGH");
        }
        break;

    default:
        handle = false;
        break;
    }

    if (handle == true)
    {
        APP_PRINT_TRACE1("[ChatGPT] chatgpt_play_cback: event_type 0x%04x", event_type);
    }
}


void chatgpt_play_start(_chatgpt_scene_thread *p_scene)
{
    T_AUDIO_FORMAT_INFO format_info = {};
    format_info.frame_num = 3;
    format_info.type = AUDIO_FORMAT_TYPE_SBC;
    format_info.attr.sbc.sample_rate = 16 * 1000;
    format_info.attr.sbc.chann_mode = AUDIO_SBC_CHANNEL_MODE_MONO;
    format_info.attr.sbc.chann_location = AUDIO_CHANNEL_LOCATION_MONO;
    format_info.attr.sbc.block_length = 16;
    format_info.attr.sbc.bitpool = 26;
    format_info.attr.sbc.subband_num = 8;
    format_info.attr.sbc.allocation_method = 0;
    chatgpt_play_handle = audio_track_create(AUDIO_STREAM_TYPE_PLAYBACK,
                                             AUDIO_STREAM_MODE_NORMAL,
                                             AUDIO_STREAM_USAGE_SNOOP,
                                             format_info,
                                             15,
                                             0,
                                             AUDIO_DEVICE_OUT_SPK,
                                             NULL,
                                             NULL);

    if (chatgpt_play_handle == NULL)
    {
        APP_PRINT_ERROR0("[ChatGPT] chatgpt_play_start: handle is NULL");
    }
    else
    {
        audio_track_threshold_set(chatgpt_play_handle, CHATGPT_VOICE_LEVEL_HIGH,
                                  CHATGPT_VOICE_LEVEL_LOW);
    }

    audio_mgr_cback_register(chatgpt_play_cback);


    if (audio_track_start(chatgpt_play_handle))
    {
        APP_PRINT_ERROR0("[ChatGPT] chatgpt_play_start: audio_track_start success");
    }
    else
    {
        APP_PRINT_ERROR0("[ChatGPT] chatgpt_play_start: audio_track_start fail");
    }

    p_play_secne = p_scene;
    chatgpt_play_buf_high = false;
    chatgpt_voice_recv_complete = false;
}

void chatgpt_play_write(_chatgpt_scene_thread *p_scene)
{
    if (chatgpt_play_buf_high)
    {
        APP_PRINT_INFO0("chatgpt_play_write: buffer level high");
        chatgpt_play_timer_start();
        return;
    }

    if (p_scene == NULL)
    {
        APP_PRINT_ERROR0("[ChatGPT] chatgpt_play_write scene null");
        return;
    }

    static uint16_t  seq_num = 0;
    chatgpt_queue_t *p_last_node = NULL;

    for (uint8_t i = 0; i < 4; i++) //1 packet = 3 sbc frames = 8 * 3 = 24 ms
    {
        p_last_node = chatgpt_queue_indexof_last(p_scene->audio_rev_list);
        if (p_last_node == NULL)
        {
            break;
        }

        // APP_PRINT_INFO2("[ChatGPT] audio write data len = %d,  %b", p_last_node->data_length,
        //                 TRACE_BINARY(10, p_last_node->p_data));
        seq_num++;
        uint16_t written_len;
        audio_track_write(chatgpt_play_handle, 0xFFFFFFFF,
                          seq_num,
                          AUDIO_STREAM_STATUS_CORRECT,
                          3,
                          p_last_node->p_data,
                          180,
                          &written_len);
        p_scene->audio_rev_list = chatgpt_queue_remove_last_node(p_scene->audio_rev_list);
    }

    if (chatgpt_voice_recv_complete && (p_last_node == NULL))
    {
        chatgpt_play_stop();
    }
    else
    {
        chatgpt_play_timer_start();
    }
}

void chatgpt_play_stop(void)
{
    APP_PRINT_TRACE0("[ChatGPT] chatgpt_play_stop");
    if (chatgpt_play_handle)
    {
        audio_track_stop(chatgpt_play_handle);
        audio_track_release(chatgpt_play_handle);
        chatgpt_play_handle = NULL;
    }
}

void chatgpt_voice_timeout_cb(uint8_t timer_evt, uint16_t param)
{
    APP_PRINT_TRACE2("chatgpt_voice_timeout_cb: timer_id %d, param %d", timer_evt, param);

    switch (timer_evt)
    {
    case CHATGPT_PLAY_WRITE:
        {
            app_stop_timer(&timer_idx_play_write);
            chatgpt_play_write(p_play_secne);
        }
        break;
    default:
        break;
    }
}

void chatgpt_play_timer_start(void)
{
    if (chatgpt_voice_timer_id == 0)
    {
        app_timer_reg_cb(chatgpt_voice_timeout_cb, &chatgpt_voice_timer_id);
    }
    app_start_timer(&timer_idx_play_write, "chatgpt_play_write",
                    chatgpt_voice_timer_id, CHATGPT_PLAY_WRITE, 0, false,
                    100);
}
