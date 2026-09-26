/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*
 * KWS Feature Frontend - public API (no TFLite headers required).
 *
 * Parameters match the wake-word model's training config:
 *   sample_rate      = 16000 Hz
 *   window_size_ms   = 30 ms  (480 samples)
 *   window_step_ms   = 10 ms  (160 samples per inference tick)
 *   num_channels     = 40 mel bins
 *   upper_band_limit = 7500 Hz
 *   lower_band_limit = 125 Hz
 *   enable_pcan      = 1 (noise suppression)
 *
 * Usage:
 *   KwsFrontendState state;
 *   KwsFrontendInit(&state);
 *
 *   size_t samples_read;
 *   KwsFrontendOutput out = KwsFrontendProcess(&state, pcm_buf, 160, &samples_read);
 *   if (out.size == 40) {
 *       // out.values -> const uint16_t[40] mel features (valid until next call)
 *   }
 */

#ifndef KWS_FRONTEND_H_
#define KWS_FRONTEND_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque state buffer sized to hold TFLM struct FrontendState (136 bytes on
 * 32-bit ARM).  All sub-state lives in this flat buffer; KwsFrontendInit only
 * heap-allocates the filter coefficient arrays, not the state struct itself. */
#define KWS_FRONTEND_STATE_BYTES  136U
typedef struct
{
    uint32_t _opaque[KWS_FRONTEND_STATE_BYTES / 4U];
} KwsFrontendState;

/* Feature output.  Memory layout is identical to TFLM struct FrontendOutput so
 * the ABI at the library boundary is unchanged.
 *   values: pointer to uint16[KWS_NUM_CHANNELS], valid until next Process call.
 *   size  : KWS_NUM_CHANNELS when a frame is ready, 0 when more input needed. */
typedef struct
{
    const uint16_t *values;
    size_t          size;
} KwsFrontendOutput;

#define KWS_SAMPLE_RATE       16000
#define KWS_WINDOW_SIZE_MS    30
#define KWS_WINDOW_STEP_MS    10
#define KWS_NUM_CHANNELS      40
#define KWS_SAMPLES_PER_TICK  (KWS_SAMPLE_RATE * KWS_WINDOW_STEP_MS / 1000)  /* 160 */

/* Initialize frontend state. Returns 1 on success, 0 on failure. */
int KwsFrontendInit(KwsFrontendState *state);

/* Process audio samples. Feed exactly KWS_SAMPLES_PER_TICK (160) samples per
 * call for best efficiency. Returns a KwsFrontendOutput with mel features. */
KwsFrontendOutput KwsFrontendProcess(KwsFrontendState *state,
                                     const int16_t *samples,
                                     size_t num_samples,
                                     size_t *num_samples_read);

/* Reset internal ring buffer (call when starting a new audio stream). */
void KwsFrontendReset(KwsFrontendState *state);

/* Free heap allocations inside state (coefficient arrays etc.). */
void KwsFrontendFree(KwsFrontendState *state);

#ifdef __cplusplus
}
#endif

#endif /* KWS_FRONTEND_H_ */
