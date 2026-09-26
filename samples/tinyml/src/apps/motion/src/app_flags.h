/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*
 * app_flags.h - TinyMotion sample tunables (precompiled header).
 *
 * The data-spec / normalization / quantization constants live in
 * motion_config.h (they must match the training pipeline). This header only
 * carries the on-device knobs specific to the Zephyr port.
 */
#ifndef _APP_FLAGS_H_
#define _APP_FLAGS_H_

/* Tensor arena for the int8 1D-CNN (~25 KB flat-buffer). 80 KB matches the
 * vendor SDK reference and leaves head-room over the measured arena_used. */
#define MOTION_ARENA_SIZE   (80u * 1024u)

/* Select the embedded demo window at build time (see motion_demo_samples.h):
 *   0 IDLE  1 SNAKE  2 UPDOWN  3 WAVE  4 UNKNOWN (open-set reject)
 * Override with -DMOTION_TEST_CASE=<n>. Defaults to UNKNOWN in that header. */

#endif /* _APP_FLAGS_H_ */
