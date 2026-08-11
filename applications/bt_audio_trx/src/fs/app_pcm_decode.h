/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_PCM_DECODE_H_
#define _APP_PCM_DECODE_H_

#include <stdint.h>
#include <stdbool.h>

/* Default format used when reading headerless raw PCM (.pcm) files */
#define PCM_DEFAULT_SAMPLE_RATE   16000
#define PCM_DEFAULT_CHANN_NUM     1
#define PCM_DEFAULT_BIT_WIDTH     16
#define PCM_DEFAULT_FRAME_MS      5

/* Upper bound on a single read chunk so it always fits the caller frame buffer */
#define PCM_MAX_FRAME_SIZE        1152 * 6 /* max PCM frame size for 192kHz/2ch/24-bit at 7ms */

typedef struct
{
    uint32_t          sample_rate;     // Hz
    uint8_t           chann_num;       // 1 = mono, 2 = stereo
    uint8_t           bit_width;       // bits per sample: 8/16/24/32
    uint16_t          frame_duration;  // ms covered by one read chunk
    uint16_t          sample_counts;   // samples per channel in one chunk
    uint16_t          frame_size;    // bytes  (all channel)
    uint32_t          data_size;       // total PCM data bytes (0 = unknown / raw)
} T_PCM_FRAME_INFO;

// --- Callback definition (same fs abstraction as the MP3 parser) ---
typedef int (*pcm_read_cb_t)(void *handle, uint8_t *buf, uint32_t len);
typedef int (*pcm_seek_cb_t)(void *handle, uint32_t offset);
typedef uint32_t (*pcm_tell_cb_t)(void *handle);

/**
 * @brief Register file read/seek/tell callbacks.
 */
void app_pcm_decode_init(pcm_read_cb_t read_func, pcm_seek_cb_t seek_func, pcm_tell_cb_t tell_func);

/**
 * @brief Probe the file and prepare for reading.
 *
 * If a RIFF/WAVE header is present, the fmt chunk is parsed for the real
 * format and the file is positioned at the start of the data chunk.
 *
 * Otherwise the file is treated as headerless raw PCM. In that case the caller
 * may pre-fill info->sample_rate / chann_num / bit_width (e.g. with a format
 * supplied by the host); any of those fields left 0 falls back to the matching
 * PCM_DEFAULT_* value. info->data_size is always reset here.
 *
 * @return 0 on success, <0 on error.
 */
int app_pcm_file_init(void *fs_handle, T_PCM_FRAME_INFO *info);

/**
 * @brief Read the next PCM chunk.
 * @return >0: bytes read, 0: EOF, <0: error
 */
int pcm_parser_get_next_frame(void *fs_handle, uint8_t *buf, uint32_t buf_len,
                              T_PCM_FRAME_INFO *info);

#endif // _APP_PCM_DECODE_H_
