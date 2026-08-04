/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "os_sync.h"
#include "clock_manager.h"
#include "wdg.h"
#include "trace.h"
#if CONFIG_OPUS
#include "rtk_opus.h"
#endif
#include "audio_track.h"
#include "audio.h"
#include "trace.h"
#include "os_mem.h"
#include "os_queue.h"
#include "walkie_talkie_adv.h"
#include "walkie_talkie_gatt_svc.h"
#include "walkie_talkie_gatt_client.h"
#include "walkie_talkie_voice.h"
#include "walkie_talkie_app.h"
#include "low_stack.h"
#include "pm.h"
#include "codec2.h"

typedef void(*P_FUN_ENCODER_CREATE)(void);
typedef int(*P_FUN_ENCODER)(uint16_t *pcm_in, uint8_t *out, int *out_len);
typedef void(*P_FUN_ENCODER_DESTROY)(void);
typedef void(*P_FUN_DECODER_CREATE)(void);
typedef int(*P_FUN_DECODER)(uint8_t *in, int in_len, uint8_t *pcm_out, int *out_samples);
typedef void(*P_FUN_DECODER_DESTROY)(void);


typedef struct t_voice_packet
{
    struct t_voice_packet     *p_next;
    uint16_t   payload_length;
    uint16_t   frame_num;
    uint8_t    p_data[0];
} T_VOICE_PACKET;

typedef enum
{
    WALKIE_TALKIE_OPUS,
    WALKIE_TALKIE_CODEC2,
} WALKIE_TALKIE_CODEC_TYPE;

typedef struct
{
    WALKIE_TALKIE_CODEC_TYPE  codec;
    int samples;
    int bitrate;
    int sample_rate;
    int encode_bytes;
    P_FUN_ENCODER_CREATE encoder_create_cb;
    P_FUN_ENCODER encoder_cb;
    P_FUN_ENCODER_DESTROY encoder_destroy_cb;
    P_FUN_DECODER_CREATE decoder_create_cb;
    P_FUN_DECODER decoder_cb;
    P_FUN_DECODER_DESTROY decoder_destroy_cb;
} WALKIE_TALKIE_CODEC_PARAM;



/********************************************************************************************************
* local static variables defined here, only used in this source file.
********************************************************************************************************/
#if CONFIG_OPUS
void *enc_handle  = NULL;
void *dec_handle = NULL;
rtk_opus_param_t param;
#endif

struct CODEC2 *codec2 = NULL;

WALKIE_TALKIE_CODEC_PARAM codec_param =
{
    .codec = WALKIE_TALKIE_CODEC2,
};

T_AUDIO_TRACK_HANDLE walkie_talkie_play_hdl = NULL;
uint8_t *voice_data_buf = NULL;

T_AUDIO_TRACK_HANDLE walkie_talkie_record_hdl = NULL;
T_OS_QUEUE voice_data_queue;
static uint8_t voice_seq = 0;

static BtPowerMode bt_pwr_state = BTPOWER_ACTIVE;

/********************************************************************************************************
* opus encode and decode apis defined here
********************************************************************************************************/
#if CONFIG_OPUS
void walkie_talkie_opus_encoder_create(void)
{
    if (enc_handle)
    {
        APP_PRINT_INFO0("opus encoder already exist");
        return;
    }
    // Set encoding parameters
    rtk_opus_param_set_default(&param);
    param.sampling_rate = codec_param.sample_rate;
    param.channels = 1;
    param.bitrate_bps = codec_param.bitrate;
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

int walkie_talkie_opus_encoder(uint16_t *pcm_in, uint8_t *out, int *out_len)
{
    int res = 0;
    if (enc_handle)
    {
        res = rtk_celt_encode(enc_handle, &param, pcm_in, out, out_len);
    }

    return res;
}

void walkie_talkie_opus_encoder_destroy(void)
{
    if (enc_handle)
    {
        rtk_celt_encoder_destroy(enc_handle);
        enc_handle = NULL;
    }
}

void walkie_talkie_opus_decoder_create(void)
{
    if (dec_handle)
    {
        APP_PRINT_INFO0("opus decoder already exist");
        return;
    }
    // Set encoding parameters
    rtk_opus_param_set_default(&param);
    param.sampling_rate = codec_param.sample_rate;
    param.channels = 1;
    param.bitrate_bps = codec_param.bitrate;
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

    dec_handle = rtk_celt_decoder_init(&param);
    if (!dec_handle)
    {
        DBG_DIRECT("rtk_decode_init failed!\n");
        return;
    }
}

int walkie_talkie_opus_decoder(uint8_t *in, int in_len, uint8_t *pcm_out, int *out_samples)
{
    int res = 0;
    if (dec_handle)
    {
        res = rtk_celt_decode(dec_handle, &param, in, in_len, (short *)pcm_out, out_samples);
    }

    return res;
}

void walkie_talkie_opus_decoder_destroy(void)
{
    if (dec_handle)
    {
        rtk_celt_decoder_destroy(dec_handle);
        dec_handle = NULL;
    }
}
#endif

/********************************************************************************************************
* codec2 encode and decode apis defined here
********************************************************************************************************/

void *codec2_malloc(size_t size)
{
    return malloc(size);
}

void *codec2_calloc(size_t nmemb, size_t size)
{
    return calloc(nmemb, size);
}

void codec2_free(void *ptr)
{
    free(ptr);
}

void walkie_talkie_codec2_create(void)
{
    codec2 = codec2_create(codec_param.bitrate);
    codec_param.samples = codec2_samples_per_frame(codec2);
    codec_param.encode_bytes = codec2_bytes_per_frame(codec2);
    APP_PRINT_INFO2("codec2 samples pre frame nsam %d, encode bytes nenc %d", codec_param.samples,
                    codec_param.encode_bytes);
}

void walkie_talkie_codec2_destroy(void)
{
    if (codec2)
    {
        codec2_destroy(codec2);
    }
}


int walkie_talkie_codec2_encoder(uint16_t *pcm_in, uint8_t *out, int *out_len)
{
    if (codec2)
    {
        codec2_encode(codec2, out, (short *)pcm_in);
        *out_len = codec2_bytes_per_frame(codec2);
        return 0;
    }
    return -1;
}

int walkie_talkie_codec2_decoder(uint8_t *in, int in_len, uint8_t *pcm_out, int *out_samples)
{
    if (codec2)
    {
        codec2_decode(codec2, (short *)pcm_out, in);
        *out_samples = codec2_samples_per_frame(codec2);
        return 0;
    }
    return -1;
}

/********************************************************************************************************
* codec type select and init
********************************************************************************************************/
void walkie_talkie_codec_init(void)
{
    switch (codec_param.codec)
    {
    case WALKIE_TALKIE_CODEC2:
        {
            codec_param.encoder_create_cb = walkie_talkie_codec2_create;
            codec_param.encoder_cb = walkie_talkie_codec2_encoder;
            codec_param.encoder_destroy_cb = walkie_talkie_codec2_destroy;
            codec_param.decoder_create_cb = walkie_talkie_codec2_create;
            codec_param.decoder_cb = walkie_talkie_codec2_decoder;
            codec_param.decoder_destroy_cb = walkie_talkie_codec2_destroy;
        }
        break;
#if CONFIG_OPUS
    case WALKIE_TALKIE_OPUS:
        {
            codec_param.encoder_create_cb = walkie_talkie_opus_encoder_create;
            codec_param.encoder_cb = walkie_talkie_opus_encoder;
            codec_param.encoder_destroy_cb = walkie_talkie_opus_encoder_destroy;
            codec_param.decoder_create_cb = walkie_talkie_opus_decoder_create;
            codec_param.decoder_cb = walkie_talkie_opus_decoder;
            codec_param.decoder_destroy_cb = walkie_talkie_opus_decoder_destroy;
        }
        break;
#endif
    default:
        break;
    }
}

void walkie_talkie_encoder_create(void)
{
    switch (codec_param.codec)
    {
    case WALKIE_TALKIE_CODEC2:
        {
            codec_param.sample_rate = 8000;
            codec_param.bitrate = CODEC2_MODE_2400;
        }
        break;
#if CONFIG_OPUS
    case WALKIE_TALKIE_OPUS:
        {
            codec_param.sample_rate = 8000;
            codec_param.bitrate = 8000;
            codec_param.samples = codec_param.sample_rate / 50;
            codec_param.encode_bytes = codec_param.bitrate / 8 / 50;
        }
        break;
#endif
    default:
        break;
    }
    if (codec_param.encoder_create_cb)
    {
        codec_param.encoder_create_cb();
    }
}

int walkie_talkie_encoder(uint16_t *pcm_in, uint8_t *out, int *out_len)
{
    if (codec_param.encoder_cb)
    {
        return codec_param.encoder_cb(pcm_in, out, out_len);
    }

    return -1;
}

void walkie_talkie_encoder_destroy(void)
{
    if (codec_param.encoder_destroy_cb)
    {
        codec_param.encoder_destroy_cb();
    }
}

void walkie_talkie_decoder_create(void)
{
    switch (codec_param.codec)
    {
    case WALKIE_TALKIE_CODEC2:
        {
            codec_param.sample_rate = 8000;
            codec_param.bitrate = CODEC2_MODE_2400;
        }
        break;
#if CONFIG_OPUS
    case WALKIE_TALKIE_OPUS:
        {
            codec_param.sample_rate = 8000;
            codec_param.bitrate = 8000;
            codec_param.samples = codec_param.sample_rate / 50;
            codec_param.encode_bytes = codec_param.bitrate / 8 / 50;
        }
        break;
#endif
    default:
        break;
    }
    if (codec_param.decoder_create_cb)
    {
        codec_param.decoder_create_cb();
    }
}

int walkie_talkie_decoder(uint8_t *in, int in_len, uint8_t *pcm_out, int *out_samples)
{
    if (codec_param.decoder_create_cb)
    {
        return codec_param.decoder_cb(in, in_len, pcm_out, out_samples);
    }
    return -1;
}

void walkie_talkie_decoder_destroy(void)
{
    if (codec_param.decoder_destroy_cb)
    {
        codec_param.decoder_destroy_cb();
    }
}


/********************************************************************************************************
* voice player apis defined here
********************************************************************************************************/

void walkie_talkie_player_data_parser(uint8_t *p_data, uint16_t length)
{
    APP_PRINT_INFO1("voice num = %d", p_data[VOICE_NUM_OFFSET]);
    static uint8_t voice_data_idx = 0;

    if (voice_data_idx != p_data[VOICE_DATA_OFFSET]) //new data
    {
        // walkie_talkie_scan_lost_analyzer(voice_data_idx, p_data[VOICE_DATA_OFFSET], p_data[VOICE_NUM_OFFSET]);
        voice_data_idx = p_data[VOICE_DATA_OFFSET];
        APP_PRINT_INFO2("voice_data_buf = 0x%x, num = %d", voice_data_buf, p_data[VOICE_NUM_OFFSET]);
        if (voice_data_buf == NULL)
        {
            return;
        }
        for (uint8_t i = 0; i < p_data[VOICE_NUM_OFFSET]; i++)
        {
            int out_samples = 0;
            walkie_talkie_decoder(p_data + VOICE_DATA_OFFSET + (codec_param.encode_bytes + 1) * i + 1, \
                                  codec_param.encode_bytes, voice_data_buf,
                                  &out_samples);
            APP_PRINT_INFO1("out_samples = %d", out_samples);
            walkie_talkie_player_data_write(voice_data_buf, out_samples * 2, 1);
        }
    }
}

void walkie_talkie_player_start(void)
{

    if (walkie_talkie_play_hdl != NULL)
    {
        APP_PRINT_ERROR0("walkie_talkie_player: already playing");
        return;
    }
    bt_pwr_state = bt_power_mode_get();
    bt_power_mode_set(BTPOWER_ACTIVE);

    T_AUDIO_FORMAT_INFO format_info;
    format_info.type = AUDIO_FORMAT_TYPE_PCM;
    format_info.attr.pcm.sample_rate = codec_param.sample_rate;
    format_info.attr.pcm.bit_width = 16;
    format_info.attr.pcm.chann_num = 1;
    format_info.attr.pcm.frame_length = codec_param.samples * 2;
    format_info.attr.pcm.chann_location = AUDIO_CHANNEL_LOCATION_MONO;

    voice_data_buf = os_mem_alloc(RAM_TYPE_DATA_ON, format_info.attr.pcm.frame_length);
    if (voice_data_buf == NULL)
    {
        return;
    }

    walkie_talkie_play_hdl = audio_track_create(AUDIO_STREAM_TYPE_PLAYBACK,
                                                AUDIO_STREAM_MODE_NORMAL,
                                                AUDIO_STREAM_USAGE_LOCAL,
                                                format_info,
                                                15,
                                                0,
                                                AUDIO_DEVICE_OUT_SPK,
                                                NULL,
                                                NULL);
    audio_track_latency_set(walkie_talkie_play_hdl, 300, true);

    if (walkie_talkie_play_hdl == NULL)
    {
        APP_PRINT_ERROR0("walkie_talkie_player: handle is NULL");
        return;
    }

    if (audio_track_start(walkie_talkie_play_hdl))
    {
        APP_PRINT_ERROR0("walkie_talkie_player: audio_track_start success");
    }
    else
    {
        APP_PRINT_ERROR0("walkie_talkie_player: audio_track_start fail");
    }
}

void walkie_talkie_player_stop(void)
{
    APP_PRINT_INFO0("walkie_talkie_player_stop");

    bt_power_mode_set(bt_pwr_state);
    if (walkie_talkie_play_hdl)
    {
        audio_track_release(walkie_talkie_play_hdl);
        walkie_talkie_play_hdl = NULL;
    }
    if (voice_data_buf)
    {
        os_mem_free(voice_data_buf);
        voice_data_buf = NULL;
    }
}

void walkie_talkie_player_data_write(uint8_t *buf, uint16_t len, uint16_t frame_num)
{
    uint32_t bb_clock_slot;
    uint16_t bb_clock_us;
    static uint16_t  seq_num = 0;
    uint16_t written_len;
    seq_num++;
    lc_get_high_dpi_native_clock(&bb_clock_slot, &bb_clock_us);
    audio_track_write(walkie_talkie_play_hdl, bb_clock_slot,
                      seq_num,
                      AUDIO_STREAM_STATUS_CORRECT,
                      frame_num,
                      buf,
                      len,
                      &written_len);
    APP_PRINT_INFO2("walkie_talkie_player data write require len = %d, actual len = %d", len,
                    written_len);
}

/********************************************************************************************************
* voice recoder apis defined here
********************************************************************************************************/

void *voice_peek(int offset)
{
    void *p_voice_pkt = os_queue_peek(&voice_data_queue, offset);
    return p_voice_pkt;
}

uint8_t voice_flush(uint16_t cnt)
{
    uint16_t i;
    T_VOICE_PACKET *p_voice_pkt;
    AUDIO_PRINT_TRACE1("voice queue flush: %d", cnt);
    if (cnt > voice_data_queue.count)
    {
        cnt = voice_data_queue.count;
    }
    for (i = 0; i < cnt; i++)
    {
        p_voice_pkt = (T_VOICE_PACKET *)os_queue_out(&voice_data_queue);
        os_mem_free(p_voice_pkt);
    }
    return 0;
}

bool voice_data_in(uint8_t *packet_pt, uint16_t packet_length, uint16_t frame_num)
{
    if (packet_pt == NULL)
    {
        return false;
    }

    T_VOICE_PACKET voice_head;
    T_VOICE_PACKET *voice_pkt;

    voice_head.p_next = NULL;
    voice_head.payload_length = packet_length;
    voice_head.frame_num = frame_num;

    voice_pkt = (T_VOICE_PACKET *)os_mem_alloc(RAM_TYPE_DATA_ON,
                                               sizeof(T_VOICE_PACKET) + packet_length);

    if (voice_pkt == NULL)
    {
        AUDIO_PRINT_ERROR0("voice pkt get buffer error");
        return false;
    }
    memcpy(voice_pkt, &voice_head, sizeof(T_VOICE_PACKET));
    memcpy((uint8_t *)voice_pkt + sizeof(T_VOICE_PACKET), packet_pt, packet_length);

    os_queue_in(&voice_data_queue, voice_pkt);

    return true;
}

bool walkie_talkie_recorder_read_cb(T_AUDIO_TRACK_HANDLE  handle,
                                    uint32_t             *timestamp,
                                    uint16_t             *seq_num,
                                    T_AUDIO_STREAM_STATUS *status,
                                    uint8_t              *frame_num,
                                    void                 *buf,
                                    uint16_t              required_len,
                                    uint16_t             *actual_len)
{
    APP_PRINT_TRACE4("walkie_talkie_recorder_read_cb: buf 0x%08x, required_len %d seq_num %d frame_num %d",
                     buf,
                     required_len, *seq_num, *frame_num);
    if (handle == walkie_talkie_record_hdl)
    {
        uint8_t out[45];
        int out_len = 0;

        out[0] = voice_seq++;
        walkie_talkie_encoder(buf, &out[1], &out_len);
        voice_data_in(out, out_len + 1, 1);
        APP_PRINT_INFO1("walkie talkie encode out_len = %d", out_len);

        if (walkie_talkie_cfg.mode == WALKIE_TALKIE_CONN)
        {
            if ((voice_data_queue.count >= 8) && (transmit_adv.role != GAP_LINK_ROLE_UNDEFINED))
            {
                APP_PRINT_INFO1("send data queue count = %d", voice_data_queue.count);
                uint16_t cid;
                uint8_t cid_num;
                uint16_t conn_handle = le_get_conn_handle(transmit_adv.conn_id);
                gap_chann_get_cid(conn_handle, 1, &cid, &cid_num);

                walkie_talkie_recorder_set_voice_data();

                if (transmit_adv.role == GAP_LINK_ROLE_SLAVE)
                {
                    wts_send_voice_data_notify(conn_handle, cid, transmit_adv_data,
                                               VOICE_DATA_OFFSET + transmit_adv_data[VOICE_NUM_OFFSET] * (codec_param.encode_bytes + 1));
                }
                else if (transmit_adv.role == GAP_LINK_ROLE_MASTER)
                {
                    wts_client_write_voice_data(conn_handle, transmit_adv_data,
                                                VOICE_DATA_OFFSET + transmit_adv_data[VOICE_NUM_OFFSET] * (codec_param.encode_bytes + 1));
                }
            }
        }

    }

    *actual_len = required_len;
    return true;
}

void walkie_talkie_recorder_start(void)
{
    if (walkie_talkie_record_hdl != NULL)
    {
        APP_PRINT_ERROR0("walkie_talkie_recorder_start: already recording");
        return;
    }
    voice_seq = 0;
    T_AUDIO_FORMAT_INFO format_info;
    format_info.type = AUDIO_FORMAT_TYPE_PCM;
    format_info.attr.pcm.sample_rate = codec_param.sample_rate;
    format_info.attr.pcm.bit_width = 16;
    format_info.attr.pcm.chann_num = 1;
    format_info.attr.pcm.frame_length = codec_param.samples * 2;
    format_info.attr.pcm.chann_location = AUDIO_CHANNEL_LOCATION_MONO;

    walkie_talkie_record_hdl = audio_track_create(AUDIO_STREAM_TYPE_RECORD,
                                                  AUDIO_STREAM_MODE_NORMAL,
                                                  AUDIO_STREAM_USAGE_LOCAL,
                                                  format_info,
                                                  0,
                                                  15,
                                                  AUDIO_DEVICE_IN_MIC,
                                                  NULL,
                                                  walkie_talkie_recorder_read_cb);

    if (walkie_talkie_record_hdl == NULL)
    {
        APP_PRINT_ERROR0("walkie_talkie_recorder_start: handle is NULL");
        return;
    }

    if (audio_track_start(walkie_talkie_record_hdl))
    {
        APP_PRINT_ERROR0("walkie_talkie_recorder_start: audio_track_start success");
    }
    else
    {
        APP_PRINT_ERROR0("walkie_talkie_recorder_start: audio_track_start fail");
    }
}

void walkie_talkie_recorder_stop(void)
{
    if (walkie_talkie_record_hdl == NULL)
    {
        APP_PRINT_ERROR0("walkie_talkie_recorder_stop: already stopped!");
        return;
    }

    APP_PRINT_TRACE0("walkie_talkie_recorder_stop");
    audio_track_release(walkie_talkie_record_hdl);
    walkie_talkie_record_hdl = NULL;
    voice_flush(voice_data_queue.count);
}

void walkie_talkie_recorder_set_voice_data(void)
{
    transmit_adv_data[VOICE_NUM_OFFSET] = 0; //clear voice frame num
    for (uint8_t i = 0; i < 8; i++)
    {
        T_VOICE_PACKET *pkt = voice_peek(0);
        if (pkt == NULL)
        {
            break;
        }
        transmit_adv_data[VOICE_NUM_OFFSET]++;
        memcpy(&transmit_adv_data[VOICE_DATA_OFFSET + i * pkt->payload_length], pkt->p_data,
               pkt->payload_length);
        voice_flush(1);
    }
    transmit_adv_data[MANUFACTURE_LEN_OFFSET] = 19 + transmit_adv_data[VOICE_NUM_OFFSET] *
                                                (codec_param.encode_bytes + 1);
}

