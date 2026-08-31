/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */



#if CONFIG_REALTEK_APP_AI_RECORD
#include "string.h"
#include <stdlib.h>
#include "app_cfg.h"
#include "app_main.h"
#include "audio_track.h"
#include "trace.h"
#include "ring_buffer.h"
#include "pm.h"
#include "app_a2dp.h"
#include "ai_record.h"
#include "app_ai_record_process.h"
#include "app_ai_record.h"
#include "app_dsp_cfg.h"
#include "nrec.h"
#include "bt_types.h"
#include "app_flags.h"


#define AI_RECORD_NUM_OF_PKT           (5)
#define AI_RECORD_NUM_OF_PKT_TO_SEND   (1)
#define AI_RECORD_VOICE_FRAME_SIZE     (40)
#define AI_RECORD_VOICE_BUFFER_SIZE    (AI_RECORD_VOICE_FRAME_SIZE*AI_RECORD_NUM_OF_PKT)

typedef enum _T_AI_AI_RECORD_AUDIO_FORMAT
{
    AI_AI_RECORD_OPUS_16KHZ_16KBPS_CBR_0_20MS,
} T_AI_AI_RECORD_AUDIO_FORMAT;

typedef struct
{
    bool is_start;
    uint8_t bd_addr[BD_ADDR_LENGTH];
    T_AUDIO_TRACK_HANDLE handle;
    T_AUDIO_EFFECT_INSTANCE record_eq_instance;
    T_AUDIO_EFFECT_INSTANCE nrec_instance;
    uint8_t format_data[AI_RECORD_APP_EVENT_LEN_VOICE_START_INFO];
    uint8_t num_voice_buf;
    uint8_t num_start_send;
    uint8_t *p_buf;
    uint8_t *tmp_pkt_buf;
    T_RING_BUFFER voice_buf;
} T_APP_AI_RECORD_AI_PROC;

static T_APP_AI_RECORD_AI_PROC ai_record_proc_db = {0};

extern uint32_t BB_read_native_clock(void);

static void ai_record_voice_gen_format_info(T_AUDIO_FORMAT_INFO *p_format_info,
                                            T_AI_AI_RECORD_AUDIO_FORMAT audio_format)
{
    switch (audio_format)
    {
    case AI_AI_RECORD_OPUS_16KHZ_16KBPS_CBR_0_20MS:
        p_format_info->type = AUDIO_FORMAT_TYPE_OPUS;
        p_format_info->frame_num = 1;
        p_format_info->attr.opus.sample_rate = 16 * 1000;
        p_format_info->attr.opus.chann_num = 1;
        p_format_info->attr.opus.cbr = 1;
        p_format_info->attr.opus.cvbr = 0;
        p_format_info->attr.opus.mode = 3;
        p_format_info->attr.opus.complexity = 3;
        p_format_info->attr.opus.frame_duration = AUDIO_OPUS_FRAME_DURATION_20_MS;
        p_format_info->attr.opus.bitrate = 16 * 1000;
        break;

    default:
        APP_PRINT_ERROR0("gen_format_info: only support AUDIO_OPUS_FRAME_DURATION_20_MS");
        break;
    }
}

static bool app_ai_record_process_voice_data(uint8_t bd_addr[6], uint8_t *data, uint16_t len)
{
    if (ring_buffer_write(&ai_record_proc_db.voice_buf, data, len))
    {
        ai_record_proc_db.num_voice_buf++;
    }
    else
    {
        APP_PRINT_ERROR0("app_ai_record_process_voice_data:voice_buf full");
    }

    if (ai_record_proc_db.num_voice_buf == AI_RECORD_NUM_OF_PKT)
    {
        ai_record_proc_db.num_voice_buf = 0;
        ai_record_proc_db.num_start_send++;
    }

    if (ai_record_proc_db.num_start_send == AI_RECORD_NUM_OF_PKT_TO_SEND)
    {
        uint16_t data_len = AI_RECORD_VOICE_BUFFER_SIZE * AI_RECORD_NUM_OF_PKT_TO_SEND;
        uint16_t param_len = data_len + 2;
        memset(ai_record_proc_db.tmp_pkt_buf, 0, param_len);
        memcpy(ai_record_proc_db.tmp_pkt_buf, &data_len, 2);
        uint32_t actual_len = ring_buffer_read(&ai_record_proc_db.voice_buf,
                                               AI_RECORD_VOICE_BUFFER_SIZE * AI_RECORD_NUM_OF_PKT_TO_SEND, ai_record_proc_db.tmp_pkt_buf + 2);
        ai_record_cmd_ai_voice_data_send(ai_record_proc_db.tmp_pkt_buf, param_len);
        APP_PRINT_INFO1("app_ai_record_process_voice_data: actual_len %d", actual_len);

        ai_record_proc_db.num_start_send = 0;
    }

    return true;
}

static bool app_ai_record_record_read_cb(T_AUDIO_TRACK_HANDLE   handle,
                                         uint32_t              *timestamp,
                                         uint16_t              *seq_num,
                                         T_AUDIO_STREAM_STATUS *status,
                                         uint8_t               *frame_num,
                                         void                  *buf,
                                         uint16_t               required_len,
                                         uint16_t              *actual_len)
{
    APP_PRINT_TRACE1("app_ai_record_record_read_cb: required_len %d", required_len);

    if (required_len != AI_RECORD_VOICE_FRAME_SIZE)
    {
        APP_PRINT_ERROR0("app_ai_record_record_read_cb required_len is incorrect");
    }
    else
    {
        uint8_t app_idx = app_a2dp_get_active_idx();

        if (false == app_ai_record_process_voice_data(app_db.br_link[app_idx].bd_addr, (uint8_t *)buf,
                                                      required_len))
        {
            return false;
        }
    }

    *actual_len = required_len;

    return true;
}

void app_ai_record_stop_recording(uint8_t bd_addr[6], bool is_ext)
{
    if (ai_record_proc_db.is_start != true)
    {
        APP_PRINT_ERROR0("app_ai_record_stop_recording: already stopped!");
        return;
    }

    APP_PRINT_INFO1("app_ai_record_stop_recording: is_ext %d", is_ext);

    ai_record_cmd_ai_voice_local_complete(0);
    ai_record_proc_db.is_start = false;

    audio_track_stop(ai_record_proc_db.handle);


    bt_power_mode_set(BTPOWER_DEEP_SLEEP);

}

void app_ai_record_start_recording(uint8_t bd_addr[6], bool is_ext)
{
    if (ai_record_proc_db.is_start != false) /*g_voice_data.is_voice_start == false8*/
    {
        APP_PRINT_ERROR0("app_ai_record_start_recording: already started");
        return;
    }

    APP_PRINT_INFO1("app_ai_record_start_recording: is_ext %d", is_ext);

    ai_record_cmd_ai_voice_start(ai_record_proc_db.format_data, sizeof(ai_record_proc_db.format_data));

    bt_power_mode_set(BTPOWER_ACTIVE);
    extern uint32_t BB_read_native_clock(void);
    BB_read_native_clock();

    ai_record_proc_db.is_start = true;
    memcpy(ai_record_proc_db.bd_addr, bd_addr, sizeof(ai_record_proc_db.bd_addr));

    audio_track_start(ai_record_proc_db.handle);

}

bool app_ai_record_is_recording(void)
{
    return ai_record_proc_db.is_start;
}

void app_ai_record_process_init(void)
{
    T_AUDIO_FORMAT_INFO format_info;

    memset(&ai_record_proc_db, 0, sizeof(T_APP_AI_RECORD_AI_PROC));

    ai_record_voice_gen_format_info(&format_info, AI_AI_RECORD_OPUS_16KHZ_16KBPS_CBR_0_20MS);

    LE_UINT8_TO_ARRAY(ai_record_proc_db.format_data, format_info.type);
    LE_UINT32_TO_ARRAY(ai_record_proc_db.format_data + 1, format_info.attr.opus.sample_rate);
    LE_UINT8_TO_ARRAY(ai_record_proc_db.format_data + 5, format_info.attr.opus.chann_num);
    LE_UINT8_TO_ARRAY(ai_record_proc_db.format_data + 6, format_info.attr.opus.cbr);
    LE_UINT8_TO_ARRAY(ai_record_proc_db.format_data + 7, format_info.attr.opus.cvbr);
    LE_UINT8_TO_ARRAY(ai_record_proc_db.format_data + 8, format_info.attr.opus.mode);
    LE_UINT8_TO_ARRAY(ai_record_proc_db.format_data + 9, format_info.attr.opus.complexity);
    LE_UINT8_TO_ARRAY(ai_record_proc_db.format_data + 10, format_info.attr.opus.frame_duration);
    LE_UINT32_TO_ARRAY(ai_record_proc_db.format_data + 11, format_info.attr.opus.bitrate);

    ai_record_proc_db.handle = audio_track_create(AUDIO_STREAM_TYPE_RECORD,
                                                  AUDIO_STREAM_MODE_NORMAL,
                                                  AUDIO_STREAM_USAGE_LOCAL,
                                                  format_info,
                                                  0,
                                                  10,
                                                  AUDIO_DEVICE_IN_MIC,
                                                  NULL,
                                                  app_ai_record_record_read_cb);
    if (ai_record_proc_db.handle != NULL)
    {
        ai_record_proc_db.record_eq_instance = app_eq_create(EQ_CONTENT_TYPE_RECORD, EQ_STREAM_TYPE_RECORD,
                                                             MIC_SW_EQ, VOICE_MIC_MODE, 0);
        if (ai_record_proc_db.record_eq_instance)
        {
            eq_enable(ai_record_proc_db.record_eq_instance);
            audio_track_effect_attach(ai_record_proc_db.handle, ai_record_proc_db.record_eq_instance);
        }

        ai_record_proc_db.nrec_instance = nrec_create(NREC_CONTENT_TYPE_RECORD,
                                                      NREC_MODE_HIGH_SOUND_QUALITY, 0);
        if (ai_record_proc_db.nrec_instance)
        {
            nrec_enable(ai_record_proc_db.nrec_instance);
            audio_track_effect_attach(ai_record_proc_db.handle, ai_record_proc_db.nrec_instance);
        }
    }

    ai_record_proc_db.p_buf = calloc(1, AI_RECORD_VOICE_BUFFER_SIZE * AI_RECORD_NUM_OF_PKT_TO_SEND + 1);
    ai_record_proc_db.num_voice_buf = 0;
    ring_buffer_init(&ai_record_proc_db.voice_buf, ai_record_proc_db.p_buf,
                     AI_RECORD_VOICE_BUFFER_SIZE * AI_RECORD_NUM_OF_PKT_TO_SEND + 1);
    ai_record_proc_db.tmp_pkt_buf = calloc(1,
                                           AI_RECORD_VOICE_BUFFER_SIZE * AI_RECORD_NUM_OF_PKT_TO_SEND);
}

#endif
