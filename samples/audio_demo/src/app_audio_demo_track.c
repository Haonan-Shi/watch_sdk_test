/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <string.h>
#include "trace.h"
#include "os_mem.h"
#include "console.h"
#include "audio_type.h"
#include "audio_track.h"
#include "audio_route.h"
#include "kws.h"
#include "app_audio_data.h"
#include "app_audio_demo_track.h"
#include "app_audio_data.h"

#define MSBC_FRAME_LEN 57

static T_AUDIO_TRACK_HANDLE audio_track_handle = NULL;
static uint8_t  app_format_type;
static uint16_t seq_num = 0;
static uint32_t data_offset = 0;
T_AUDIO_EFFECT_INSTANCE instance;

void app_audio_track_write_sbc_data(void)
{
    uint16_t written_len;

    if (data_offset + 486 <= sbc_data_len)
    {
        if (audio_track_write(audio_track_handle,
                              0,
                              seq_num,
                              AUDIO_STREAM_STATUS_CORRECT,
                              6,
                              (uint8_t *)(sbc_data + data_offset),
                              486,
                              &written_len))
        {
            seq_num++;
            data_offset += 486;
        }
    }
    else
    {
        data_offset = 0;
        app_audio_track_write_sbc_data();
    }
}

void app_audio_track_write_msbc_data(void)
{
    uint16_t written_len;

    if (data_offset + MSBC_FRAME_LEN <= msbc_data_len)
    {
        if (audio_track_write(audio_track_handle,
                              0,
                              seq_num,
                              AUDIO_STREAM_STATUS_CORRECT,
                              1,
                              (uint8_t *)(msbc_data + data_offset),
                              MSBC_FRAME_LEN,
                              &written_len))
        {
            seq_num++;
            data_offset += MSBC_FRAME_LEN;
        }
    }
    else
    {
        data_offset = 0;
        app_audio_track_write_msbc_data();
    }
}

void app_audio_track_write(void)
{
    switch (app_format_type)
    {
    case AUDIO_FORMAT_TYPE_PCM:
        {
            //app_audio_pipe_fill_pcm_data();
        }
        break;

    case AUDIO_FORMAT_TYPE_MSBC:
        {
            app_audio_track_write_msbc_data();
        }
        break;

    case AUDIO_FORMAT_TYPE_SBC:
        {
            app_audio_track_write_sbc_data();
        }
        break;

    case AUDIO_FORMAT_TYPE_LC3:
        {
            //app_audio_pipe_fill_lc3_data();
        }
        break;

    default:
        break;
    }
}

static uint8_t app_audio_get_channel_num(T_AUDIO_FORMAT_INFO format_info)
{
    uint8_t  chann_num = 0;
    uint32_t chann_location = 0;

    switch (format_info.type)
    {
    case AUDIO_FORMAT_TYPE_PCM:
        chann_location = format_info.attr.pcm.chann_location;
        break;
    case AUDIO_FORMAT_TYPE_SBC:
        chann_location = format_info.attr.sbc.chann_location;
        break;
    case AUDIO_FORMAT_TYPE_MSBC:
        chann_location = format_info.attr.msbc.chann_location;
        break;
    case AUDIO_FORMAT_TYPE_LC3:
        chann_location = format_info.attr.lc3.chann_location;
        break;
    default:
        break;
    }

    chann_num = (chann_location == AUDIO_CHANNEL_LOCATION_MONO) ? 1 : __builtin_popcount(
                    chann_location);
    return chann_num;
}

static bool app_audio_get_category(T_AUDIO_TRACK_HANDLE  handle,
                                   T_AUDIO_STREAM_TYPE   stream_type,
                                   T_AUDIO_CATEGORY     *category)
{
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

    default:
        break;
    }

    return true;
}

static bool app_audio_get_endpoint(T_AUDIO_TRACK_HANDLE  handle,
                                   uint32_t              device,
                                   uint8_t              *endpoint)
{
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

static bool app_audio_get_gateway_num(T_AUDIO_TRACK_HANDLE  handle,
                                      T_AUDIO_STREAM_TYPE   stream_type,
                                      uint32_t              device,
                                      uint8_t              *gateway_chann_num)
{
    uint32_t ret = 0;
    uint8_t pri_gateway_chann_num = 0;
    uint8_t sec_gateway_chann_num = 0;
    T_AUDIO_CATEGORY            category;
    T_AUDIO_ROUTE_ENDPOINT_TYPE endpoint_check_type;

    if (!app_audio_get_category(handle, stream_type, &category))
    {
        ret = 1;
        goto err;
    }

    if (category != AUDIO_CATEGORY_AUDIO &&
        category != AUDIO_CATEGORY_RECORD)
    {
        ret = 2;
        goto err;
    }

    if (!app_audio_get_endpoint(handle, device, &endpoint_check_type))
    {
        ret = 3;
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
                pri_gateway_chann_num = p_entry->gateway_attr.channs;
            }

            if (io_type == AUDIO_ROUTE_IO_RECORD_SECONDARY_IN || io_type == AUDIO_ROUTE_IO_AUDIO_SECONDARY_OUT)
            {
                sec_gateway_chann_num = p_entry->gateway_attr.channs;
            }
        }
    }
    audio_route_path_give(&path);

    *gateway_chann_num = __builtin_popcount(pri_gateway_chann_num) + __builtin_popcount(
                             sec_gateway_chann_num);

    return true;

err:
    APP_PRINT_ERROR1("app_audio_get_gateway_num: failed -%d", ret);
    return false;
}

static void app_audio_track_chann_set(T_AUDIO_TRACK_HANDLE handle,
                                      T_AUDIO_STREAM_TYPE  stream_type,
                                      T_AUDIO_FORMAT_INFO  format_info,
                                      uint32_t             device)
{
    uint8_t chann_num = 0;
    uint8_t gateway_chann_num = 0;

    chann_num = app_audio_get_channel_num(format_info);

    if (chann_num == 1)
    {
        return;
    }

    if (!app_audio_get_gateway_num(handle, stream_type, device, &gateway_chann_num))
    {
        return;
    }

    if (stream_type == AUDIO_STREAM_TYPE_PLAYBACK)
    {
        if (gateway_chann_num == 1)
        {
            // always mix 2 channels data
            uint32_t chann_mask = 0x00000001 | 0x00000002;
            audio_track_channel_out_set(handle, gateway_chann_num, &chann_mask);
        }
        else if (gateway_chann_num == 2)
        {
            uint32_t chann_array[] = {0x00000001, 0x00000002};
            audio_track_channel_out_set(handle, gateway_chann_num, chann_array);
        }
        else
        {
            // TODO: Please modify here to support if there are special requirements
        }
    }
    else if (stream_type == AUDIO_STREAM_TYPE_RECORD)
    {
        if (gateway_chann_num == 1)
        {
            uint32_t chann_array[] = {0x00000001, 0x00000001};
            audio_track_channel_in_set(handle, chann_num, chann_array);
        }
        else if (gateway_chann_num == 2)
        {
            uint32_t chann_array[] = {0x00000001, 0x00000002};
            audio_track_channel_in_set(handle, chann_num, chann_array);
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

bool app_audio_track_create(uint8_t stream_type,
                            uint8_t format_type,
                            uint8_t volume_out,
                            uint8_t volume_in)
{
    uint32_t device;
    bool     param_check = true;

    if (audio_track_handle == NULL)
    {
        T_AUDIO_FORMAT_INFO format = {};

        format.type = (T_AUDIO_FORMAT_TYPE)format_type;
        switch (format_type)
        {
        case AUDIO_FORMAT_TYPE_PCM:
            {
                format.frame_num = 1;
                format.attr.pcm.sample_rate = 48000;
                format.attr.pcm.frame_length = 240;
                format.attr.pcm.chann_num = 2;
                format.attr.pcm.bit_width = 16;
                format.attr.pcm.chann_location = AUDIO_CHANNEL_LOCATION_FL | AUDIO_CHANNEL_LOCATION_FR;
            }
            break;

        case AUDIO_FORMAT_TYPE_MSBC:
            {
                format.frame_num = 1;
                format.attr.msbc.sample_rate = 16000;
                format.attr.msbc.chann_mode = AUDIO_SBC_CHANNEL_MODE_MONO;
                format.attr.msbc.block_length = 15;
                format.attr.msbc.subband_num = 8;
                format.attr.msbc.allocation_method = 0;
                format.attr.msbc.bitpool = 26;
                format.attr.msbc.chann_location = AUDIO_CHANNEL_LOCATION_MONO;
            }
            break;

        case AUDIO_FORMAT_TYPE_LC3:
            {
                format.frame_num = 1;
                format.attr.lc3.sample_rate = 48000;
                format.attr.lc3.chann_location = AUDIO_CHANNEL_LOCATION_MONO;
                format.attr.lc3.frame_duration = AUDIO_LC3_FRAME_DURATION_10_MS;
                format.attr.lc3.frame_length = 120;
                format.attr.lc3.bit_width = 16;
                format.attr.lc3.presentation_delay = 0;
            }
            break;

        case AUDIO_FORMAT_TYPE_SBC:
            {
                format.frame_num = 6;
                format.attr.sbc.sample_rate = 48000;
                format.attr.sbc.chann_mode = AUDIO_SBC_CHANNEL_MODE_JOINT_STEREO;
                format.attr.sbc.block_length = 16;
                format.attr.sbc.subband_num = 8;
                format.attr.sbc.allocation_method = 0;
                format.attr.sbc.bitpool = 0x22;
                format.attr.sbc.chann_location = AUDIO_CHANNEL_LOCATION_FL | AUDIO_CHANNEL_LOCATION_FR;
            }
            break;

        default:
            param_check = false;
            break;
        }

        switch (stream_type)
        {
        case AUDIO_STREAM_TYPE_PLAYBACK:
            device = AUDIO_DEVICE_OUT_SPK;
            break;

        case AUDIO_STREAM_TYPE_VOICE:
            device = AUDIO_DEVICE_OUT_SPK | AUDIO_DEVICE_IN_MIC;
            break;

        case AUDIO_STREAM_TYPE_RECORD:
            device = AUDIO_DEVICE_IN_MIC;
            break;

        default:
            break;
        }

        if (param_check)
        {
            app_format_type = format_type;

            audio_track_handle = audio_track_create((T_AUDIO_STREAM_TYPE)stream_type,
                                                    AUDIO_STREAM_MODE_NORMAL,
                                                    AUDIO_STREAM_USAGE_LOCAL,
                                                    format,
                                                    volume_out,
                                                    volume_in,
                                                    device,
                                                    NULL,
                                                    NULL);
            audio_track_threshold_set(audio_track_handle, 330, 60);
            audio_track_latency_set(audio_track_handle, 100, false);
            app_audio_track_chann_set(audio_track_handle, (T_AUDIO_STREAM_TYPE)stream_type, format, device);

            if (stream_type == AUDIO_STREAM_TYPE_RECORD)
            {
                instance = kws_effect_create(KWS_EFFECT_CONTENT_TYPE_RECORD);
                audio_track_effect_attach(audio_track_handle, instance);
                kws_effect_enable(instance);
            }
        }
        else
        {
            char *temp_buff = "Audio Track Format Not Supported!\r\n";
            console_write((uint8_t *)temp_buff, strlen(temp_buff));
        }
    }
    else
    {
        char *temp_buff = "Audio Track Already Exist!\r\n";
        console_write((uint8_t *)temp_buff, strlen(temp_buff));
    }

    return true;
}

bool app_audio_track_start(void)
{
    if (audio_track_handle != NULL)
    {
        if (audio_track_start(audio_track_handle) == false)
        {
            char *temp_buff = "Audio Track Start Failed!\r\n";
            console_write((uint8_t *)temp_buff, strlen(temp_buff));
        }
    }
    else
    {
        char *temp_buff = "Audio Track Not Exist!\r\n";
        console_write((uint8_t *)temp_buff, strlen(temp_buff));
    }

    return true;
}

bool app_audio_track_stop(void)
{
    if (audio_track_handle != NULL)
    {
        if (instance != NULL)
        {
            kws_effect_disable(instance);
            audio_track_effect_detach(audio_track_handle, instance);
            kws_effect_release(instance);
        }
        if (audio_track_stop(audio_track_handle) == false)
        {
            char *temp_buff = "Audio Track Start Failed!\r\n";
            console_write((uint8_t *)temp_buff, strlen(temp_buff));
        }
    }
    else
    {
        char *temp_buff = "Audio Track Not Exist!\r\n";
        console_write((uint8_t *)temp_buff, strlen(temp_buff));
    }

    return true;
}

bool app_audio_track_pause(void)
{
    if (audio_track_handle != NULL)
    {
        if (audio_track_pause(audio_track_handle) == false)
        {
            char *temp_buff = "Audio Track Start Failed!\r\n";
            console_write((uint8_t *)temp_buff, strlen(temp_buff));
        }
    }
    else
    {
        char *temp_buff = "Audio Track Not Exist!\r\n";
        console_write((uint8_t *)temp_buff, strlen(temp_buff));
    }

    return true;
}

bool app_audio_track_restart(void)
{
    if (audio_track_handle != NULL)
    {
        if (audio_track_restart(audio_track_handle) == false)
        {
            char *temp_buff = "Audio Track Start Failed!\r\n";
            console_write((uint8_t *)temp_buff, strlen(temp_buff));
        }
    }
    else
    {
        char *temp_buff = "Audio Track Not Exist!\r\n";
        console_write((uint8_t *)temp_buff, strlen(temp_buff));
    }

    return true;
}

bool app_audio_track_release(void)
{
    if (audio_track_handle != NULL)
    {
        if (audio_track_release(audio_track_handle) == false)
        {
            char *temp_buff = "Audio Track Start Failed!\r\n";
            console_write((uint8_t *)temp_buff, strlen(temp_buff));
        }

        audio_track_handle = NULL;
    }
    else
    {
        char *temp_buff = "Audio Track Not Exist!\r\n";
        console_write((uint8_t *)temp_buff, strlen(temp_buff));
    }

    return true;
}

bool app_audio_track_volume_out_set(uint8_t volume_out)
{
    if (audio_track_handle != NULL)
    {
        audio_track_volume_out_set(audio_track_handle, volume_out);
    }
    else
    {
        char *temp_buff = "Audio Track Not Exist!\r\n";
        console_write((uint8_t *)temp_buff, strlen(temp_buff));
    }

    return true;
}

bool app_audio_track_volume_out_mute(void)
{
    if (audio_track_handle != NULL)
    {
        audio_track_volume_out_mute(audio_track_handle);
    }
    else
    {
        char *temp_buff = "Audio Track Not Exist!\r\n";
        console_write((uint8_t *)temp_buff, strlen(temp_buff));
    }

    return true;
}

bool app_audio_track_volume_out_unmute(void)
{
    if (audio_track_handle != NULL)
    {
        audio_track_volume_out_unmute(audio_track_handle);
    }
    else
    {
        char *temp_buff = "Audio Track Not Exist!\r\n";
        console_write((uint8_t *)temp_buff, strlen(temp_buff));
    }

    return true;
}
