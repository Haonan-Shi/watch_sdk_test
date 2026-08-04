/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "app_fs_if.h"
#include "os_sync.h"
#include "clock_manager.h"
#include "wdg.h"
#include "trace.h"
#include "rtk_opus.h"
#include "app_opus_demo.h"
#include "app_module_init.h"

#define MAX_FRAME_SIZE 1920
#define MAX_PACKET_SIZE 1500
#define PCM_TOTAL_SIZE (16000 * 2 * 8) //  Example: 8 seconds, 16 kHz sampling rate, mono; users can adjust based on the actual PCM length.

/**
 * Custom malloc for OPUS library memory allocation.
 * Registered with rtk_opus_register_alloc.
 */
static void *app_opus_malloc(size_t size)
{
    return malloc(size);
}

/**
 * Custom realloc for OPUS library memory reallocation.
 * Registered with rtk_opus_register_realloc.
 */
static void *app_opus_realloc(void *ptr, size_t size)
{
    return realloc(ptr, size);
}

/**
 * Custom free for OPUS library memory release.
 * Registered with rtk_opus_register_free.
 */
static void app_opus_free(void *ptr)
{
    free(ptr);
}


void app_opus_init(void)
{
    // Register memory allocation functions for the codec
    rtk_opus_register_alloc(app_opus_malloc);
    rtk_opus_register_free(app_opus_free);
    rtk_opus_register_realloc(app_opus_realloc);
}

static void opus_module_init(void)
{
    app_opus_init();
}
APP_MODULE_INIT(opus_module_init);


#if 0
/**
 * @brief Write a 32-bit unsigned integer to file in big-endian format.
 *
 * @param file     Pointer to the file handle.
 * @param value    32-bit unsigned integer to write.
 */
static void write_uint32_be(T_FILE_HANDLE *file, unsigned int value)
{
    unsigned char buffer[4];
    buffer[0] = (value >> 24) & 0xFF; // Most significant byte first
    buffer[1] = (value >> 16) & 0xFF;
    buffer[2] = (value >> 8) & 0xFF;
    buffer[3] = value & 0xFF;         // Least significant byte last
    app_fs_write(file, buffer, 4);
}

/**
 * @brief Read a 32-bit unsigned integer from file in big-endian format.
 *
 * @param file     Pointer to the file handle.
 * @return         The 32-bit unsigned integer read from the file.
 *                 Returns 0 if reading fails.
 */
static unsigned int read_uint32_be(T_FILE_HANDLE *file)
{
    unsigned char buffer[4];
    if (app_fs_read(file, buffer, 4) != 4)
    {
        return 0;    // Read failed, return 0
    }

    return ((unsigned int)buffer[0] << 24)
           | ((unsigned int)buffer[1] << 16)
           | ((unsigned int)buffer[2] << 8)
           | (unsigned int)buffer[3];
}

/**
 * Free allocated memory resources if not NULL.
 *
 * Each pointer will be freed only if it is not NULL.
 * Call this function to avoid code duplication when freeing resources.
 *
 * @param pcm_in   Pointer to input PCM buffer
 * @param pcm_out  Pointer to output PCM buffer
 * @param packet   Pointer to packet buffer
 * @param fbytes   Pointer to frame bytes buffer
 */
static void free_resources(void *pcm_in, void *pcm_out, void *packet, void *fbytes)
{
    if (pcm_in) { free(pcm_in); }
    if (pcm_out) { free(pcm_out); }
    if (packet) { free(packet); }
    if (fbytes) { free(fbytes); }
}

/**
 * @brief
 *   Test routine for encoding raw PCM data into OPUS format and then decoding it back.
 *   This function demonstrates the complete workflow:
 *     1. The user should first store a PCM file named voice_new.pcm in the working path of the SD card, such as SD\audio.
 *     2. Reads a raw PCM file, encodes frames using the OPUS codec (via CELT or SILK interface),
 *        and saves the encoded data to a binary file.
 *     3. Then re-opens the encoded file, decodes each frame, and writes the decoded PCM
 *        data into a new file.
 *     4. In actual use, the main focus is on parameter configuration and interface call processes:
 *        encode/decode init --> encode/decode --> encode/decode destroy.
 *   Timing information and encode/decode success status for each frame are logged.
 *
 *   File paths used:
 *      Input PCM  : "voice_new.pcm"
 *      OPUS Output: "opus_encoded2.bin"
 *      Decoded PCM: "voice_new_decoded2.pcm"
 *
 * @param codec_algo   Select which codec to use: OPUS_CODEC_CELT or OPUS_CODEC_SILK
 * @return int         Returns 0 on success, negative on error
 */

int app_pcm_encode_decode_test(T_OPUS_CODEC_ALGO codec_algo)
{

    T_FILE_HANDLE *fin = NULL, *fout = NULL, *fdec = NULL;

    short *pcm_in = NULL;
    short *pcm_out = NULL;
    unsigned char *packet = NULL;
    unsigned char *fbytes = NULL;

    pcm_in = (short *)malloc(sizeof(short) * MAX_FRAME_SIZE);
    pcm_out = (short *)malloc(sizeof(short) * MAX_FRAME_SIZE);
    packet = (unsigned char *)malloc(MAX_PACKET_SIZE);
    fbytes = (unsigned char *)malloc(MAX_FRAME_SIZE * 2);

    int encoded_bytes = 0, decoded_samples = 0;
    int frame_num = 0;
    int i;
    uint32_t encode_timestamp_start;
    uint32_t encode_timestamp_end;

    // Set encoding parameters
    rtk_opus_param_t param;
    rtk_opus_param_set_default(&param);
    param.sampling_rate = 16000;
    param.channels = 1;
    param.bitrate_bps = 32000;
    param.application = RTK_OPUS_APP_VOIP;
    param.bandwidth = RTK_OPUS_BW_AUTO;
    param.frame_size_type = RTK_OPUS_FS_20MS;
    param.frame_size = param.sampling_rate / 50;
    param.complexity = 3;
    param.use_vbr = 1;
    param.use_inbandfec = 0;
    param.use_dtx = 0;
    param.forcechannels = RTK_OPUS_FORCE_AUTO;
    param.max_payload_bytes = MAX_PACKET_SIZE;
    param.final_range_mode = RTK_OPUS_FINAL_RANGE_GET;
    param.final_range = 0;

    // Register memory allocation functions for the codec
    rtk_opus_register_alloc(app_opus_malloc);
    rtk_opus_register_free(app_opus_free);
    rtk_opus_register_realloc(app_opus_realloc);

    // Open input PCM file
    fin = app_fs_open_file("voice_new.pcm", FS_O_READ);
    if (!fin)
    {
        DBG_DIRECT("open input file voice_new.pcm failed!\n");
        goto err;
    }
    // Open output file for encoded data
    fout = app_fs_open_file("opus_encoded.bin", FS_O_CREATE | FS_O_WRITE);
    if (!fout)
    {
        DBG_DIRECT("open output file failed");
        app_fs_close_file(fin);
        goto err;
    }

    // Initialize encoder handle according to selected codec
    void *enc_handle  = NULL;
    if (codec_algo == OPUS_CODEC_CELT)
    {
        enc_handle = rtk_celt_encoder_init(&param);
    }
    else if (codec_algo == OPUS_CODEC_SILK)
    {
        enc_handle = rtk_silk_encoder_init(&param);
    }

    if (!enc_handle)
    {
        DBG_DIRECT("rtk_encoder_init failed!\n");
        app_fs_close_file(fin);
        goto err;
    }

    // Backup watchdog settings and set a longer timeout to avoid triggering WDG during long encodes
    T_WDG_MODE bkup_core_wdg_mode = wdg_get_mode();
    uint32_t bkup_core_wdg_period = wdg_get_timeout_ms();
    WDG_Start(10000, bkup_core_wdg_mode);

    // Encoding loop: read each PCM frame, encode, write encoded packet
    while (app_fs_read(fin, fbytes, param.frame_size * 2) == param.frame_size * 2)
    {
        for (i = 0; i < param.frame_size; ++i)
        {
            pcm_in[i] = (short)(fbytes[2 * i] | (fbytes[2 * i + 1] << 8));
        }

        //Record the current system timestamp
        encode_timestamp_start = log_timestamp_get();

        int res = 0;
        if (codec_algo == OPUS_CODEC_CELT)
        {
            res = rtk_celt_encode(enc_handle, &param, pcm_in, packet, &encoded_bytes);
        }
        else if (codec_algo == OPUS_CODEC_SILK)
        {
            res = rtk_silk_encode(enc_handle, &param, pcm_in, packet, &encoded_bytes);
        }

        if (res == 0 && encoded_bytes > 0)
        {
            //Calculate encoding time per frame
            encode_timestamp_end = log_timestamp_get();
            uint32_t time = encode_timestamp_end - encode_timestamp_start;
            uint32_t time_ms = time / 1000;
            uint32_t time_us = time % 1000;
            DBG_DIRECT("Encode test SystemCpuClock = %dM; t=%d.%d ms, encoded_bytes=%d, frame: %d, codec: %d", \
                       SystemCpuClock / 1000000, time_ms, time_us, encoded_bytes, frame_num, codec_algo);

            // Write encoded_bytes and final_range as 4-byte big-endian integers, then write the packet
            write_uint32_be(fout, encoded_bytes);
            write_uint32_be(fout, param.final_range);
            app_fs_write(fout, packet, encoded_bytes);
            frame_num++;
        }
        else
        {
            DBG_DIRECT("encode failed at frame %d\n", frame_num);
        }
    }

    // Restore watchdog settings
    WDG_Start(bkup_core_wdg_period, bkup_core_wdg_mode);

    app_fs_close_file(fin);
    app_fs_close_file(fout);

    // Destroy the encoder handle to free resources
    if (codec_algo == OPUS_CODEC_CELT)
    {
        rtk_celt_encoder_destroy(enc_handle);
    }
    else if (codec_algo == OPUS_CODEC_SILK)
    {
        rtk_silk_encoder_destroy(enc_handle);
    }

    // Decoding code
    fdec = app_fs_open_file("opus_encoded.bin", FS_O_READ);
    if (!fdec)
    {
        DBG_DIRECT("open encoded file failed!\n");
        goto err;
    }
    // Open output file for new pcm data
    fout = app_fs_open_file("voice_new_decoded.pcm", FS_O_CREATE | FS_O_WRITE);
    if (!fout)
    {
        DBG_DIRECT("open decoded output file failed!\n");
        app_fs_close_file(fdec);
        goto err;
    }
    // Initialize decoder handle for decoding
    void *dec_handle = NULL;
    if (codec_algo == OPUS_CODEC_CELT)
    {
        dec_handle = rtk_celt_decoder_init(&param);
    }
    else if (codec_algo == OPUS_CODEC_SILK)
    {
        dec_handle = rtk_silk_decoder_init(&param);
    }

    if (!dec_handle)
    {
        DBG_DIRECT("rtk_decoder_init failed!\n");
        app_fs_close_file(fdec);
        app_fs_close_file(fout);
        goto err;
    }
    // Decoding loop: read each encoded frame, decode, write PCM output
    for (i = 0; ; ++i)
    {
        unsigned int len = read_uint32_be(fdec);
        unsigned int final_range = read_uint32_be(fdec);
        if (len == 0 || len > MAX_PACKET_SIZE) { break; }
        if (app_fs_read(fdec, packet, len) != len) { break; }

        encode_timestamp_start = log_timestamp_get();

        int res = 0;
        if (codec_algo == OPUS_CODEC_CELT)
        {
            res = rtk_celt_decode(dec_handle, &param, packet, len, pcm_out, &decoded_samples);
        }
        else if (codec_algo == OPUS_CODEC_SILK)
        {
            res = rtk_silk_decode(dec_handle, &param, packet, len, pcm_out, &decoded_samples);
        }

        if (res == 0 && decoded_samples > 0)
        {
            //Calculate decoding time per frame
            encode_timestamp_end = log_timestamp_get();
            uint32_t time = encode_timestamp_end - encode_timestamp_start;
            uint32_t time_ms = time / 1000;
            uint32_t time_us = time % 1000;
            DBG_DIRECT("Decode test SystemCpuClock = %dM; t=%d.%d ms, decoded_samples %d, codec: %d", \
                       SystemCpuClock / 1000000, time_ms, time_us, decoded_samples, codec_algo);

            for (int j = 0; j < decoded_samples; ++j)
            {
                fbytes[2 * j] = pcm_out[j] & 0xFF;
                fbytes[2 * j + 1] = (pcm_out[j] >> 8) & 0xFF;
            }
            app_fs_write(fout, fbytes, decoded_samples * 2);
        }
        else
        {
            DBG_DIRECT("Decode failed at frame %d\n", i);
        }
    }
    app_fs_close_file(fdec);
    app_fs_close_file(fout);

    // Destroy the decoder handle to free resources
    if (codec_algo == OPUS_CODEC_CELT)
    {
        rtk_celt_decoder_destroy(dec_handle);
    }
    else if (codec_algo == OPUS_CODEC_SILK)
    {
        rtk_silk_decoder_destroy(dec_handle);
    }

    DBG_DIRECT("Encode and Decode Test Done. Encoded frames: %d\n", frame_num);

    free_resources(pcm_in, pcm_out, packet, fbytes);
    return 0;
err:
    free_resources(pcm_in, pcm_out, packet, fbytes);
    return -1;
}
#endif