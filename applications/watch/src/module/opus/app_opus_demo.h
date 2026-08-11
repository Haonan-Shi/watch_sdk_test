/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

typedef enum
{
    OPUS_CODEC_CELT        = 0x0,
    OPUS_CODEC_SILK        = 0x1,
} T_OPUS_CODEC_ALGO;


void app_opus_init(void);

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

int app_pcm_encode_decode_test(T_OPUS_CODEC_ALGO codec_algo);
