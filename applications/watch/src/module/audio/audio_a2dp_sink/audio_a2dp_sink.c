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
#include "bt_avrcp.h"
#include "audio_volume.h"
#include "app_audio_policy.h"
#include "app_main.h"
#include "app_report.h"
#include "app_cfg.h"
#include "app_audio_if.h"
#include "bt_a2dp.h"
#include "app_audio_route.h"
#include "bt_bond.h"
#include "audio_a2dp_sink.h"


void audio_a2dp_sink_stream_start_handle(T_BT_EVENT_PARAM_A2DP_STREAM_START_IND *stream_ind)
{
    T_APP_BR_LINK *p_link;
    uint8_t pair_idx;
    T_AUDIO_FORMAT_INFO format_info = {};
    T_AUDIO_STREAM_MODE mode = AUDIO_STREAM_MODE_NORMAL;
    T_AUDIO_STREAM_USAGE stream = AUDIO_STREAM_USAGE_SNOOP;

    p_link = app_find_br_link(stream_ind->bd_addr);
    if (p_link == NULL)
    {
        return;
    }

    if (bt_bond_index_get(p_link->bd_addr, &pair_idx) == false)
    {
        APP_PRINT_ERROR0("a2dp stream start ind, get pair idx false");
        return;
    }

    bt_a2dp_stream_start_cfm(stream_ind->bd_addr, true);

    if (stream_ind->codec_type == BT_A2DP_CODEC_TYPE_SBC)
    {
        format_info.type = AUDIO_FORMAT_TYPE_SBC;
        format_info.frame_num = 5;
        switch (stream_ind->codec_info.sbc.sampling_frequency)
        {
        case BT_A2DP_SBC_SAMPLING_FREQUENCY_16KHZ:
            format_info.attr.sbc.sample_rate = 16000;
            break;
        case BT_A2DP_SBC_SAMPLING_FREQUENCY_32KHZ:
            format_info.attr.sbc.sample_rate = 32000;
            break;
        case BT_A2DP_SBC_SAMPLING_FREQUENCY_44_1KHZ:
            format_info.attr.sbc.sample_rate = 44100;
            break;
        case BT_A2DP_SBC_SAMPLING_FREQUENCY_48KHZ:
            format_info.attr.sbc.sample_rate = 48000;
            break;
        }

        switch (stream_ind->codec_info.sbc.channel_mode)
        {
        case BT_A2DP_SBC_CHANNEL_MODE_MONO:
            format_info.attr.sbc.chann_mode = AUDIO_SBC_CHANNEL_MODE_MONO;
            format_info.attr.sbc.chann_location = AUDIO_CHANNEL_LOCATION_MONO;
            break;
        case BT_A2DP_SBC_CHANNEL_MODE_DUAL_CHANNEL:
            format_info.attr.sbc.chann_mode = AUDIO_SBC_CHANNEL_MODE_DUAL;
            format_info.attr.sbc.chann_location = AUDIO_CHANNEL_LOCATION_FL | AUDIO_CHANNEL_LOCATION_FR;
            break;
        case BT_A2DP_SBC_CHANNEL_MODE_STEREO:
            format_info.attr.sbc.chann_mode = AUDIO_SBC_CHANNEL_MODE_STEREO;
            format_info.attr.sbc.chann_location = AUDIO_CHANNEL_LOCATION_FL | AUDIO_CHANNEL_LOCATION_FR;
            break;
        case BT_A2DP_SBC_CHANNEL_MODE_JOINT_STEREO:
            format_info.attr.sbc.chann_mode = AUDIO_SBC_CHANNEL_MODE_JOINT_STEREO;
            format_info.attr.sbc.chann_location = AUDIO_CHANNEL_LOCATION_FL | AUDIO_CHANNEL_LOCATION_FR;
            break;
        }
        switch (stream_ind->codec_info.sbc.block_length)
        {
        case BT_A2DP_SBC_BLOCK_LENGTH_4:
            format_info.attr.sbc.block_length = 4;
            break;
        case BT_A2DP_SBC_BLOCK_LENGTH_8:
            format_info.attr.sbc.block_length = 8;
            break;
        case BT_A2DP_SBC_BLOCK_LENGTH_12:
            format_info.attr.sbc.block_length = 12;
            break;
        case BT_A2DP_SBC_BLOCK_LENGTH_16:
            format_info.attr.sbc.block_length = 16;
            break;
        }
        switch (stream_ind->codec_info.sbc.subbands)
        {
        case BT_A2DP_SBC_SUBBANDS_4:
            format_info.attr.sbc.subband_num = 4;
            break;
        case BT_A2DP_SBC_SUBBANDS_8:
            format_info.attr.sbc.subband_num = 8;
            break;
        }
        switch (stream_ind->codec_info.sbc.allocation_method)
        {
        case BT_A2DP_SBC_ALLOCATION_METHOD_LOUDNESS:
            format_info.attr.sbc.allocation_method = 0;
            break;
        case BT_A2DP_SBC_ALLOCATION_METHOD_SNR:
            format_info.attr.sbc.allocation_method = 1;
            break;
        }

        app_db.sampling_frequency = format_info.attr.sbc.sample_rate;
        p_link->a2dp_codec_info.sbc.channel_mode = format_info.attr.sbc.chann_mode;
        p_link->a2dp_codec_info.sbc.block_length = format_info.attr.sbc.block_length;
        p_link->a2dp_codec_info.sbc.subbands = format_info.attr.sbc.subband_num;
        p_link->a2dp_codec_info.sbc.allocation_method = format_info.attr.sbc.allocation_method;
    }
    else if (stream_ind->codec_type == BT_A2DP_CODEC_TYPE_AAC)
    {
        format_info.type = AUDIO_FORMAT_TYPE_AAC;
        format_info.frame_num = 1;
        switch (stream_ind->codec_info.aac.sampling_frequency)
        {
        case BT_A2DP_AAC_SAMPLING_FREQUENCY_8KHZ:
            format_info.attr.aac.sample_rate = 8000;
            break;
        case BT_A2DP_AAC_SAMPLING_FREQUENCY_16KHZ:
            format_info.attr.aac.sample_rate = 16000;
            break;

        case BT_A2DP_AAC_SAMPLING_FREQUENCY_44_1KHZ:
            format_info.attr.aac.sample_rate = 44100;
            break;

        case BT_A2DP_AAC_SAMPLING_FREQUENCY_48KHZ:
            format_info.attr.aac.sample_rate = 48000;
            break;

        case BT_A2DP_AAC_SAMPLING_FREQUENCY_96KHZ:
            format_info.attr.aac.sample_rate = 96000;
            break;
        default:
            break;
        }

        switch (stream_ind->codec_info.aac.channel_number)
        {
        case BT_A2DP_AAC_CHANNEL_NUMBER_1:
            format_info.attr.aac.chann_num = 1;
            format_info.attr.aac.chann_location = AUDIO_CHANNEL_LOCATION_MONO;
            break;

        case BT_A2DP_AAC_CHANNEL_NUMBER_2:
            format_info.attr.aac.chann_num = 2;
            format_info.attr.aac.chann_location = AUDIO_CHANNEL_LOCATION_FL | AUDIO_CHANNEL_LOCATION_FR;
            break;

        default:
            break;
        }
        format_info.attr.aac.transport_format = AUDIO_AAC_TRANSPORT_FORMAT_LATM;

        app_db.sampling_frequency = format_info.attr.aac.sample_rate;
        p_link->a2dp_codec_info.aac.channel_number = format_info.attr.aac.chann_num;
    }

    audio_a2dp_sink_track_release();

    p_link->a2dp_track_handle = audio_track_create(AUDIO_STREAM_TYPE_PLAYBACK,
                                                   mode,
                                                   stream,
                                                   format_info,
                                                   app_cfg_nv.audio_gain_level[pair_idx],
                                                   0,
                                                   AUDIO_DEVICE_OUT_SPK,
                                                   NULL,
                                                   NULL);


    if (p_link->a2dp_track_handle != NULL)
    {
        app_db.eq_instance = app_eq_create(EQ_CONTENT_TYPE_AUDIO, SPK_SW_EQ,
                                           app_db.spk_eq_mode, app_cfg_nv.eq_idx);

        app_db.audio_eq_enabled = false;
        if (app_db.eq_instance != NULL)
        {
            app_eq_audio_eq_enable(&app_db.eq_instance, &app_db.audio_eq_enabled);

            audio_track_effect_attach(p_link->a2dp_track_handle, app_db.eq_instance);
        }

        audio_track_latency_set(p_link->a2dp_track_handle, A2DP_LATENCY_MS, true);
        bt_a2dp_stream_delay_report_req(p_link->bd_addr, A2DP_LATENCY_MS);
        audio_track_start(p_link->a2dp_track_handle);
    }
}

void audio_a2dp_sink_data_ind(uint8_t *bd_addr, uint8_t *data_ind)
{
    T_APP_BR_LINK *p_link;
    p_link = app_find_br_link(bd_addr);
    if (p_link != NULL)
    {
        uint16_t written_len;
        T_BT_EVENT_PARAM_A2DP_STREAM_DATA_IND  *data = (T_BT_EVENT_PARAM_A2DP_STREAM_DATA_IND *)data_ind;
        audio_track_write(p_link->a2dp_track_handle, data->bt_clock,
                          data->seq_num,
                          AUDIO_STREAM_STATUS_CORRECT,
                          data->frame_num,
                          data->payload,
                          data->len,
                          &written_len);
    }
}


void audio_a2dp_sink_track_release(void)
{
    T_APP_BR_LINK *p_link;
    uint8_t active_a2dp_idx = app_a2dp_get_active_idx();

    p_link = &(app_db.br_link[active_a2dp_idx]);
    if (p_link != NULL)
    {
        if (p_link->a2dp_track_handle != NULL)
        {
            audio_track_release(p_link->a2dp_track_handle);
            p_link->a2dp_track_handle = NULL;
            p_link->avrcp_play_status = APP_AUDIO_STATE_STOP;
            //gui_update_by_event(GUI_EVENT_PLAYER, NULL, false);
        }
    }

    if (app_db.eq_instance != NULL)
    {
        eq_release(app_db.eq_instance);
        app_db.eq_instance = NULL;
    }

}

void audio_a2dp_sink_volume_up(void)
{
    uint8_t max_volume = 0;
    uint8_t curr_volume = 0;
    uint8_t active_idx;
    uint8_t pair_idx;
    uint8_t level;
    active_idx = app_a2dp_get_active_idx();
    if (bt_bond_index_get(app_db.br_link[active_idx].bd_addr, &pair_idx) == false)
    {
        APP_PRINT_ERROR0("app_volume_up: find active a2dp pair idx fail");
        return;
    }
    curr_volume = app_cfg_nv.audio_gain_level[pair_idx];
    max_volume = app_cfg_volume.playback_volume_max;

    if (curr_volume < max_volume)
    {
        curr_volume++;
    }
    else
    {
        curr_volume = max_volume;
    }

    level = (curr_volume * 0x7F + app_cfg_volume.playback_volume_max / 2) /
            app_cfg_volume.playback_volume_max;
    app_cfg_nv.audio_gain_level[pair_idx] = curr_volume;
    audio_track_volume_out_set(app_db.br_link[active_idx].a2dp_track_handle, curr_volume);
    bt_avrcp_volume_change_req(app_db.br_link[active_idx].bd_addr, level);
    //gui_update_by_event(GUI_EVENT_PLAYER, NULL, false);
}

void audio_a2dp_sink_volume_down(void)
{
    uint8_t min_volume = 0;
    uint8_t curr_volume = 0;
    uint8_t active_idx;
    uint8_t pair_idx;
    uint8_t level;
    active_idx = app_a2dp_get_active_idx();
    if (bt_bond_index_get(app_db.br_link[active_idx].bd_addr, &pair_idx) == false)
    {
        APP_PRINT_ERROR0("app_volume_up: find active a2dp pair idx fail");
        return;
    }
    curr_volume = app_cfg_nv.audio_gain_level[pair_idx];
    min_volume = app_cfg_volume.playback_volume_min;

    if (curr_volume > min_volume)
    {
        curr_volume--;
    }
    else
    {
        curr_volume = min_volume;
    }

    level = (curr_volume * 0x7F + app_cfg_volume.playback_volume_max / 2) /
            app_cfg_volume.playback_volume_max;
    app_cfg_nv.audio_gain_level[pair_idx] = curr_volume;
    audio_track_volume_out_set(app_db.br_link[active_idx].a2dp_track_handle, curr_volume);
    bt_avrcp_volume_change_req(app_db.br_link[active_idx].bd_addr, level);
    //gui_update_by_event(GUI_EVENT_PLAYER, NULL, false);
}

void audio_a2dp_sink_set_volume(uint8_t volume)
{
    uint8_t pair_idx;
    uint8_t active_idx;
    active_idx = app_a2dp_get_active_idx();

    if (bt_bond_index_get(app_db.br_link[active_idx].bd_addr, &pair_idx) == false)
    {
        APP_PRINT_ERROR0("abs volume set: find active a2dp pair idx fail");
        return;
    }

    app_cfg_nv.audio_gain_level[pair_idx] = (volume * app_cfg_volume.playback_volume_max \
                                             + 0x7F / 2) / 0x7F;
    APP_PRINT_INFO2("abs volume set, pair idx = %d, gain level = %d", pair_idx,
                    app_cfg_nv.audio_gain_level[pair_idx]);
    audio_track_volume_out_set(app_db.br_link[active_idx].a2dp_track_handle,
                               app_cfg_nv.audio_gain_level[pair_idx]);
    //gui_update_by_event(GUI_EVENT_PLAYER, NULL, false);
}
