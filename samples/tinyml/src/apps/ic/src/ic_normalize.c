/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*
 * ic_normalize.c - RGB image normalization (Zephyr port, pure C).
 *
 * The vendor SDK tinyml_ic demo linked a prebuilt image-normalize archive
 * (libic_feature.a), but that archive ships only in a Cortex-M55 single-
 * precision build, whose float ABI does not match the double-precision
 * FPU used here. The operation is trivial (divide each 0..255 byte by 255 to
 * get a [0,1] float), so it is reimplemented in C and no feature library is
 * linked - mirroring how the KWS/motion samples keep their preprocess in
 * source.
 *
 * Implements ic_normalize_rgb888_interleaved(), the entry point the demo uses
 * (declared extern in the original tinyml_main.c). Layout is interleaved
 * RGBRGB...; normalization is element-wise so interleaved vs planar is moot.
 */

#include "ic_normalize.h"

int ic_normalize_rgb888_interleaved(const uint8_t *input, uint32_t input_size,
                                    float *output, uint32_t output_size)
{
    if (input == NULL)
    {
        return IC_ERROR_INVALID_INPUT;
    }
    if (output == NULL)
    {
        return IC_ERROR_INVALID_OUTPUT;
    }
    /* input is IC_IMAGE_DATA_SIZE bytes (uint8), output is IC_IMAGE_FEATURES
     * floats. Both cover the same 96*96*3 elements. */
    if (input_size < IC_IMAGE_DATA_SIZE || output_size < IC_IMAGE_FEATURES)
    {
        return IC_ERROR_SIZE_MISMATCH;
    }

    for (uint32_t i = 0; i < IC_IMAGE_FEATURES; ++i)
    {
        output[i] = (float)input[i] * (1.0f / 255.0f);
    }

    return IC_SUCCESS;
}

const char *ic_get_version(void)
{
    return "1.0.0";
}
