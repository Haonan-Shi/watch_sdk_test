/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */


#include "stdlib.h"
#include "string.h"
#include "app_cmd.h"
#include "app_audio_route.h"
#include "trace.h"
#include "audio_cmd.h"
#include "audio_volume.h"
#include "audio_probe.h"

//for CMD_AUDIO_DSP_CTRL_SEND
#define CFG_H2D_DAC_GAIN                0x0C
#define CFG_H2D_VOICE_ADC_POST_GAIN     0x5D
#define CFG_H2D_APT_DAC_GAIN            0x4C

void app_audio_cmd_handle(uint8_t *cmd_ptr, uint16_t cmd_len, uint8_t cmd_path, uint8_t app_idx,
                          uint8_t *ack_pkt)
{
    APP_PRINT_INFO1("app_audio_cmd_handle: %b", TRACE_BINARY(cmd_len, cmd_ptr));
    uint16_t cmd_id = (uint16_t)(cmd_ptr[0] | (cmd_ptr[1] << 8));

    switch (cmd_id)
    {
    case CMD_AUDIO_DSP_CTRL_SEND:
        {
            uint8_t *buf;
            T_AUDIO_STREAM_TYPE stream_type;
            T_DSP_TOOL_GAIN_LEVEL_DATA *gain_level_data;
            bool send_cmd_flag = true;

            buf = malloc(cmd_len - 2);
            if (buf == NULL)
            {
                return;
            }

            memcpy(buf, &cmd_ptr[2], (cmd_len - 2));

            gain_level_data = (T_DSP_TOOL_GAIN_LEVEL_DATA *)buf;

            app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);

            if (gain_level_data->cmd_id == CFG_H2D_DAC_GAIN ||
                gain_level_data->cmd_id == CFG_H2D_VOICE_ADC_POST_GAIN)
            {
                switch (gain_level_data->category)
                {
                case AUDIO_CATEGORY_TONE:
                case AUDIO_CATEGORY_VP:
                case AUDIO_CATEGORY_AUDIO:
                    {
                        stream_type = AUDIO_STREAM_TYPE_PLAYBACK;
                    }
                    break;

                case AUDIO_CATEGORY_APT:
                case AUDIO_CATEGORY_LLAPT:
                case AUDIO_CATEGORY_ANC:
                case AUDIO_CATEGORY_LINE:
                case AUDIO_CATEGORY_VOICE:
                    {
                        stream_type = AUDIO_STREAM_TYPE_VOICE;
                    }
                    break;

                case AUDIO_CATEGORY_VAD:
                case AUDIO_CATEGORY_RECORD:
                    {
                        stream_type = AUDIO_STREAM_TYPE_RECORD;
                    }
                    break;

                default:
                    {
                        stream_type = AUDIO_STREAM_TYPE_PLAYBACK;
                    }
                    break;
                }

                if (gain_level_data->cmd_id == CFG_H2D_DAC_GAIN)
                {
                    app_audio_route_dac_gain_set((T_AUDIO_CATEGORY)gain_level_data->category, gain_level_data->level,
                                                 gain_level_data->gain);
                    audio_volume_out_set(stream_type, gain_level_data->level);
                }
                else
                {
                    app_audio_route_adc_gain_set((T_AUDIO_CATEGORY)gain_level_data->category, gain_level_data->level,
                                                 gain_level_data->gain);
                    if (((T_AUDIO_CATEGORY)gain_level_data->category != AUDIO_CATEGORY_APT) &&
                        ((T_AUDIO_CATEGORY)gain_level_data->category != AUDIO_CATEGORY_LLAPT))
                    {
                        audio_volume_in_set(stream_type, gain_level_data->level);
                    }
                }
                send_cmd_flag = false;
            }

            APP_PRINT_TRACE2("app_audio_cmd_handle send_cmd_flag %d, buf %b", send_cmd_flag,
                             TRACE_BINARY(cmd_len - 2, buf));
            if (send_cmd_flag)
            {
                audio_probe_dsp_send(buf, cmd_len - 2);
            }
            free(buf);
        }
        break;

    default:
        break;
    }
}


