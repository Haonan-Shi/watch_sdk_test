/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_FLAGS_H_
#define _APP_FLAGS_H_

/** @defgroup  TINYML_KWS_Config TinyML KWS App Configuration
  * @brief This file configures the offline streaming keyword-spotting demo.
  * @{
  */

/* KWS class labels - must match training order. */
#define KWS_NUM_CLASSES        2
#define KWS_LABELS             { "hi_realtek", "noise" }

/* Index of the wake class in KWS_LABELS. */
#define KWS_HIT_INDEX          0

/* Detection threshold (wake-word probability 0-1). */
#define KWS_THRESHOLD          0.80f

/* Consecutive 30ms steps that must stay above KWS_THRESHOLD before firing. */
#define KWS_REQUIRED_HITS      3

/* Tensor-arena size handed to the ts_realtek inference engine (bytes). */
#define KWS_ARENA_SIZE         (80u * 1024u)

/** @} */ /* End of group TINYML_KWS_Config */

#endif /* _APP_FLAGS_H_ */
