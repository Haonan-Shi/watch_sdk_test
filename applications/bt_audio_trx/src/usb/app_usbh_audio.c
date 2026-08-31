/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "usbh_mgr.h"
#include "usbh_audio_driver.h"
#include "usb_audio1_spec.h"
#include "trace.h"
/* -------------------------------------------------------------------------
 * Stream parameters
 * -------------------------------------------------------------------------*/
// #define USBH_AUDIO_PREFERRED_RATE     48000
// #define USBH_AUDIO_PREFERRED_CH       2
// #define USBH_AUDIO_PREFERRED_BITS     16
// #define USBH_AUDIO_BUF_PROC_INTRVL    8

#define USBH_AUDIO_PREFERRED_RATE     192000
#define USBH_AUDIO_PREFERRED_CH       2
#define USBH_AUDIO_PREFERRED_BITS     24
#define USBH_AUDIO_BUF_PROC_INTRVL    16

#define TEST_PATTERN_SINE       0
#define TEST_PATTERN_RAMP       1
#ifndef TEST_PATTERN
#define TEST_PATTERN            TEST_PATTERN_SINE
#endif
static uint8_t g_test_pattern = TEST_PATTERN;

static uint32_t g_preferred_rate     = USBH_AUDIO_PREFERRED_RATE;
static uint8_t  g_preferred_ch       = USBH_AUDIO_PREFERRED_CH;
static uint8_t  g_preferred_bits     = USBH_AUDIO_PREFERRED_BITS;
static uint8_t  g_buf_proc_intrvl    = USBH_AUDIO_BUF_PROC_INTRVL;

#define SINE_LUT_LEN  48
static const int16_t g_sine_lut[SINE_LUT_LEN] =
{
    0,  4276,  8480, 12539, 16383, 19947, 23169, 25994,
    28377, 30272, 31650, 32485, 32767, 32485, 31650, 30272,
    28377, 25994, 23169, 19947, 16383, 12539,  8480,  4276,
    0, -4276, -8480, -12539, -16383, -19947, -23169, -25994,
    -28377, -30272, -31650, -32485, -32767, -32485, -31650, -30272,
    -28377, -25994, -23169, -19947, -16383, -12539, -8480, -4276,
};

#if F_APP_DBG_DUMP_PCM_TO_RINGBUF
#include "ring_buffer.h"
extern struct
{
    T_RING_BUFFER           ring_buf;
    uint8_t                 *buf;
} pcm_drain_rb;
#endif
extern bool app_src_play_sd_pipe_is_playing(void);
extern bool app_read_start;
/* -------------------------------------------------------------------------
 * Runtime state
 * -------------------------------------------------------------------------*/
static T_USBH_AUDIO_FORMAT g_active_fmt;
static bool     g_stream_started = false;
static uint32_t g_sine_phase     = 0;
static uint8_t  g_pkt_fill_val   = 0;

static void audio_update_active_fmt(void)
{
    g_active_fmt.sample_freq    = g_preferred_rate;
    g_active_fmt.nr_channels    = (uint8_t)g_preferred_ch;
    g_active_fmt.bit_resolution = (uint8_t)g_preferred_bits;
    g_active_fmt.subframe_size  = (uint8_t)((g_preferred_bits + 7) / 8);
}

void app_usbh_audio_set_param(uint32_t rate, uint32_t ch, uint32_t bits, uint32_t buf_intrvl)
{
    g_preferred_rate     = rate;
    g_preferred_ch       = (uint8_t)ch;
    g_preferred_bits     = (uint8_t)bits;
    g_buf_proc_intrvl    = (uint8_t)buf_intrvl;
    audio_update_active_fmt();

    APP_PRINT_INFO4("app_usbh_audio_set_param: rate=%lu ch=%lu bits=%lu buf_intrvl=%lu",
                    rate, ch, bits, buf_intrvl);
}

/* -------------------------------------------------------------------------
 * Internal helpers
 * -------------------------------------------------------------------------*/
static void fill_sine(uint8_t *buf, uint32_t nr_samples, uint8_t nr_ch,
                      uint8_t subframe_size)
{
    uint32_t i;
    uint8_t  ch;
    for (i = 0; i < nr_samples; i++)
    {
        int32_t s = (int32_t)g_sine_lut[g_sine_phase % SINE_LUT_LEN];
        g_sine_phase++;
        for (ch = 0; ch < nr_ch; ch++)
        {
            if (subframe_size == 3)
            {
                /* 24-bit little-endian: scale 16-bit LUT value up by 256 */
                int32_t s24 = s << 8;
                *buf++ = (uint8_t)(s24 & 0xFF);
                *buf++ = (uint8_t)((s24 >> 8) & 0xFF);
                *buf++ = (uint8_t)((s24 >> 16) & 0xFF);
            }
            else
            {
                /* 16-bit little-endian (default) */
                *buf++ = (uint8_t)(s & 0xFF);
                *buf++ = (uint8_t)((s >> 8) & 0xFF);
            }
        }
    }
}
/* -------------------------------------------------------------------------
 * ISO stream callback
 * -------------------------------------------------------------------------*/
static int audio_out_stream_cb(uint8_t dir, uint8_t *buf,
                               T_USBH_AUDIO_PKT_INFO *pkt_info,
                               uint8_t pkt_cnt, void *priv)
{
    uint8_t i;
    (void)dir;
    (void)priv;
    // if (g_test_pattern == TEST_PATTERN_SINE)
    // {
    uint32_t total_bytes = 0;
    uint32_t bytes_per_sample;
    uint32_t nr_samples;
#if F_APP_DBG_DUMP_PCM_TO_RINGBUF
    if (!app_src_play_sd_pipe_is_playing())
    {
        return;
    }
#endif
    bytes_per_sample = (uint32_t)g_active_fmt.nr_channels * g_active_fmt.subframe_size;;
    APP_PRINT_WARN1("uac_audio_out_complete_cb: bytes_per_sample %d", bytes_per_sample);

    for (i = 0; i < pkt_cnt; i++)
    {
        uint8_t *p = buf + pkt_info[i].offset;
        uint32_t nr_samples =
            (bytes_per_sample > 0) ? (pkt_info[i].length / bytes_per_sample) : 0;
#if F_APP_DBG_DUMP_PCM_TO_RINGBUF
        if (app_read_start)
        {
            uint32_t data_cnt = ring_buffer_get_data_count(&pcm_drain_rb.ring_buf);
            USB_PRINT_INFO2("audio_out_stream_cb1:count %d,pkt_info[i].length %d", data_cnt,
                            pkt_info[i].length);
            if (data_cnt >= pkt_info[i].length)
            {
                ring_buffer_read(&pcm_drain_rb.ring_buf, pkt_info[i].length, p);
            }
            else
            {
                memset(p, 0, pkt_info[i].length);
            }
        }
        else
        {
            APP_PRINT_WARN0("uac_audio_out_complete_cb: app_read_start is false, memset buf to 0");
            memset(p, 0, pkt_info[i].length);
        }
#endif
    }

    return 0;
}
/* -------------------------------------------------------------------------
 * Stream start sequence
 * -------------------------------------------------------------------------*/
static void audio_out_start_streaming(void)
{
    if (g_test_pattern == TEST_PATTERN_SINE)
    {
        g_sine_phase = 0;
    }
    else
    {
        g_pkt_fill_val = 0;
    }
    if (usbh_audio_driver_stream_start(USBH_AUDIO_DIR_OUT, &g_active_fmt, g_buf_proc_intrvl,
                                       audio_out_stream_cb, NULL) == 0)
    {
        g_stream_started = true;
    }
}
/* -------------------------------------------------------------------------
 * USB host manager event callback
 * -------------------------------------------------------------------------*/
static int app_usbh_audio_cb(T_USBH_MGR_EVT evt, T_USBH_MGR_EVT_PARAM *param)
{
    if (evt == USBH_MGR_EVT_DEV_INFO_INFORM)
    {
        if (param->dev_info.class  != UAC1_CLASS_CODE_AUDIO)
        {
            return 0;
        }
        if (param->dev_info.audio->dir_cap & (1 << USBH_AUDIO_DIR_OUT) &&
            param->dev_info.audio->nr_formats_out == 0)
        {
            return 0;
        }
        if (g_stream_started)
        {
            return 0;
        }
        audio_out_start_streaming();
        USB_PRINT_INFO3("app_usbh_audio_cb(g_active_fmt): sample_freq %d nr_channels %d bit_resolution %d\n",
                        g_active_fmt.sample_freq, g_active_fmt.nr_channels, g_active_fmt.bit_resolution);
    }
    else if (evt == USBH_MGR_EVT_PORT_STATE_CHANGED &&
             param->port_state.state == USBH_MGR_PORT_STATE_DEV_DETTACHED)
    {
        if (g_stream_started)
        {
            usbh_audio_driver_stream_stop(USBH_AUDIO_DIR_OUT);
            g_stream_started = false;
        }
    }
    return 0;
}
/* -------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------*/
void app_usbh_audio_init(void)
{
    T_USBH_MGR_EVT_MSK msk;
    msk.d32              = 0;
    msk.b.dev_info_inform  = 1;
    msk.b.port_sts_changed = 1;
    usbh_mgr_init();
    usbh_audio_driver_init();
    usbh_mgr_cb_register(msk, app_usbh_audio_cb);
    audio_update_active_fmt();
}
