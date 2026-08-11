/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <string.h>
#include "os_mem.h"
#include "trace.h"
#include "app_timer.h"
#include "btm.h"
#include "audio.h"
#include "ringtone.h"
#include "bt_avrcp.h"
#include "bt_hfp.h"
#include "bt_bond.h"
#include "audio_volume.h"
#include "app_audio_policy.h"
#include "app_main.h"
#include "app_report.h"
#include "app_cfg.h"
#include "app_hfp.h"
#include "eq.h"
#include "app_audio_if.h"
#include "app_mmi.h"
#include "bt_a2dp.h"
#include "app_cmd.h"
#include "app_audio_route.h"
#include "audio_probe.h"
#include "app_eq.h"
#include "audio_playback.h"
#include "stdlib.h"
#include "audio_hfp.h"
#if CONFIG_REALTEK_APP_AUDIO_DATA_CAPTURE
#include "audio_data_capture.h"
#endif


bool (*app_audio_notify_check)(void) = NULL;
void app_audio_speaker_channel(T_AUDIO_CHANCEL_SET set_mode, uint8_t spk_channel,
                               uint8_t mic_channel);

static bool app_audio_get_category(T_AUDIO_TRACK_HANDLE handle, T_AUDIO_CATEGORY *category)
{
    T_AUDIO_STREAM_TYPE stream_type;

    if (!audio_track_stream_type_get(handle, &stream_type))
    {
        return false;
    }

    switch (stream_type)
    {
    case AUDIO_STREAM_TYPE_PLAYBACK:
        *category = AUDIO_CATEGORY_AUDIO;
        break;

    case AUDIO_STREAM_TYPE_VOICE:
        *category = AUDIO_CATEGORY_VOICE;
        break;

    case AUDIO_STREAM_TYPE_RECORD:
        *category = AUDIO_CATEGORY_RECORD;
        break;
    }

    return true;
}

static bool app_audio_get_endpoint(T_AUDIO_TRACK_HANDLE handle, uint8_t *endpoint)
{
    uint32_t device;
    if (!audio_track_device_get(handle, &device))
    {
        return false;
    }

    if (__builtin_popcount(device) != 1)
    {
        return false;
    }

    switch (device)
    {
    case AUDIO_DEVICE_OUT_SPK:
        *endpoint = AUDIO_ROUTE_ENDPOINT_SPK;
        break;
    case AUDIO_DEVICE_OUT_SPDIF:
        *endpoint = AUDIO_ROUTE_ENDPOINT_SPDIF;
        break;
    case AUDIO_DEVICE_IN_MIC:
        *endpoint = AUDIO_ROUTE_ENDPOINT_MIC;
        break;
    case AUDIO_DEVICE_IN_AUX:
        *endpoint = AUDIO_ROUTE_ENDPOINT_AUX;
        break;
    case AUDIO_DEVICE_IN_SPDIF:
        *endpoint = AUDIO_ROUTE_ENDPOINT_SPDIF;
        break;
    default:
        return false;
    }

    return true;
}

static bool app_audio_get_channel_num(T_AUDIO_FORMAT_INFO *format_info, uint8_t *chnl_cnt)
{
    uint32_t chann_location = 0;
    switch (format_info->type)
    {
    case AUDIO_FORMAT_TYPE_PCM:
        chann_location = format_info->attr.pcm.chann_location;
        break;
    case AUDIO_FORMAT_TYPE_SBC:
        chann_location = format_info->attr.sbc.chann_location;
        break;
    case AUDIO_FORMAT_TYPE_AAC:
        chann_location = format_info->attr.aac.chann_location;
        break;
    case AUDIO_FORMAT_TYPE_LDAC:
        chann_location = format_info->attr.ldac.chann_location;
        break;
    case AUDIO_FORMAT_TYPE_LHDC:
        chann_location = format_info->attr.lhdc.chann_location;
        break;
    case AUDIO_FORMAT_TYPE_LC3:
        chann_location = format_info->attr.lc3.chann_location;
        break;
    case AUDIO_FORMAT_TYPE_LC3PLUS:
        chann_location = format_info->attr.lc3plus.chann_location;
        break;
    case AUDIO_FORMAT_TYPE_MP3:
        chann_location = format_info->attr.mp3.chann_location;
        break;
    default:
        APP_PRINT_ERROR1("app_audio_get_channel_num: Unexpected codec %d", format_info->type);
        return false;
    }
    *chnl_cnt = (chann_location == AUDIO_CHANNEL_LOCATION_MONO) ? 1 : __builtin_popcount(
                    chann_location);
    return true;
}

static bool app_audio_get_gateway_num(T_AUDIO_TRACK_HANDLE handle, uint8_t *gateway_channel_num)
{
    uint8_t ret = 0;

    uint32_t device;
    T_AUDIO_CATEGORY category;
    T_AUDIO_ROUTE_ENDPOINT_TYPE endpoint_check_type;
    uint8_t pri_gateway_channel_mask = 0;
    uint8_t sec_gateway_channel_mask = 0;

    if (!audio_track_device_get(handle, &device))
    {
        ret = 1;
        goto err;
    }

    if (!app_audio_get_category(handle, &category))
    {
        ret = 2;
        goto err;
    }

    if (category != AUDIO_CATEGORY_AUDIO &&
        category != AUDIO_CATEGORY_RECORD)
    {
        ret = 3;
        goto err;
    }


    if (!app_audio_get_endpoint(handle, &endpoint_check_type))
    {
        ret = 4;
        goto err;
    }

    T_AUDIO_ROUTE_PATH path;
    path = audio_route_path_take(category,
                                 device);

    for (int i = 0; i < path.entry_num; i++)
    {
        T_AUDIO_ROUTE_ENTRY *p_entry = &path.entry[i];
        T_AUDIO_ROUTE_IO_TYPE io_type = p_entry->io_type;
        T_AUDIO_ROUTE_ENDPOINT_TYPE endpoint_type = p_entry->endpoint_type;

        if (endpoint_type == endpoint_check_type)
        {
            if (io_type == AUDIO_ROUTE_IO_RECORD_PRIMARY_IN || io_type == AUDIO_ROUTE_IO_AUDIO_PRIMARY_OUT)
            {
                pri_gateway_channel_mask = p_entry->gateway_attr.channs;
            }

            if (io_type == AUDIO_ROUTE_IO_RECORD_SECONDARY_IN || io_type == AUDIO_ROUTE_IO_AUDIO_SECONDARY_OUT)
            {
                sec_gateway_channel_mask = p_entry->gateway_attr.channs;
            }
        }
    }
    audio_route_path_give(&path);

    *gateway_channel_num = __builtin_popcount(pri_gateway_channel_mask) + __builtin_popcount(
                               sec_gateway_channel_mask);

    return true;

err:
    APP_PRINT_ERROR1("app_audio_get_gateway_num: failed -%d", ret);
    return false;
}

void app_audio_pipe_chann_set(T_AUDIO_PIPE_HANDLE handle)
{
    uint8_t input_channel_num = 0;
    uint8_t output_channel_num = 0;

    T_AUDIO_FORMAT_INFO in_fmt;
    T_AUDIO_FORMAT_INFO out_fmt;

    audio_pipe_format_info_get(handle, &in_fmt, &out_fmt);

    if (!app_audio_get_channel_num(&in_fmt, &input_channel_num) ||
        !app_audio_get_channel_num(&out_fmt, &output_channel_num))
    {
        return;
    }

    if (input_channel_num != 2)
    {
        // TODO: Please modify here to support if there are special requirements
        APP_PRINT_INFO1("app_audio_pipe_chann_set: no need to set channel for channel num %d",
                        input_channel_num);
        return;
    }

    if (output_channel_num == 1)
    {
        uint32_t chnl = 0x00000001 | 0x00000002;
        audio_pipe_channel_out_set(handle, output_channel_num, &chnl);
    }
    else if (output_channel_num == 2)
    {
        uint32_t chann_array[] = {0x00000001, 0x00000002};
        audio_pipe_channel_out_set(handle, output_channel_num, chann_array);
    }
    else
    {
        // TODO: Please modify here to support if there are special requirements
        APP_PRINT_ERROR1("app_audio_pipe_chann_set: Unsupported output channel num %d",
                         output_channel_num);
    }
}

static void app_audio_track_chann_set(T_AUDIO_TRACK_HANDLE handle)
{
    T_AUDIO_STREAM_TYPE stream_type;
    T_AUDIO_FORMAT_INFO format_info;
    uint8_t chnl_cnt = 0;
    uint8_t gateway_channel_num = 0;

    audio_track_stream_type_get(handle, &stream_type);
    audio_track_format_info_get(handle, &format_info);
    if (!app_audio_get_channel_num(&format_info, &chnl_cnt))
    {
        return;
    }

    if (chnl_cnt != 2)
    {
        // set channel map when codec supports 2 channels
        APP_PRINT_INFO0("app_audio_track_chann_set: no need to set");
        return;
    }

    if (!app_audio_get_gateway_num(handle, &gateway_channel_num))
    {
        return;
    }

    if (stream_type == AUDIO_STREAM_TYPE_PLAYBACK)
    {
        if (gateway_channel_num == 1)
        {
            // always mix 2 channels data
            uint32_t chann_mask = 0x00000001 | 0x00000002;
            audio_track_channel_out_set(handle, gateway_channel_num, &chann_mask);
        }
        else if (gateway_channel_num == 2)
        {
            uint32_t chann_array[] = {0x00000001, 0x00000002};
            audio_track_channel_out_set(handle, gateway_channel_num, chann_array);
        }
        else
        {
            // TODO: Please modify here to support if there are special requirements
        }
    }
    else if (stream_type == AUDIO_STREAM_TYPE_RECORD)
    {
        if (gateway_channel_num == 1)
        {
            uint32_t chann_array[] = {0x00000001, 0x00000001}; // array_size = chnl_cnt
            audio_track_channel_in_set(handle, chnl_cnt, chann_array);
        }
        else if (gateway_channel_num == 2)
        {
            uint32_t chann_array[] = {0x00000001, 0x00000002}; // array_size = chnl_cnt
            audio_track_channel_in_set(handle, chnl_cnt, chann_array);
        }
        else
        {
            // TODO: Please modify here to support if there are special requirements
        }
    }
    else
    {
        // Voice always uses 1 channel
    }
}

static void app_audio_policy_cback(T_AUDIO_EVENT event_type, void *event_buf, uint16_t buf_len)
{
    T_AUDIO_EVENT_PARAM *param = event_buf;
    bool handle = true;

    switch (event_type)
    {
    case AUDIO_EVENT_TRACK_STATE_CHANGED:
        {
            T_APP_BR_LINK *p_link;
            T_AUDIO_STREAM_TYPE stream_type;

            if (audio_track_stream_type_get(param->track_state_changed.handle, &stream_type) == false)
            {
                APP_PRINT_INFO0("audio track stream type get fail");
                break;
            }
            DBG_DIRECT("audio stream type = %d, state = %d", stream_type, param->track_state_changed.state);
            if (stream_type == AUDIO_STREAM_TYPE_PLAYBACK)
            {
                if (param->track_state_changed.state == AUDIO_TRACK_STATE_STARTED)
                {

                }
                else if (param->track_state_changed.state == AUDIO_TRACK_STATE_STOPPED ||
                         param->track_state_changed.state == AUDIO_TRACK_STATE_PAUSED)
                {
                }
                else if (param->track_state_changed.state == AUDIO_TRACK_STATE_CREATED)
                {
                    app_audio_track_chann_set(param->track_state_changed.handle);
                }
            }
            else if (stream_type == AUDIO_STREAM_TYPE_VOICE)
            {
                if (param->track_state_changed.state == AUDIO_TRACK_STATE_STOPPED ||
                    param->track_state_changed.state == AUDIO_TRACK_STATE_PAUSED)
                {

                }
                else if (param->track_state_changed.state == AUDIO_TRACK_STATE_STARTED ||
                         param->track_state_changed.state == AUDIO_TRACK_STATE_RESTARTED)
                {
                    p_link = &app_db.br_link[app_hfp_get_active_hf_index()];
                    p_link->duplicate_fst_sco_data = true;
                }
                else if (param->track_state_changed.state == AUDIO_TRACK_STATE_RELEASED)
                {
                }
            }
        }
        break;
    case AUDIO_EVENT_TRACK_DATA_IND:
        {
#if (CONFIG_REALTEK_APP_AUDIO_DATA_CAPTURE == 1)
            if (((audio_data_capture_get_state() & DATA_CAPTURE_DATA_START_SCO_MODE) == 0))
#endif
            {
                audio_sco_data_read(param);
            }
        }
        break;

    case AUDIO_EVENT_TRACK_VOLUME_OUT_CHANGED:
        {

        }
        break;

    case AUDIO_EVENT_VOICE_PROMPT_STARTED:
        {
            app_db.tone_vp_status.state = APP_TONE_VP_STARTED;
            app_db.tone_vp_status.index = param->voice_prompt_started.index;
        }
        break;

    case AUDIO_EVENT_VOICE_PROMPT_STOPPED:
        {
            app_db.tone_vp_status.state = APP_TONE_VP_STOP;
            app_db.tone_vp_status.index = param->voice_prompt_stopped.index;
        }
        break;

    case AUDIO_EVENT_RINGTONE_STARTED:
        {
            app_db.tone_vp_status.state = APP_TONE_VP_STARTED;
            app_db.tone_vp_status.index = param->ringtone_started.index;
        }
        break;

    case AUDIO_EVENT_RINGTONE_STOPPED:
        {
            app_db.tone_vp_status.state = APP_TONE_VP_STOP;
            app_db.tone_vp_status.index = param->ringtone_stopped.index;
        }
        break;

    default:
        handle = false;
        break;
    }

    if (handle == true)
    {
        APP_PRINT_INFO1("app_audio_policy_cback: event_type 0x%04x", event_type);
    }
}

void app_audio_init(void)
{
    audio_mgr_cback_register(app_audio_policy_cback);

    app_audio_route_gain_init();
    app_eq_init();

    voice_prompt_language_set((T_VOICE_PROMPT_LANGUAGE_ID)app_cfg_nv.voice_prompt_language);

    audio_volume_out_max_set(AUDIO_STREAM_TYPE_PLAYBACK, app_cfg_volume.playback_volume_max);
    audio_volume_out_max_set(AUDIO_STREAM_TYPE_VOICE, app_cfg_volume.voice_out_volume_max);
    audio_volume_in_max_set(AUDIO_STREAM_TYPE_VOICE, app_cfg_volume.voice_volume_in_max);

    audio_volume_out_min_set(AUDIO_STREAM_TYPE_PLAYBACK, app_cfg_volume.playback_volume_min);
    audio_volume_out_min_set(AUDIO_STREAM_TYPE_VOICE, app_cfg_volume.voice_out_volume_min);
    audio_volume_in_min_set(AUDIO_STREAM_TYPE_VOICE, app_cfg_volume.voice_volume_in_min);

    audio_volume_out_set(AUDIO_STREAM_TYPE_PLAYBACK, app_cfg_volume.playback_volume_default);
    audio_volume_out_set(AUDIO_STREAM_TYPE_VOICE, app_cfg_volume.voice_out_volume_default);
    audio_volume_in_set(AUDIO_STREAM_TYPE_VOICE, app_cfg_volume.voice_volume_in_default);

    voice_prompt_volume_max_set(app_cfg_volume.voice_prompt_volume_max);
    voice_prompt_volume_min_set(app_cfg_volume.voice_prompt_volume_min);
    voice_prompt_volume_set(app_cfg_volume.voice_prompt_volume_default);

    ringtone_volume_max_set(app_cfg_volume.ringtone_volume_max);
    ringtone_volume_min_set(app_cfg_volume.ringtone_volume_min);
    ringtone_volume_set(app_cfg_volume.ringtone_volume_default);
}



bool app_audio_tone_play(uint8_t index, bool flush, bool relay)
{
    bool ret = false;

    APP_PRINT_TRACE3("app_audio_tone_play: 0x%02x,state=%d,index=%d",
                     index,
                     app_db.tone_vp_status.state,
                     app_db.tone_vp_status.index);
    if (app_audio_notify_check != NULL)
    {
        if (app_audio_notify_check())
        {
            if (index < VOICE_PROMPT_INDEX)
            {
                if ((app_db.tone_vp_status.state == APP_TONE_VP_STARTED) &&
                    (app_db.tone_vp_status.index == index))
                {
                    APP_PRINT_TRACE2("app_audio_tone_play: must ignore same tone_index=%d, cur_index=%d",
                                     index,
                                     app_db.tone_vp_status.index);
                    return ret;
                }
            }
            else
            {
                if ((app_db.tone_vp_status.state == APP_TONE_VP_STARTED) &&
                    (app_db.tone_vp_status.index == (index - VOICE_PROMPT_INDEX)))
                {
                    APP_PRINT_TRACE2("app_audio_tone_play: must ignore same vp_index=%d, cur_index=%d",
                                     index,
                                     app_db.tone_vp_status.index);
                    return ret;
                }
            }
        }
    }
    if (index < VOICE_PROMPT_INDEX)
    {
        if (flush)
        {
            ;//ringtone_cancel(index);//need owner fix
        }
        ret = ringtone_play(index, relay);
    }
    else if (index < TONE_INVALID_INDEX)
    {
        if (flush)
        {
            ;//voice_prompt_cancel(index - VOICE_PROMPT_INDEX);//need owner fix
        }
        ret = voice_prompt_play(index - VOICE_PROMPT_INDEX,
                                (T_VOICE_PROMPT_LANGUAGE_ID)app_cfg_const.voice_prompt_language,
                                relay);
    }

    return ret;
}

bool app_audio_tone_cancel(uint8_t index, bool relay)
{
    bool ret = false;

    if (index < VOICE_PROMPT_INDEX)
    {
        ret = ringtone_cancel(index, relay);
    }
    else if (index < TONE_INVALID_INDEX)
    {
        ret = voice_prompt_cancel(index - VOICE_PROMPT_INDEX, relay);
    }

    APP_PRINT_TRACE3("app_audio_tone_type_cancel: index 0x%02x, realy %d, ret %d", index, relay, ret);

    return ret;
}

void app_audio_speaker_channel(T_AUDIO_CHANCEL_SET set_mode, uint8_t spk_channel,
                               uint8_t mic_channel)
{
    uint32_t mic_speaker_channel = 0;
    switch (set_mode)
    {
    case AUDIO_COUPLE_SET:
    case AUDIO_COUPLE_MIC_SET:
        {
            mic_speaker_channel = app_cfg_const.couple_speaker_channel & 0x0F;
            if (set_mode == AUDIO_COUPLE_MIC_SET)
            {
                mic_speaker_channel |= mic_channel << 8;
            }
            else
            {
                mic_speaker_channel |= app_cfg_const.mic_channel << 8;
            }
        }
        break;

    case AUDIO_SINGLE_SET:
    case AUDIO_SINGLE_MIC_SET:
        {
            mic_speaker_channel = app_cfg_const.solo_speaker_channel;
            if (set_mode == AUDIO_SINGLE_MIC_SET)
            {
                mic_speaker_channel |= mic_channel << 8;
            }
            else
            {
                mic_speaker_channel |= app_cfg_const.mic_channel << 8;
            }
        }
        break;

    case AUDIO_SPECIFIC_SET:
    case AUDIO_SPECIFIC_MIC_SET:
        {
            mic_speaker_channel = spk_channel;
            if (set_mode == AUDIO_SPECIFIC_MIC_SET)
            {
                mic_speaker_channel |= mic_channel << 8;
            }
            else
            {
                mic_speaker_channel |= app_cfg_const.mic_channel << 8;
            }
        }
        break;

    default:
        break;
    }
    app_cfg_nv.cur_spk_channel = mic_speaker_channel;
    app_cfg_nv.cur_mic = mic_speaker_channel >> 8;
    AUDIO_PRINT_TRACE4("app_audio_speaker_channel: set_mode = %d, couple_speaker_channel = %d, mic_speaker_channel = %x",
                       set_mode,
                       app_cfg_const.couple_speaker_channel,
                       app_cfg_const.solo_speaker_channel,
                       mic_speaker_channel);
}


void app_audio_speaker_channel_set(uint8_t spk_channel)
{
    app_audio_speaker_channel(AUDIO_SPECIFIC_SET, spk_channel, app_cfg_nv.cur_mic);
}

void app_audio_mic_channel_set(uint8_t mic_channel)
{
    app_audio_speaker_channel(AUDIO_SPECIFIC_MIC_SET, app_cfg_nv.cur_spk_channel, mic_channel);
}

