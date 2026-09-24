/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef __MAIN2_H__
#define __MAIN2_H__

#ifdef __cplusplus
extern "C" {
#endif


int MinimalInferenceWithTime(const unsigned char *model_buf, unsigned int model_len,
                             const void *input, unsigned int input_len);

void PrintUniqueOperators(const void *tflite_model);

void tf_profiler_event_upload(const unsigned char *model_buf, const void *input,
                              unsigned int input_len);

#ifdef __cplusplus
}
#endif

#endif