/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * @file ic_normalize.h
 * @brief RGB Image Normalization Library for CM55
 *
 * Processes 96x96 RGB images and normalizes pixel values to [0, 1] range
 * for direct input to ML models.
 *
 * Input: 96x96 RGB image (uint8_t, 0-255 per channel)
 * Output: Normalized float array (1.0 per channel, [0.0, 1.0] range)
 *
 * Supported Platforms: ARM CM55 (Cortex-M55) with MVE/DSP acceleration
 */

#ifndef C90BE7F4_7AF5_486A_9576_4FB3634E4A05
#define C90BE7F4_7AF5_486A_9576_4FB3634E4A05

#ifndef IC_NORMALIZE_H
#define IC_NORMALIZE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

/* =============================================================================
 * Configuration Macros
 * ============================================================================= */

/** Image dimensions */
#define IC_IMAGE_WIDTH      96
#define IC_IMAGE_HEIGHT     96
#define IC_IMAGE_CHANNELS   3  /* RGB */

/** Total pixel count per image */
#define IC_IMAGE_PIXELS     (IC_IMAGE_WIDTH * IC_IMAGE_HEIGHT)

/** Total feature count (pixels * channels) */
#define IC_IMAGE_FEATURES   (IC_IMAGE_PIXELS * IC_IMAGE_CHANNELS)

/** Image data size in bytes (uint8_t input) */
#define IC_IMAGE_DATA_SIZE  (IC_IMAGE_FEATURES * sizeof(uint8_t))

/** Feature output size in bytes (float output) */
#define IC_FEATURE_SIZE     (IC_IMAGE_FEATURES * sizeof(float))

/* =============================================================================
 * Data Types
 * ============================================================================= */

/**
 * @struct ic_image_t
 * @brief Input image structure (96x96 RGB)
 *
 * Memory layout: R[0,0] G[0,0] B[0,0] R[0,1] G[0,1] B[0,1] ... (row-major, interleaved RGB)
 * Or: RRR...GGG...BBB... (planar format, depending on source)
 *
 * @note Actual memory must be pre-allocated by caller
 */
typedef struct
{
    uint8_t *data;          /**< Pointer to image data buffer (96x96x3 = 27648 bytes) */
    size_t size;            /**< Size of data in bytes (should be IC_IMAGE_DATA_SIZE) */
} ic_image_t;

/**
 * @struct ic_features_t
 * @brief Output feature vector (normalized)
 *
 * Memory layout: Same as input (interleaved or planar, depending on config)
 * Values: [0.0, 1.0] normalized float
 */
typedef struct
{
    float *data;            /**< Pointer to feature buffer (27648 floats) */
    size_t size;            /**< Size of data in floats (should be IC_IMAGE_FEATURES) */
} ic_features_t;

/**
 * @enum ic_pixel_layout_t
 * @brief Pixel layout configuration
 */
typedef enum
{
    IC_PIXEL_LAYOUT_INTERLEAVED,  /**< RGBRGBRGB... (interleaved) */
    IC_PIXEL_LAYOUT_PLANAR        /**< RRR...GGG...BBB... (planar, 3 separate planes) */
} ic_pixel_layout_t;

/* =============================================================================
 * Main API Functions
 * ============================================================================= */

/**
 * @brief Normalize an interleaved RGB888 image to [0, 1] range (demo entry point)
 *
 * Divides each 0..255 byte by 255 to produce a [0.0, 1.0] float. This is the
 * entry point used by the Zephyr tinyml_ic demo; it is implemented in
 * ic_normalize.c (no prebuilt feature library is linked).
 *
 * @param[in]  input        Raw image data (uint8_t array, IC_IMAGE_DATA_SIZE bytes)
 * @param[in]  input_size   Size of @p input in bytes
 * @param[out] output       Output buffer (float array, IC_IMAGE_FEATURES floats)
 * @param[in]  output_size  Size of @p output in floats
 *
 * @return IC_SUCCESS on success, negative IC_ERROR_* on failure
 */
int ic_normalize_rgb888_interleaved(const uint8_t *input, uint32_t input_size,
                                    float *output, uint32_t output_size);

/**
 * @brief Normalize a single RGB image to [0, 1] range
 *
 * Converts uint8_t pixel values (0-255) to float (0.0-1.0).
 * Division by 255.0 for normalization.
 *
 * @param[in]  input           Input image structure with raw pixel data
 * @param[out] output          Output feature buffer for normalized values
 * @param[in]  pixel_layout    Pixel memory layout (interleaved or planar)
 *
 * @return 0 on success, negative value on error
 *
 * @note
 * - Input must contain exactly 96x96x3 = 27648 bytes
 * - Output buffer must be pre-allocated with at least 27648 floats
 * - Optimized for ARM CM55 with MVE/DSP
 */
int ic_normalize_image(
    const ic_image_t *input,
    ic_features_t *output,
    ic_pixel_layout_t pixel_layout
);

/**
 * @brief Normalize image with explicit buffer pointers
 *
 * Simplified API for direct pointer usage.
 *
 * @param[in]  input_data      Raw image data (uint8_t array, 27648 bytes)
 * @param[out] output_data     Output buffer (float array, 27648 floats)
 * @param[in]  pixel_layout    Pixel memory layout
 *
 * @return 0 on success, negative value on error
 *
 * @example
 * uint8_t raw_image[27648];  // 96x96x3
 * float features[27648];
 * ic_normalize_image_ptr(raw_image, features, IC_PIXEL_LAYOUT_INTERLEAVED);
 */
int ic_normalize_image_ptr(
    const uint8_t *input_data,
    float *output_data,
    ic_pixel_layout_t pixel_layout
);

/**
 * @brief Normalize image with RGB channel separation
 *
 * Extract R, G, B channels separately and normalize each.
 * Useful for processing camera raw data.
 *
 * @param[in]  input_data      Raw image data (uint8_t array)
 * @param[out] r_channel       Output R channel (9216 floats)
 * @param[out] g_channel       Output G channel (9216 floats)
 * @param[out] b_channel       Output B channel (9216 floats)
 * @param[in]  pixel_layout    Pixel memory layout
 *
 * @return 0 on success, negative value on error
 *
 * @note Useful for training pipeline compatibility
 */
int ic_normalize_channels(
    const uint8_t *input_data,
    float *r_channel,
    float *g_channel,
    float *b_channel,
    ic_pixel_layout_t pixel_layout
);

/**
 * @brief Validate input image dimensions and data
 *
 * @param[in] input           Input image structure
 *
 * @return 0 if valid, negative value if invalid
 */
int ic_validate_input(const ic_image_t *input);

/* =============================================================================
 * Error Codes
 * ============================================================================= */

#define IC_SUCCESS              0     /**< Operation successful */
#define IC_ERROR_INVALID_INPUT  -1    /**< Invalid input pointer */
#define IC_ERROR_INVALID_OUTPUT -2    /**< Invalid output pointer */
#define IC_ERROR_SIZE_MISMATCH  -3    /**< Input size mismatch */
#define IC_ERROR_INVALID_LAYOUT -4    /**< Invalid pixel layout */
#define IC_ERROR_MEMORY         -5    /**< Memory allocation failed */

/* =============================================================================
 * Version Information
 * ============================================================================= */

#define IC_FEATURE_VERSION_MAJOR    1
#define IC_FEATURE_VERSION_MINOR    0
#define IC_FEATURE_VERSION_PATCH    0

/**
 * @brief Get library version string
 * @return Version string (e.g., "1.0.0")
 */
const char *ic_get_version(void);

#ifdef __cplusplus
}
#endif

#endif /* IC_NORMALIZE_H */


#endif /* C90BE7F4_7AF5_486A_9576_4FB3634E4A05 */
