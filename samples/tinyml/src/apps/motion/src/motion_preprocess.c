/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*
 * motion_preprocess.c - TinyMotion raw-window preprocessing & result decode.
 *
 * Self-built, pure C. See motion_preprocess.h / motion_config.h.
 */
#include "motion_preprocess.h"
#include <math.h>

const char *const MOTION_CLASS_NAMES[MOTION_N_CLASSES] =
{
    "idle", "snake", "updown", "wave"
};

static inline int8_t saturate_int8(int32_t v)
{
    if (v < -128) { return (int8_t) - 128; }
    if (v >  127) { return (int8_t)  127; }
    return (int8_t)v;
}

void motion_preprocess_quantize(const float *raw, int8_t *out_q)
{
    /* Layout is [t0_x,t0_y,t0_z, t1_x,...] with 125 timesteps x 3 axes.
     * z-score uses per-axis mean/std; quantize with the model's input params. */
    const float inv_scale = 1.0f / MOTION_IN_SCALE;

    for (int t = 0; t < MOTION_WINDOW; ++t)
    {
        for (int a = 0; a < MOTION_AXES; ++a)
        {
            int idx = t * MOTION_AXES + a;
            float norm = (raw[idx] - MOTION_NORM_MEAN[a]) / MOTION_NORM_STD[a];
            int32_t q = (int32_t)lroundf(norm * inv_scale) + MOTION_IN_ZERO;
            out_q[idx] = saturate_int8(q);
        }
    }
}

void motion_dequantize_output(const int8_t *out_q, float *probs)
{
    for (int c = 0; c < MOTION_N_CLASSES; ++c)
    {
        probs[c] = ((float)out_q[c] - (float)MOTION_OUT_ZERO) * MOTION_OUT_SCALE;
    }
}

int motion_classify(const float *probs, float *out_conf)
{
    int best = 0;
    float best_p = probs[0];
    for (int c = 1; c < MOTION_N_CLASSES; ++c)
    {
        if (probs[c] > best_p) { best_p = probs[c]; best = c; }
    }
    if (out_conf) { *out_conf = best_p; }

    if (best_p < MOTION_CONF_THRESHOLD)
    {
        return MOTION_CLASS_UNKNOWN;   /* open-set rejection */
    }
    return best;
}
