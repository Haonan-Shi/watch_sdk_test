/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 *****************************************************************************************
 * @file     main.c
 * @brief    TinyML image classification - 96x96 RGB CNN (Zephyr port).
 *
 * This sample mirrors the vendor SDK tinyml_ic demo. A raw 96x96 RGB image
 * (embedded in ic_image_samples.c) is normalized to [0,1] float in plain C
 * (ic_normalize.c), fed to a convolutional classifier (model_tflite.cpp) via
 * the ts_realtek TFLite-Micro engine, and the softmax output is printed.
 *
 *   raw_image (96x96x3 uint8)
 *        -> ic_normalize_rgb888_interleaved   ([0,255] -> [0,1] float)
 *        -> ts_realtek_invoke                 (CNN -> float softmax)
 *        -> argmax + score print
 *
 * Compared to the bare-metal SDK version, the work runs in a dedicated Zephyr
 * thread, logging goes through printk(), and the tensor arena is allocated
 * from the C library heap.
 *****************************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/devicetree.h>

#include "ts_realtek.h"
#include "model_tflite.h"
#include "ts_mem.h"
#include "ic_normalize.h"
#include "app_flags.h"
#include "tinyml_psram.h"

/*============================================================================*
 *                              Macros
 *============================================================================*/
#define IC_TASK_PRIORITY     5
/* TFLM invoke uses deep call stacks; give the worker its own stack. */
#define IC_TASK_STACK_SIZE   (20 * 1024)

/*============================================================================*
 *                              External data
 *============================================================================*/
/* Embedded 96x96 RGB demo image (see ic_image_samples.c). */
extern const uint8_t raw_image[];

/*============================================================================*
 *                              Variables
 *============================================================================*/
static K_THREAD_STACK_DEFINE(s_ic_stack, IC_TASK_STACK_SIZE);
static struct k_thread s_ic_thread;

/*============================================================================*
 *                              Functions
 *============================================================================*/

static void ts_log_cb(const char *msg)
{
    printk("[TS-LIB] %s\n", msg);
}

/* -----------------------------------------------------------------------
 * ts_run_inference - direct ts_realtek API, no TFLite headers required.
 * Assumes a float32 softmax output tensor; num_classes is derived from the
 * out_actual_len returned by ts_realtek_invoke.
 * ----------------------------------------------------------------------- */
static int ts_run_inference(const void *input, uint32_t input_len)
{
    ts_realtek_register_log(ts_log_cb);
    ts_realtek_register_heap(ts_malloc, ts_free);

    int sret = ts_realtek_set_arena_size(IC_ARENA_SIZE);
    printk("[TS] set_arena_size(%uKB) ret=%d\n", IC_ARENA_SIZE / 1024u, sret);

    /* Diagnostics (buffer-only, no init required): reveals the real input
     * tensor type/size so the input we feed can be validated at runtime. */
    ts_realtek_model_info_t info;
    int mi = ts_realtek_get_model_info(get_model_pointer(), get_model_size(), &info);
    if (mi == 0)
    {
        printk("[TS] model_info: in_type=%d out_type=%d in_bytes=%u out_bytes=%u\n",
               (int)info.input_type, (int)info.output_type,
               info.input_bytes, info.output_bytes);
    }

    int rret = ts_realtek_register_model(get_model_pointer(), get_model_size());
    printk("[TS] register_model ret=%d, model_ptr=%p, size=%u\n",
           rret, get_model_pointer(), (unsigned)get_model_size());
    if (rret != 0)
    {
        printk("[TS] ERR: ts_realtek_register_model failed\n");
        return -1;
    }

    int iret = ts_realtek_init();
    printk("[TS] ts_realtek_init ret=%d\n", iret);
    if (iret != 0)
    {
        printk("[TS] ERR: ts_realtek_init failed (%d)\n", iret);
        return -2;
    }

    float output_buf[128]; /* covers up to 128-class output */
    uint32_t out_actual_len = 0;
    int ret = ts_realtek_invoke(input, input_len,
                                output_buf, sizeof(output_buf), &out_actual_len);
    if (ret != 0)
    {
        ts_realtek_deinit();
        printk("[TS] ERR: ts_realtek_invoke failed: %d\n", ret);
        return -3;
    }

    printk("[TS] inference cycles: %u\n", (unsigned)ts_realtek_last_invoke_cycles());

    uint32_t num_classes = out_actual_len / sizeof(float);
    int best = 0;
    float best_p = (num_classes > 0) ? output_buf[0] : 0.0f;
    for (uint32_t j = 0; j < num_classes; ++j)
    {
        int p_i = (int)(output_buf[j] * 10000.0f + 0.5f);
        printk("  class[%u] = 0.%04d\n", j, p_i);
        if (output_buf[j] > best_p) { best_p = output_buf[j]; best = (int)j; }
    }
    printk("[TS] RESULT: class = %d, conf = 0.%04d\n",
           best, (int)(best_p * 10000.0f + 0.5f));

    ts_realtek_deinit();
    return 0;
}

/**
 * @brief  App task: normalize the embedded image and run one classification.
 */
static void ic_main_task(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    float *features = ts_malloc(sizeof(float) * IC_IMAGE_FEATURES);
    if (features == NULL)
    {
        printk("[TS] failed: out of memory\n");
        return;
    }

    int ret = ic_normalize_rgb888_interleaved(raw_image, IC_IMAGE_DATA_SIZE,
                                              features, IC_IMAGE_FEATURES);
    if (ret == IC_SUCCESS)
    {
        int infer_ret = ts_run_inference(features, IC_FEATURE_SIZE);
        printk("[TS] tflite model infer done, ret = %d\n", infer_ret);
    }
    else
    {
        printk("[TS] image normalize failed: %d\n", ret);
    }

    ts_free(features);
}

int main(void)
{
    printk("TinyML image-classification sample on %s\n", CONFIG_BOARD_TARGET);

    /* Bring up external PSRAM before any ts_malloc (see src/common). */
    tinyml_psram_init();

    k_thread_create(&s_ic_thread, s_ic_stack, IC_TASK_STACK_SIZE,
                    ic_main_task, NULL, NULL, NULL,
                    IC_TASK_PRIORITY, 0, K_NO_WAIT);
    k_thread_name_set(&s_ic_thread, "ic");

    return 0;
}
