/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "trace.h"
#include "app_cmd.h"
#include "app_report.h"
#include "app_fs_test.h"
#include "app_fs_if.h"
#include "app_mp3_decode.h"
#include <string.h>

static mp3_read_cb_t g_read = NULL;
static mp3_seek_cb_t g_seek = NULL;
static mp3_tell_cb_t g_tell = NULL;

// Bitrate Table (kbps)
// [Version_Index][Layer_Index][Bitrate_Index]
// Version Index: 0=MPEG2/2.5(LSF), 1=MPEG1
// Layer Index:   0=Layer1, 1=Layer2, 2=Layer3
static const uint16_t mp3_bitrates[2][3][16] =
{
    {
        // MPEG 2 & 2.5 (LSF)
        {0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256, 0}, // Layer 1
        {0, 8,  16, 24, 32, 40, 48, 56,  64,  80,  96,  112, 128, 144, 160, 0}, // Layer 2
        {0, 8,  16, 24, 32, 40, 48, 56,  64,  80,  96,  112, 128, 144, 160, 0}  // Layer 3
    },
    {
        // MPEG 1
        {0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 0}, // Layer 1
        {0, 32, 48, 56, 64,  80,  96,  112, 128, 160, 192, 224, 256, 320, 384, 0}, // Layer 2
        {0, 32, 40, 48, 56,  64,  80,  96,  112, 128, 160, 192, 224, 256, 320, 0}  // Layer 3
    }
};

// Sample Rate Table (Hz)
// [Version][SampleRate_Index]
// Version: 0=2.5, 1=Rsrv, 2=2, 3=1
static const uint16_t mp3_samplerates[4][3] =
{
    {11025, 12000, 8000},   // MPEG 2.5
    {0,     0,     0   },   // Reserved
    {22050, 24000, 16000},  // MPEG 2
    {44100, 48000, 32000}   // MPEG 1
};

// Samples per Frame
// [Version_Index][Layer_Index]
// Version Index: 0=MPEG1, 1=MPEG2/2.5
// Layer Index:   0=Layer1, 1=Layer2, 2=Layer3
static const uint16_t mp3_samples_per_frame[2][3] =
{
    {384, 1152, 1152}, // MPEG 1
    {384, 1152, 576 }  // MPEG 2 & 2.5
};

static int parse_header_raw(uint8_t *h, T_MP3_FRAME_INFO *info)
{
    // 1. Sync Word check (11 bits set)
    // h[0] Shall be FF
    // h[1] Highest 3 bits shall be 7 (111xxxxx) -> & 0xE0 == 0xE0
    if ((h[0] != 0xFF) || ((h[1] & 0xE0) != 0xE0)) { return -1; }

    // 2. Get Version and Layer
    uint8_t ver_id = (h[1] >> 3) & 0x03; // 00=2.5, 01=res, 10=2, 11=1
    uint8_t lay_id = (h[1] >> 1) & 0x03; // 00=res, 01=L3, 10=L2, 11=L1

    if (ver_id == 1 || lay_id == 0) { return -1; } // Reserved illegal value

    // 3. Get Bitrate Index, Samplerate Index, Padding
    uint8_t br_idx = (h[2] >> 4) & 0x0F;
    uint8_t sr_idx = (h[2] >> 2) & 0x03;
    uint8_t padding = (h[2] >> 1) & 0x01;

    if (br_idx == 0 || br_idx == 15 || sr_idx == 3) { return -1; } // Reserved illegal value

    // --- Check Table ---

    // ver_idx_for_br: 1 for MPEG1, 0 for MPEG2/2.5
    int ver_idx_for_br = (ver_id == MP3_VER_1) ? 1 : 0;

    // ver_idx_for_spf: 0 for MPEG1, 1 for MPEG2/2.5
    int ver_idx_for_spf = (ver_id == MP3_VER_1) ? 0 : 1;

    // lay_idx: 3->0(L1), 2->1(L2), 1->2(L3)
    int lay_idx = 3 - lay_id;

    // Fill Info
    info->version = ver_id;
    info->layer = lay_id;
    info->channel_mode = (h[3] >> 6) & 0x03;

    info->bit_rate = mp3_bitrates[ver_idx_for_br][lay_idx][br_idx] * 1000;
    info->sampling_frequency = mp3_samplerates[ver_id][sr_idx];

    if (info->bit_rate == 0 || info->sampling_frequency == 0) { return -1; }

    info->sample_counts = mp3_samples_per_frame[ver_idx_for_spf][lay_idx];

    // Calculate frame_duration (ms)
    info->frame_duration = (info->sample_counts * 1000) / info->sampling_frequency;

    // Calculate Frame Size (bytes)
    // Integer operations avoid floating-point:
    // Layer 1: (12 * BitRate / SampleRate + Padding) * 4
    // Layer 2/3: 144 * BitRate / SampleRate + Padding
    // Note: The coefficient for MPEG2/2.5 Layer3 is 72, but since SamplePerFrame is 576 (half of 1152)
    // The formula can actually be unified as: (SamplesPerFrame / 8) * BitRate / SampleRate + Padding

    if (lay_id == MP3_LAYER_1)
    {
        info->frame_size = ((12 * info->bit_rate) / info->sampling_frequency + padding) * 4;
    }
    else
    {
        // Layer 2 & 3
        // For MPEG1 Layer3: 1152/8 = 144
        // For MPEG2 Layer3: 576/8 = 72
        // Use sample_counts / 8
        info->frame_size = ((info->sample_counts / 8) * info->bit_rate) / info->sampling_frequency +
                           padding;
    }

    return 0;
}


// ---------------------------------------------------------------------------
// Xing/Info Header detection & parsing
//
// VBR (Variable Bitrate) MP3 files embed a Xing/Info header in the first
// frame's side info area.  The bitrate in the frame header itself is only a
// placeholder the real average bitrate comes from the Xing payload.
// ---------------------------------------------------------------------------

// The Xing/Info marker sits just after the side information.
// Offset from frame start = 4 (header) + side_info_size.
static int xing_header_offset(T_MP3_FRAME_INFO *info)
{
    if (info->version == MP3_VER_1)
    {
        return (info->channel_mode == MP3_MODE_MONO) ? 21 : 36;
    }
    // MPEG 2 / 2.5
    return 21;
}

// Parse Xing/Info header embedded in the first frame of a VBR file.
// Updates info->bit_rate with the true average bitrate and fills Xing fields.
// Returns 0 on success, -1 if no Xing/Info header is present.
static int parse_xing_header(uint8_t *frame_start, T_MP3_FRAME_INFO *info)
{
    int offset = xing_header_offset(info);
    uint8_t *xing = frame_start + offset;

    // Check for "Xing" or "Info" marker
    if (memcmp(xing, "Xing", 4) != 0 && memcmp(xing, "Info", 4) != 0)
    {
        return -1;
    }
    info->is_xing_frame = 1;

    // Flags (big-endian)
    //   0x0001  num_frames  present
    //   0x0002  num_bytes   present
    //   0x0004  TOC         present
    //   0x0008  VBR scale   present
    uint32_t flags = ((uint32_t)xing[4] << 24) |
                     ((uint32_t)xing[5] << 16) |
                     ((uint32_t)xing[6] << 8)  |
                     (uint32_t)xing[7];

    int pos = 8;
    if (flags & 0x0001UL)
    {
        info->xing_num_frames = ((uint32_t)xing[pos]     << 24) |
                                ((uint32_t)xing[pos + 1] << 16) |
                                ((uint32_t)xing[pos + 2] << 8)  |
                                (uint32_t)xing[pos + 3];
        pos += 4;
    }
    if (flags & 0x0002UL)
    {
        info->xing_num_bytes = ((uint32_t)xing[pos]      << 24) |
                               ((uint32_t)xing[pos + 1]  << 16) |
                               ((uint32_t)xing[pos + 2]  << 8)  |
                               (uint32_t)xing[pos + 3];
        pos += 4;
    }

    // Calculate average bitrate from Xing cumulative data
    if (info->xing_num_frames > 0 && info->xing_num_bytes > 0)
    {
        uint64_t total_samples = (uint64_t)info->xing_num_frames * info->sample_counts;
        uint64_t duration_ms  = (total_samples * 1000) / info->sampling_frequency;
        if (duration_ms > 0)
        {
            info->bit_rate = (uint32_t)(((uint64_t)info->xing_num_bytes * 8000) / duration_ms);
        }
    }

    return 0;
}

int app_mp3_file_init(void *fs_handle)
{
    if (!fs_handle || !g_read || !g_seek || !g_tell) { return -1; }

    uint8_t header[10];
    if (g_read(fs_handle, header, 10) != 10)
    {
        g_seek(fs_handle, 0);
        return 0;
    }

    // Check and skip ID3v2
    if (header[0] == 'I' && header[1] == 'D' && header[2] == '3')
    {
        // ID3v2 format:
        // | 0 - 2 | 3 Bytes | Identifier: "ID3"
        // | 3 - 4 | 2 Bytes | Version: sample: 0x03 0x00 (ID3v2.3)
        // | 5     | 1 Byte  | Flags
        // | 6 - 9 | 4 Bytes | Tag Size

        // Synchsafe integer conversion
        uint32_t tag_size = ((header[6] & 0x7F) << 21) |
                            ((header[7] & 0x7F) << 14) |
                            ((header[8] & 0x7F) << 7)  |
                            (header[9] & 0x7F);

        tag_size += 10; // Header size itself
        g_seek(fs_handle, tag_size);
    }
    else
    {
        g_seek(fs_handle, 0);
    }

    return 0;
}

int mp3_parser_get_next_frame(void *fs_handle, uint8_t *buf, uint32_t buf_len,
                              T_MP3_FRAME_INFO *info)
{
    if (!fs_handle || !g_read || !g_seek || !g_tell) { return -1; }

    uint8_t tmp_buf[4];
    int res;
    int sync_retry_limit = 4096; // used for skipping error data

    while (sync_retry_limit > 0)
    {
        uint32_t start_pos = g_tell(fs_handle);

        if (g_read(fs_handle, tmp_buf, 4) != 4) { return 0; } // EOF

        info->is_xing_frame = 0;

        if (parse_header_raw(tmp_buf, info) == 0)
        {
            // --> Found Valid Header

            if (info->frame_size > buf_len)
            {
                // Inssuficient Buffer
                return -2;
            }

            // Copy header to output buffer
            memcpy(buf, tmp_buf, 4);

            // Read Payload
            int remaining = info->frame_size - 4;
            if (remaining > 0)
            {
                res = g_read(fs_handle, buf + 4, remaining);
                if (res != remaining)
                {
                    return 0; // Unexpected EOF
                }
            }

            // Check for Xing/Info header in the first frame (VBR files).
            // Extracts average bitrate if present.
            parse_xing_header(buf, info);

            return info->frame_size;
        }

        // --> Sync Lost, step forward 1 byte
        g_seek(fs_handle, start_pos + 1);
        sync_retry_limit--;
    }

    return -3; // Sync failed
}

/**
 * @brief Peek first frame header without consuming it.
 *
 * Reads the 4-byte frame header at the current file position, parses it
 * into @p info, then seeks back so the frame can be read again by
 * mp3_parser_get_next_frame().
 *
 * @return 0 on success, -1 on error
 */
int mp3_parser_peek_frame_info(void *fs_handle, T_MP3_FRAME_INFO *info)
{
    if (!fs_handle || !g_read || !g_seek || !g_tell || !info) { return -1; }

    uint32_t saved_pos = g_tell(fs_handle);
    uint8_t  hdr[4];

    if (g_read(fs_handle, hdr, 4) != 4)
    {
        g_seek(fs_handle, saved_pos);
        return -1;
    }

    if (parse_header_raw(hdr, info) != 0)
    {
        g_seek(fs_handle, saved_pos);
        return -1;
    }

    /* No Xing check needed here ¡ª Xing lives in the sideinfo/payload,
     * not in the 4-byte header.  seek back so the caller can re-read
     * the full frame later. */
    g_seek(fs_handle, saved_pos);
    return 0;
}

void app_mp3_decode_init(mp3_read_cb_t read_func, mp3_seek_cb_t seek_func, mp3_tell_cb_t tell_func)
{
    g_read = read_func;
    g_seek = seek_func;
    g_tell = tell_func;
}