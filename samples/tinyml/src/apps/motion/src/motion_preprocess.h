/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*
 * motion_preprocess.h  -  TinyMotion raw-window preprocessing & result decode.
 *
 * End-to-end pipeline (no hand-crafted features): a raw accelerometer window is
 * z-score normalized and int8-quantized to form the CNN input, and the int8
 * model output is dequantized to softmax probabilities.
 *
 * Self-built, pure C, no external dependencies (no Edge Impulse code).
 */
#ifndef MOTION_PREPROCESS_H
#define MOTION_PREPROCESS_H

#include <stdint.h>
#include "motion_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Convert a raw window into the int8 model input.
 *
 * @param raw   Input: MOTION_TOTAL_RAW (375) floats, interleaved per timestep
 *              [x0,y0,z0, x1,y1,z1, ...], units m/s^2 (same layout as training).
 * @param out_q Output: MOTION_TOTAL_RAW (375) int8 values, ready for inference.
 *
 * Applies per-axis z-score (MOTION_NORM_MEAN/STD) then int8 quantization
 * (MOTION_IN_SCALE/ZERO) with saturation to [-128, 127].
 */
void motion_preprocess_quantize(const float *raw, int8_t *out_q);

/**
 * Dequantize the model's int8 output tensor into float softmax probabilities.
 *
 * @param out_q  Input : MOTION_N_CLASSES (4) int8 values from ts_realtek_invoke.
 * @param probs  Output: MOTION_N_CLASSES (4) floats in [0,1].
 */
void motion_dequantize_output(const int8_t *out_q, float *probs);

/**
 * Pick the class and apply the confidence-threshold open-set gate.
 *
 * @param probs       Input : MOTION_N_CLASSES probabilities.
 * @param out_conf    Optional: receives the max probability (may be NULL).
 * @return class index 0..3, or MOTION_CLASS_UNKNOWN (-1) if max prob is below
 *         MOTION_CONF_THRESHOLD (rejected as not-a-known-motion).
 */
int motion_classify(const float *probs, float *out_conf);

/** Human-readable class names, indexed by class id (0..3). */
extern const char *const MOTION_CLASS_NAMES[MOTION_N_CLASSES];

#ifdef __cplusplus
}
#endif

#endif /* MOTION_PREPROCESS_H */
