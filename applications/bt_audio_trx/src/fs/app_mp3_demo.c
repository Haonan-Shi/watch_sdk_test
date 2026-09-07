/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if MP3_DECODE_TEST
#include "trace.h"
#include "app_mp3_decode.h"
#include "app_fs_if.h"
#include <string.h>

static int my_adapter_read(void *handle, uint8_t *buf, uint32_t len)
{
    T_FILE_HANDLE *hdl = (T_FILE_HANDLE *)handle;
    return app_fs_read(hdl, buf, len);
}

static int my_adapter_seek(void *handle, uint32_t offset)
{
    T_FILE_HANDLE *hdl = (T_FILE_HANDLE *)handle;
    return app_fs_seek(hdl, offset);
}

static uint32_t my_adapter_tell(void *handle)
{
    T_FILE_HANDLE *hdl = (T_FILE_HANDLE *)handle;
    return app_fs_tell(hdl);
}

const char *get_ver_str(int ver)
{
    const char *s[] = {"MPEG 2.5", "Rsrv", "MPEG 2", "MPEG 1"};
    return (ver >= 0 && ver <= 3) ? s[ver] : "Unknown";
}
const char *get_layer_str(int layer)
{
    const char *s[] = {"Rsrv", "Layer III", "Layer II", "Layer I"};
    return (layer >= 0 && layer <= 3) ? s[layer] : "Unknown";
}

void play_mp3_test(void)
{
    T_FILE_HANDLE *mp3_hdl = app_fs_open_file("Free_Test_Data_5MB_MP3.mp3", FS_O_READ);
    if (!mp3_hdl)
    {
        APP_PRINT_ERROR0("Failed to open mp3 file");
        return;
    }

    if (app_mp3_file_init(mp3_hdl) != 0)
    {
        APP_PRINT_ERROR0("MP3 Init failed");
        app_fs_close_file(mp3_hdl);
        return;
    }

    T_MP3_FRAME_INFO frame_info;
    static uint8_t frame_buf[2048];
    int frame_cnt = 0;

    APP_PRINT_INFO0("Start decoding...");


    int len = mp3_parser_get_next_frame(mp3_hdl, frame_buf, sizeof(frame_buf), &frame_info);

    if (len <= 0)
    {
        // len == 0: EOF
        // len < 0: error
        APP_PRINT_INFO1("Decode finish or stop. ret=%d", len);
        return;
    }

    frame_cnt++;

    if (frame_cnt == 1)
    {
        APP_PRINT_INFO7("MP3 Format: %s, %s, Rate=%d Hz, Bitrate=%d bps, ChMode=%d, dura=%d, fsize=%d",
                        TRACE_STRING(get_ver_str(frame_info.version)),
                        TRACE_STRING(get_layer_str(frame_info.layer)),
                        frame_info.sampling_frequency,
                        frame_info.bit_rate,
                        frame_info.channel_mode,
                        frame_info.frame_duration,
                        frame_info.frame_size
                       );
    }

    app_fs_close_file(mp3_hdl);
}


void play_mp3_demo(void)
{
    // 1. Open a file
    T_FILE_HANDLE *mp3_hdl = app_fs_open_file("test.mp3", FS_O_READ);
    if (!mp3_hdl)
    {
        APP_PRINT_ERROR0("Failed to open mp3 file");
        return;
    }

    // 2. init parser and skip ID3v2 Tag
    if (app_mp3_file_init(mp3_hdl) != 0)
    {
        APP_PRINT_ERROR0("MP3 Init failed");
        app_fs_close_file(mp3_hdl);
        return;
    }

    T_MP3_FRAME_INFO frame_info;
    static uint8_t frame_buf[2048]; // size is big enough for a Layer1/2/3 frame
    int frame_cnt = 0;

    APP_PRINT_INFO0("Start decoding...");

    // 3. Read frames
    while (1)
    {
        int len = mp3_parser_get_next_frame(mp3_hdl, frame_buf, sizeof(frame_buf), &frame_info);

        if (len <= 0)
        {
            // len == 0: EOF
            // len < 0: Sync lost too many times
            APP_PRINT_INFO1("Decode finish or stop. ret=%d", len);
            break;
        }

        frame_cnt++;

        // Print info
        if (frame_cnt == 1)
        {
            APP_PRINT_INFO5("MP3 Format: %s, %s, Rate=%d Hz, Bitrate=%d bps, ChMode=%d",
                            get_ver_str(frame_info.version),
                            get_layer_str(frame_info.layer),
                            frame_info.sampling_frequency,
                            frame_info.bit_rate,
                            frame_info.channel_mode);
        }

        // 4. Send to decode
    }

    app_fs_close_file(mp3_hdl);
}
#endif
