/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef fmna_motion_detection_h
#define fmna_motion_detection_h

void fmna_motion_detection_init(void);
void fmna_motion_detection_stop(void);
void fmna_motion_detection_start(void);
void fmna_motion_detection_start_active_polling(void);
void motion_detected_handler(void);

#ifdef DEBUG
void fmna_motion_detection_set_separated_ut_backoff_timeout_seconds(uint32_t
                                                                    separated_ut_backoff_timeout_seconds);
#endif //DEBUG

#endif /* fmna_motion_detection_h */
