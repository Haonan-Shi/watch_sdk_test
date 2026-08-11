/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "trace.h"
#include "app_pcm_decode.h"
#include <string.h>

#define PCM_UNBOUNDED   0xFFFFFFFF

static pcm_read_cb_t g_read = NULL;
static pcm_seek_cb_t g_seek = NULL;
static pcm_tell_cb_t g_tell = NULL;

/* Remaining payload bytes. For raw PCM it stays PCM_UNBOUNDED (read until EOF);
 * for WAV it is the size of the data chunk. */
static uint32_t g_data_remaining = PCM_UNBOUNDED;

static uint16_t rd_le16(const uint8_t *p)
{
    return (uint16_t)(p[0] | (p[1] << 8));
}

static uint32_t rd_le32(const uint8_t *p)
{
    return (uint32_t)(p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t)p[3] << 24));
}

static void pcm_calc_frame(T_PCM_FRAME_INFO *info)
{
    if (info->chann_num == 0) { info->chann_num = PCM_DEFAULT_CHANN_NUM; }
    if (info->bit_width == 0) { info->bit_width = PCM_DEFAULT_BIT_WIDTH; }
    if (info->sample_rate == 0) { info->sample_rate = PCM_DEFAULT_SAMPLE_RATE; }
    if (info->frame_duration == 0) { info->frame_duration = PCM_DEFAULT_FRAME_MS; }

    /* bytes for one sample across all channels */
    uint32_t bytes_per_unit = (uint32_t)info->chann_num * (info->bit_width / 8);
    if (bytes_per_unit == 0) { bytes_per_unit = 2; }

    uint32_t samples = (uint32_t)info->sample_rate * info->frame_duration / 1000; // per channel
    uint32_t max_samples = PCM_MAX_FRAME_SIZE / bytes_per_unit;
    if (max_samples == 0) { max_samples = 1; }
    if (samples > max_samples) { samples = max_samples; }
    if (samples == 0) { samples = 1; }

    info->sample_counts  = (uint16_t)samples;
    info->frame_size   = (uint16_t)(samples * bytes_per_unit);
    info->frame_duration = (uint16_t)(samples * 1000 / info->sample_rate);
}

int app_pcm_file_init(void *fs_handle, T_PCM_FRAME_INFO *info)
{
    if (!fs_handle || !info || !g_read || !g_seek || !g_tell) { return -1; }

    /* Caller may pre-fill sample_rate/chann_num/bit_width for headerless raw PCM
     * (e.g. supplied by the host). Any field left 0 is filled with a PCM_DEFAULT_*
     * by pcm_calc_frame(). A real WAV header overrides these below. */
    info->data_size = 0;

    uint8_t hdr[12];
    if (g_read(fs_handle, hdr, 12) != 12)
    {
        /* file too small for a WAV header -> treat as raw PCM */
        g_seek(fs_handle, 0);
        g_data_remaining = PCM_UNBOUNDED;
        pcm_calc_frame(info);
        return 0;
    }

    bool is_wav = (hdr[0] == 'R' && hdr[1] == 'I' && hdr[2] == 'F' && hdr[3] == 'F' &&
                   hdr[8] == 'W' && hdr[9] == 'A' && hdr[10] == 'V' && hdr[11] == 'E');

    if (!is_wav)
    {
        g_seek(fs_handle, 0);
        g_data_remaining = PCM_UNBOUNDED;
        pcm_calc_frame(info);
        return 0;
    }

    /* Walk the RIFF sub-chunks looking for "fmt " and "data". */
    bool fmt_found = false;
    bool data_found = false;

    for (int guard = 0; guard < 16 && !data_found; guard++)
    {
        uint8_t ch[8];
        if (g_read(fs_handle, ch, 8) != 8) { break; }

        uint32_t csize = rd_le32(&ch[4]);
        uint32_t pos = g_tell(fs_handle);

        if (ch[0] == 'f' && ch[1] == 'm' && ch[2] == 't' && ch[3] == ' ')
        {
            uint8_t fb[16];
            if (g_read(fs_handle, fb, 16) != 16) { return -2; }

            info->chann_num   = (uint8_t)rd_le16(&fb[2]);
            info->sample_rate = rd_le32(&fb[4]);
            info->bit_width   = (uint8_t)rd_le16(&fb[14]);
            fmt_found = true;


            /* skip any extension bytes (+ pad to even boundary) */
            g_seek(fs_handle, pos + csize + (csize & 1));
        }
        else if (ch[0] == 'd' && ch[1] == 'a' && ch[2] == 't' && ch[3] == 'a')
        {
            info->data_size  = csize;
            g_data_remaining = csize;
            data_found = true; /* file is already positioned at the data start */
        }
        else
        {
            g_seek(fs_handle, pos + csize + (csize & 1));
        }
    }

    if (!fmt_found || !data_found) { return -3; }

    pcm_calc_frame(info);

    APP_PRINT_INFO5("app_pcm_file_init: WAV rate=%d ch=%d bits=%d data=%d frame_size=%d",
                    info->sample_rate, info->chann_num, info->bit_width,
                    info->data_size, info->frame_size);
    return 0;
}

int pcm_parser_get_next_frame(void *fs_handle, uint8_t *buf, uint32_t buf_len,
                              T_PCM_FRAME_INFO *info)
{
    if (!fs_handle || !buf || !info || !g_read) { return -1; }

    uint32_t want = info->frame_size;
    if (want == 0) { return -2; }
    if (want > buf_len) { want = buf_len; }
    if (want > g_data_remaining) { want = g_data_remaining; }
    if (want == 0) { return 0; } // reached end of data chunk

    int res = g_read(fs_handle, buf, want);
    if (res <= 0) { return 0; } // EOF

    if (g_data_remaining != PCM_UNBOUNDED)
    {
        g_data_remaining -= (uint32_t)res;
    }
    return res;
}

void app_pcm_decode_init(pcm_read_cb_t read_func, pcm_seek_cb_t seek_func, pcm_tell_cb_t tell_func)
{
    g_read = read_func;
    g_seek = seek_func;
    g_tell = tell_func;
    g_data_remaining = PCM_UNBOUNDED;
}
