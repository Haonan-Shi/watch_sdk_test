/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*
 * motion_config.h  -  TinyMotion MCU inference configuration.
 *
 * Self-built (no Edge Impulse code). All constants here MUST match the Python
 * training pipeline exactly:
 *   - data spec   : src/config.py   (62.5 Hz, 125 samples, 3 axes, 4 classes)
 *   - z-score     : artifacts/norm_params.json  (per-axis mean/std)
 *   - int8 quant  : tinymotion_cnn_int8.tflite  (input/output scale & zero-point)
 *   - OOD (opt.)  : artifacts/ood_params.json   (Mahalanobis threshold)
 *
 */
#ifndef MOTION_CONFIG_H
#define MOTION_CONFIG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Data spec (must match src/config.py) --------------------------------*/
#define MOTION_SAMPLING_HZ          62.5f
#define MOTION_AXES                 3        /* accX, accY, accZ            */
#define MOTION_WINDOW               125      /* samples per window (2 s)    */
#define MOTION_TOTAL_RAW            (MOTION_WINDOW * MOTION_AXES)  /* 375   */
#define MOTION_N_CLASSES            4

/* Class label order (index == model output index). */
#define MOTION_CLASS_IDLE           0
#define MOTION_CLASS_SNAKE          1
#define MOTION_CLASS_UPDOWN         2
#define MOTION_CLASS_WAVE           3

/* ---- z-score normalization (artifacts/norm_params.json) ------------------*
 * normalized[axis] = (raw[axis] - MOTION_NORM_MEAN[axis]) / MOTION_NORM_STD[axis]
 * SUBTRACT_MEAN=false, LOWPASS=none  (see src/config.py).                   */
static const float MOTION_NORM_MEAN[MOTION_AXES] =
{
    0.08868937f, 0.65972757f, 4.66419983f
};
static const float MOTION_NORM_STD[MOTION_AXES] =
{
    7.23031569f, 4.98954248f, 6.17046642f
};

/* ---- int8 quantization params (from tinymotion_cnn_int8.tflite) ----------*
 * input  : q = round(normalized / IN_SCALE) + IN_ZERO,  clamp [-128,127]
 * output : prob = (q - OUT_ZERO) * OUT_SCALE            (softmax, 0..1)     */
#define MOTION_IN_SCALE             0.03130468726158142f
#define MOTION_IN_ZERO              3
#define MOTION_OUT_SCALE            0.00390625f
#define MOTION_OUT_ZERO             (-128)

/* ---- Open-set rejection --------------------------------------------------*
 * On-device gate (works with the single-output ts_realtek API):
 *   if max_prob < MOTION_CONF_THRESHOLD -> reject as "unknown".
 * Tune to trade off acceptance vs rejection. See README for the stronger
 * Mahalanobis-embedding path (requires an embedding-model export).          */
#define MOTION_CONF_THRESHOLD       0.60f

/* Mahalanobis threshold from artifacts/ood_params.json (host reference; used
 * only if an embedding model is added - see README OOD section).            */
#define MOTION_OOD_MAHALANOBIS_THRESH   142.58f
#define MOTION_OOD_EMBED_DIM            64

#define MOTION_CLASS_UNKNOWN        (-1)

#ifdef __cplusplus
}
#endif

#endif /* MOTION_CONFIG_H */
