/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_MP3_DECODE_H_
#define _APP_MP3_DECODE_H_

#include <stdint.h>
#include <stdbool.h>

// MP3 Version ID
typedef enum
{
    MP3_VER_2_5       = 0,    // MPEG 2.5
    MP3_VER_RESERVED  = 1,
    MP3_VER_2         = 2,      // MPEG 2
    MP3_VER_1         = 3       // MPEG 1
} T_MP3_VERSION;

// MP3 Layer
typedef enum
{
    MP3_LAYER_RESERVED = 0,
    MP3_LAYER_3 = 1,    // Layer III
    MP3_LAYER_2 = 2,    // Layer II
    MP3_LAYER_1 = 3     // Layer I
} T_MP3_LAYER;

// Channel Mode
typedef enum
{
    MP3_MODE_STEREO = 0,
    MP3_MODE_JOINT_STEREO = 1,
    MP3_MODE_DUAL_CHANNEL = 2,
    MP3_MODE_MONO = 3
} T_MP3_CHANNEL_MODE;

typedef struct
{
    uint32_t          sampling_frequency; // Hz
    uint8_t           channel_mode;       // T_MP3_CHANNEL_MODE
    uint8_t           version;            // T_MP3_VERSION
    uint8_t           layer;              // T_MP3_LAYER
    uint32_t          bit_rate;           // bps
    uint16_t          sample_counts;      // sample counts of each frame
    uint16_t          frame_duration;     // frame duration of frame
    uint16_t          frame_size;         // Header + SideInfo + Data
    uint8_t           is_xing_frame;      // Xing/Info header present in this frame
    uint32_t          xing_num_frames;    // Total frames (from Xing, VBR files)
    uint32_t          xing_num_bytes;     // Total bytes (from Xing, VBR files)
} T_MP3_FRAME_INFO;

// --- Callback definition ---
typedef int (*mp3_read_cb_t)(void *handle, uint8_t *buf, uint32_t len);
typedef int (*mp3_seek_cb_t)(void *handle, uint32_t offset);
typedef uint32_t (*mp3_tell_cb_t)(void *handle);


/**
 * @brief Init parser and skip ID3v2 Tag
 */
int app_mp3_file_init(void *fs_handle);
void app_mp3_decode_init(mp3_read_cb_t read_func, mp3_seek_cb_t seek_func, mp3_tell_cb_t tell_func);

/**
 * @brief Get next frame
 * @return >0: Frame Length, 0: EOF, <0: Error
 */
int mp3_parser_get_next_frame(void *fs_handle, uint8_t *buf, uint32_t buf_len,
                              T_MP3_FRAME_INFO *info);

/**
 * @brief Peek first-frame header without consuming the frame.
 *
 * Reads the 4-byte frame header at the current file position, parses it
 * into @p info, then seeks back.  The frame can be read again by
 * mp3_parser_get_next_frame() afterward.
 *
 * @return 0 on success, -1 on error
 */
int mp3_parser_peek_frame_info(void *fs_handle, T_MP3_FRAME_INFO *info);

#endif // _APP_MP3_DECODE_H_
