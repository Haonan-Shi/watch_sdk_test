/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "trace.h"
#include "audio_type.h"
#include "vad.h"
#include "kws.h"
#include "app_audio_demo_vad.h"

static T_VAD_SM_STATE   vad_state = VAD_STOPPED;
T_AUDIO_EFFECT_INSTANCE kws;

bool app_audio_vad_enable(uint8_t  vad_type,
                          uint8_t  bit_width,
                          uint16_t frame_length)
{
    if (vad_state != VAD_STARTED)
    {
        T_AUDIO_FORMAT_INFO        format_info;
        T_VAD_AGGRESSIVENESS_LEVEL aggressiveness_level;

        vad_state = VAD_STARTING;
        aggressiveness_level = VAD_AGGRESSIVENESS_LEVEL_LOW;

        if (vad_type == VAD_TYPE_SW)
        {
            format_info.type = AUDIO_FORMAT_TYPE_SBC;
            format_info.frame_num = 1;
            format_info.attr.sbc.sample_rate = 16000;
            format_info.attr.sbc.chann_mode = AUDIO_SBC_CHANNEL_MODE_JOINT_STEREO;
            format_info.attr.sbc.chann_location = AUDIO_CHANNEL_LOCATION_FL | AUDIO_CHANNEL_LOCATION_FR;
            format_info.attr.sbc.block_length = 16;
            format_info.attr.sbc.subband_num = 8;
            format_info.attr.sbc.allocation_method = 0;
            format_info.attr.sbc.bitpool = 0x22;
        }
        else
        {
            format_info.type = AUDIO_FORMAT_TYPE_PCM;
            format_info.attr.pcm.bit_width = bit_width;
            format_info.attr.pcm.chann_num = 1;
            format_info.attr.pcm.frame_length = frame_length;
            format_info.attr.pcm.sample_rate = 16000;
            format_info.attr.pcm.chann_location = AUDIO_CHANNEL_LOCATION_MONO;
        }

        vad_enable((T_VAD_TYPE)vad_type, aggressiveness_level, format_info);
        vad_filter(VAD_FILTER_LEVEL_DEFAULT);

        kws = kws_effect_create(KWS_EFFECT_CONTENT_TYPE_VAD);
        vad_effect_attach(kws);
        kws_effect_enable(kws);
        return true;
    }

    return false;
}

bool app_audio_vad_disable(void)
{
    if (vad_state != VAD_STOPPED)
    {
        vad_state = VAD_STOPPING;
        kws_effect_disable(kws);
        vad_effect_detach(kws);
        kws_effect_release(kws);
        vad_disable();
    }

    return false;
}
