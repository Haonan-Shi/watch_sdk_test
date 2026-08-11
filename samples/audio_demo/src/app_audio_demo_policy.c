/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "os_sched.h"
#include "trace.h"
#include "audio.h"
#include "audio_type.h"
#include "console.h"
#include "ringtone.h"
#include "voice_prompt.h"
#include "app_timer.h"
#include "app_audio_demo_track.h"
#include "app_audio_demo_policy.h"

#define VOICE_PROMPT_INDEX              0x80
#define TONE_INVALID_INDEX              0xFF
#define VOICE_PROMPT_LANGUAGE           0

#define AUDIO_TRACK_SBC_PLAY_INTERVAL   48

static uint8_t timer_idx_sbc_play = 0;
static uint8_t timer_idx_msbc_play = 0;
static uint8_t read_count = 0;

uint8_t audio_policy_timer_id = 0;

typedef enum t_app_audio_policy_timer
{
    APP_TIMER_AUDIO_DEMO_SBC_PLAY  = 0x00,
    APP_TIMER_AUDIO_DEMO_MSBC_PLAY = 0x01,
} T_APP_AUDIO_POLICY_TIMER;

static const int16_t default_dac_gain_table[] =
{
    0xc000, 0xeb00, 0xec80, 0xee00, 0xef80, 0xf100, 0xf280, 0xf400,
    0xf580, 0xf700, 0xf880, 0xfa00, 0xfb80, 0xfd00, 0xfe80, 0x0000
};

static const int16_t default_adc_gain_table[] =
{
    0x0000, 0x002f, 0x0037, 0x003f, 0x0047, 0x012f, 0x0137, 0x013f,
    0x0147, 0x022f, 0x0237, 0x023f, 0x0247, 0x032f, 0x0337, 0x033f
};

static void app_audio_policy_cback(T_AUDIO_EVENT event_type, void *event_buf, uint16_t buf_len)
{
    T_AUDIO_EVENT_PARAM *param = event_buf;
    T_AUDIO_STREAM_TYPE stream_type;

    switch (event_type)
    {
    case AUDIO_EVENT_TTS_STARTED:
        {

        }
        break;
    case AUDIO_EVENT_TTS_ALERTED:
        {

        }
        break;
    case AUDIO_EVENT_TTS_PAUSED:
        {

        }
        break;
    case AUDIO_EVENT_TTS_RESUMED:
        {

        }
        break;
    case AUDIO_EVENT_TTS_STOPPED:
        {

        }
        break;
    case AUDIO_EVENT_RINGTONE_STARTED:
        {
            char *temp_buff = "Ringtone played!\r\n";
            console_write((uint8_t *)temp_buff, strlen(temp_buff));
        }
        break;
    case AUDIO_EVENT_RINGTONE_STOPPED:
        {

        }
        break;
    case AUDIO_EVENT_VOICE_PROMPT_STARTED:
        {
            char *temp_buff = "VP played!\r\n";
            console_write((uint8_t *)temp_buff, strlen(temp_buff));
        }
        break;
    case AUDIO_EVENT_VOICE_PROMPT_STOPPED:
        {

        }
        break;
    case AUDIO_EVENT_PASSTHROUGH_ENABLED:
        {
            char *temp_buff = "Passthrough enabled!\r\n";
            console_write((uint8_t *)temp_buff, strlen(temp_buff));
        }
        break;
    case AUDIO_EVENT_PASSTHROUGH_DISABLED:
        {
            char *temp_buff = "Passthrough disabled!\r\n";
            console_write((uint8_t *)temp_buff, strlen(temp_buff));
        }
        break;
    case AUDIO_EVENT_LINE_STATE_CHANGED:
        {
            if (param->line_state_changed.state == AUDIO_LINE_STATE_STARTED)
            {
                char *temp_buff = "Line started!\r\n";
                console_write((uint8_t *)temp_buff, strlen(temp_buff));
            }
            else if (param->line_state_changed.state == AUDIO_LINE_STATE_STOPPED)
            {
                char *temp_buff = "Line stopped!\r\n";
                console_write((uint8_t *)temp_buff, strlen(temp_buff));
            }
        }
        break;
    case AUDIO_EVENT_ANC_ENABLED:
        {
            char *temp_buff = "ANC enabled!\r\n";
            console_write((uint8_t *)temp_buff, strlen(temp_buff));
        }
        break;
    case AUDIO_EVENT_ANC_DISABLED:
        {
            char *temp_buff = "ANC disabled!\r\n";
            console_write((uint8_t *)temp_buff, strlen(temp_buff));
        }
        break;
    case AUDIO_EVENT_VAD_ENABLED:
        {
            char *temp_buff = "VAD enabled!\r\n";
            console_write((uint8_t *)temp_buff, strlen(temp_buff));
        }
        break;
    case AUDIO_EVENT_VAD_DISABLED:
        {
            char *temp_buff = "VAD disabled!\r\n";
            console_write((uint8_t *)temp_buff, strlen(temp_buff));
        }
        break;
    case AUDIO_EVENT_VAD_DATA_IND:
        {
            char *temp_buff = "Voice activity detected!\r\n";
            console_write((uint8_t *)temp_buff, strlen(temp_buff));
        }
        break;
    case AUDIO_EVENT_KWS_HIT:
        {
            uint8_t  temp_buff[40];
            uint16_t buf_len;
            uint8_t  keyword_index;

            keyword_index = param->kws_hit.keyword_index;

            buf_len =  sprintf((char *)temp_buff, "KWS Hit: index %d\r\n", keyword_index);
            console_write(temp_buff, buf_len);
        }
        break;
    case AUDIO_EVENT_TRACK_STATE_CHANGED:
        {
            if (param->track_state_changed.state == AUDIO_TRACK_STATE_RELEASED)
            {
                char *temp_buff = "Audio Track Released!\r\n";
                console_write((uint8_t *)temp_buff, strlen(temp_buff));

                break;
            }

            if (audio_track_stream_type_get(param->track_state_changed.handle, &stream_type) == false)
            {
                break;
            }

            if (param->track_state_changed.state == AUDIO_TRACK_STATE_CREATED)
            {
                char *temp_buff = "Audio Track Created!\r\n";
                console_write((uint8_t *)temp_buff, strlen(temp_buff));
            }
            else if (param->track_state_changed.state == AUDIO_TRACK_STATE_STARTED ||
                     param->track_state_changed.state == AUDIO_TRACK_STATE_RESTARTED)
            {
                char *temp_buff = "Audio Track Started!\r\n";
                console_write((uint8_t *)temp_buff, strlen(temp_buff));

                if (stream_type == AUDIO_STREAM_TYPE_PLAYBACK)
                {
                    app_start_timer(&timer_idx_sbc_play,
                                    "audio_demo_sbc_play",
                                    audio_policy_timer_id,
                                    APP_TIMER_AUDIO_DEMO_SBC_PLAY,
                                    0,
                                    true,
                                    AUDIO_TRACK_SBC_PLAY_INTERVAL);
                }
                else if (stream_type == AUDIO_STREAM_TYPE_VOICE)
                {
                    app_start_timer(&timer_idx_msbc_play,
                                    "audio_demo_msbc_play",
                                    audio_policy_timer_id,
                                    APP_TIMER_AUDIO_DEMO_MSBC_PLAY,
                                    0,
                                    false,
                                    AUDIO_TRACK_SBC_PLAY_INTERVAL);
                }
                else if (stream_type == AUDIO_STREAM_TYPE_RECORD)
                {

                }
            }
            else if (param->track_state_changed.state == AUDIO_TRACK_STATE_STOPPED)
            {
                char *temp_buff = "Audio Track Stopped!\r\n";
                console_write((uint8_t *)temp_buff, strlen(temp_buff));

                if (stream_type == AUDIO_STREAM_TYPE_PLAYBACK)
                {
                    app_stop_timer(&timer_idx_sbc_play);
                }
            }
            else if (param->track_state_changed.state == AUDIO_TRACK_STATE_PAUSED)
            {
                char *temp_buff = "Audio Track Paused!\r\n";
                console_write((uint8_t *)temp_buff, strlen(temp_buff));

                if (stream_type == AUDIO_STREAM_TYPE_PLAYBACK)
                {
                    app_stop_timer(&timer_idx_sbc_play);
                }
            }
        }
        break;

    case AUDIO_EVENT_TRACK_DATA_IND:
        {
            T_AUDIO_STREAM_STATUS status;
            uint32_t timestamp;
            uint16_t seq_num;
            uint8_t  frame_num;
            uint16_t read_len;
            uint8_t *buf;

            buf = (uint8_t *)malloc(sizeof(uint8_t) + param->track_data_ind.len);
            if (buf != NULL)
            {
                if (!audio_track_read(param->track_data_ind.handle,
                                      &timestamp,
                                      &seq_num,
                                      &status,
                                      &frame_num,
                                      buf,
                                      param->track_data_ind.len,
                                      &read_len))
                {
                    APP_PRINT_TRACE0("AUDIO_EVENT_TRACK_DATA_IND: audio_track_read fail");
                    free(buf);
                    break;
                }

                //bt_sco_data_send(p_link->bd_addr, seq_num, buf, read_len);
                free(buf);
            }

            if (audio_track_stream_type_get(param->track_data_ind.handle, &stream_type) == true)
            {
                read_count++;

                if (stream_type == AUDIO_STREAM_TYPE_VOICE)
                {
                    if (read_count == 100)
                    {
                        char *temp_buff = "Voice Data In!\r\n";
                        console_write((uint8_t *)temp_buff, strlen(temp_buff));

                        read_count = 0;
                    }

                    app_audio_track_write();
                }
                else if (stream_type == AUDIO_STREAM_TYPE_RECORD)
                {
                    if (read_count == 100)
                    {
                        char *temp_buff = "Record Data In!\r\n";
                        console_write((uint8_t *)temp_buff, strlen(temp_buff));

                        read_count = 0;
                    }
                }
            }
        }
        break;

    case AUDIO_EVENT_TRACK_VOLUME_OUT_CHANGED:
        {
            uint8_t temp_buff[40];
            uint16_t buf_len;

            buf_len =  sprintf((char *)temp_buff, "Audio Track Volume Out Level: %d \r\n",
                               param->track_volume_out_changed.curr_volume);
            console_write(temp_buff, buf_len);
        }
        break;

    case AUDIO_EVENT_TRACK_VOLUME_OUT_MUTED:
        {
            char *temp_buff = "Audio Track Volume Out Muted!\r\n";
            console_write((uint8_t *)temp_buff, strlen(temp_buff));
        }
        break;

    case AUDIO_EVENT_TRACK_VOLUME_OUT_UNMUTED:
        {
            char *temp_buff = "Audio Track Volume Out Unmuted!\r\n";
            console_write((uint8_t *)temp_buff, strlen(temp_buff));
        }
        break;

    case AUDIO_EVENT_TRACK_BUFFER_LOW:
        {
            if (audio_track_stream_type_get(param->track_state_changed.handle, &stream_type) == true)
            {
                if (stream_type == AUDIO_STREAM_TYPE_PLAYBACK)
                {
                    app_audio_track_write();
                }
            }
        }
        break;

    case AUDIO_EVENT_TRACK_BUFFER_HIGH:
        {
            if (audio_track_stream_type_get(param->track_state_changed.handle, &stream_type) == true)
            {
                if (stream_type == AUDIO_STREAM_TYPE_PLAYBACK)
                {
                    app_stop_timer(&timer_idx_sbc_play);
                }
            }
        }
        break;

    default:
        break;
    }
}

static void app_audio_policy_timeout_cb(uint8_t timer_evt, uint16_t param)
{
    APP_PRINT_TRACE2("app_audio_policy_timeout_cb: timer_evt %d, param %d", timer_evt, param);

    switch (timer_evt)
    {
    case APP_TIMER_AUDIO_DEMO_SBC_PLAY:
        {
            for (int i = 0; i < 3; i++)
            {
                app_audio_track_write();
            }
        }
        break;

    case APP_TIMER_AUDIO_DEMO_MSBC_PLAY:
        {
            for (int i = 0; i < 15; i++)
            {
                app_audio_track_write();
            }
            app_stop_timer(&timer_idx_msbc_play);
        }
        break;

    default:
        break;
    }
}

static bool app_audio_route_dac_gain_get_cback(T_AUDIO_CATEGORY        category,
                                               uint32_t                level,
                                               T_AUDIO_ROUTE_DAC_GAIN *gain)
{
    switch (category)
    {
    case AUDIO_CATEGORY_AUDIO:
    case AUDIO_CATEGORY_VOICE:
    case AUDIO_CATEGORY_LINE:
    case AUDIO_CATEGORY_TONE:
    case AUDIO_CATEGORY_VP:
    case AUDIO_CATEGORY_APT:
    case AUDIO_CATEGORY_LLAPT:
        {
            gain->left_gain = default_dac_gain_table[level];
            gain->right_gain = default_dac_gain_table[level];
        }
        break;

    default:
        return false;
    }

    return true;
}

static bool app_audio_route_adc_gain_get_cback(T_AUDIO_CATEGORY        category,
                                               uint32_t                level,
                                               T_AUDIO_ROUTE_ADC_GAIN *gain)
{
    switch (category)
    {
    case AUDIO_CATEGORY_RECORD:
    case AUDIO_CATEGORY_VOICE:
    case AUDIO_CATEGORY_LINE:
    case AUDIO_CATEGORY_APT:
    case AUDIO_CATEGORY_LLAPT:
        {
            gain->left_gain = default_adc_gain_table[level];
            gain->right_gain = default_adc_gain_table[level];
        }
        break;

    default:
        return false;
    }

    return true;
}

void app_audio_route_gain_init(void)
{
    uint8_t index;

    for (index = 0; index < AUDIO_CATEGORY_NUM; index++)
    {
        audio_route_gain_register((T_AUDIO_CATEGORY)index,
                                  app_audio_route_dac_gain_get_cback,
                                  app_audio_route_adc_gain_get_cback);
    }
}

void app_audio_init(void)
{
    audio_mgr_cback_register(app_audio_policy_cback);
    app_timer_reg_cb(app_audio_policy_timeout_cb, &audio_policy_timer_id);
    app_audio_route_gain_init();
}

bool app_audio_notification_play(uint8_t tone_index,  bool flush, bool relay)
{
    bool ret;

    ret = false;

    if (tone_index < VOICE_PROMPT_INDEX)
    {
        if (flush)
        {
            ringtone_cancel(tone_index, true);
        }
        ret = ringtone_play(tone_index, relay);
    }
    else if (tone_index < TONE_INVALID_INDEX)
    {
        if (flush)
        {
            voice_prompt_cancel(tone_index - VOICE_PROMPT_INDEX, true);
        }
        ret = voice_prompt_play(tone_index - VOICE_PROMPT_INDEX,
                                (T_VOICE_PROMPT_LANGUAGE_ID)VOICE_PROMPT_LANGUAGE, relay);
    }

    return ret;
}
