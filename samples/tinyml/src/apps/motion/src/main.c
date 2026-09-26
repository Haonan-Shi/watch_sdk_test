/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 *****************************************************************************************
 * @file     main.c
 * @brief    TinyMotion - 1D-CNN motion-trajectory recognition (Zephyr port).
 *
 * This sample mirrors the vendor SDK tinyml_motion demo. A raw accelerometer
 * window (125 x 3 float, m/s^2, embedded in motion_demo_samples.h) is z-score
 * normalized and int8-quantized in plain C, fed to an int8 CNN via the
 * ts_realtek TFLite-Micro engine, and the int8 softmax output is decoded to a
 * class with a confidence-threshold open-set (OOD) gate.
 *
 *   raw window (125 x 3 float, interleaved [x0,y0,z0,x1,...])
 *        -> motion_preprocess_quantize()   [z-score + int8 quant]
 *        -> ts_realtek_invoke()            [int8 CNN, 375B in -> float softmax]
 *        -> motion_classify()              [argmax + reject-if-low-confidence]
 *
 * There is NO hand-crafted DSP feature extraction and NO feature library:
 * the whole preprocess path is in motion_preprocess.c. ts_realtek_invoke()
 * dequantizes the int8 softmax output tensor to float32 internally, so the
 * caller receives ready-to-use probabilities.
 *
 * Compared to the bare-metal SDK version, the work runs in a dedicated Zephyr
 * thread, logging goes through printk(), timing uses the kernel cycle counter,
 * and the tensor arena is allocated from the C library heap.
 *****************************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "ts_realtek.h"
#include "model_tflite.h"
#include "ts_mem.h"
#include "app_flags.h"
#include "motion_preprocess.h"
#include "motion_demo_samples.h"
#include "tinyml_psram.h"

/*============================================================================*
 *                              Macros
 *============================================================================*/
#define MOTION_TASK_PRIORITY     5
/* TFLM invoke uses deep call stacks; give the worker its own stack. */
#define MOTION_TASK_STACK_SIZE   (20 * 1024)

/*============================================================================*
 *                              Variables
 *============================================================================*/
/* Scratch buffers (static to keep them off the task stack). */
static int8_t s_input_q[MOTION_TOTAL_RAW];   /* 375 int8 model input          */
static float  s_probs[MOTION_N_CLASSES];     /* 4 softmax probs (dequantized) */

static K_THREAD_STACK_DEFINE(s_motion_stack, MOTION_TASK_STACK_SIZE);
static struct k_thread s_motion_thread;

/*============================================================================*
 *                              Functions
 *============================================================================*/

/* Forward TFLite-Micro / ts_realtek log messages to the console. This surfaces
 * the exact reason for an init failure (unsupported op, schema version, ...). */
static void ts_log_cb(const char *msg)
{
    printk("[TSLOG] %s\n", msg);
}

/* -----------------------------------------------------------------------
 * ts_run_inference_int8 - run the int8 CNN on a pre-quantized int8 window.
 * Direct ts_realtek API, no TFLite headers required. Model is fully int8
 * (input int8[1,125,3], output int8[1,4]); ts_realtek_invoke dequantizes the
 * int8 output tensor to float32 softmax and writes it into out_probs, so
 * out_len must be >= MOTION_N_CLASSES floats.
 * ----------------------------------------------------------------------- */
static int ts_run_inference_int8(const int8_t *input_q, uint32_t input_len,
                                 float *out_probs, uint32_t out_len)
{
    ts_realtek_register_heap(ts_malloc, ts_free);
    ts_realtek_register_log(ts_log_cb);

    int sret = ts_realtek_set_arena_size(MOTION_ARENA_SIZE);
    printk("[TM] set_arena_size(%uKB) ret=%d\n", MOTION_ARENA_SIZE / 1024u, sret);

    /* Diagnostics (buffer-only, no init required). */
    ts_realtek_model_info_t info;
    int mi = ts_realtek_get_model_info(get_model_pointer(), get_model_size(), &info);
    if (mi == 0)
    {
        printk("[TM] model_info: in_type=%d out_type=%d in_bytes=%u out_bytes=%u\n",
               (int)info.input_type, (int)info.output_type,
               info.input_bytes, info.output_bytes);
        printk("[TM] model_info: in_elems=%u out_elems=%u in_zp=%d out_zp=%d\n",
               info.input_elem_count, info.output_elem_count,
               (int)info.input_zero_point, (int)info.output_zero_point);
    }
    else
    {
        printk("[TM] ERR: ts_realtek_get_model_info failed: %d\n", mi);
    }

    char ops[256];
    if (ts_realtek_get_unique_ops(get_model_pointer(), get_model_size(),
                                  ops, sizeof(ops)) == 0)
    {
        printk("[TM] model ops: %s\n", ops);
    }

    if (ts_realtek_register_model(get_model_pointer(), get_model_size()) != 0)
    {
        printk("[TM] ERR: ts_realtek_register_model failed\n");
        return -1;
    }
    int init_ret = ts_realtek_init();
    if (init_ret != 0)
    {
        printk("[TM] ERR: ts_realtek_init failed, err=%d\n", init_ret);
        return -2;
    }

    uint32_t out_actual_len = 0;
    int ret = ts_realtek_invoke(input_q, input_len,
                                out_probs, out_len, &out_actual_len);
    if (ret != 0)
    {
        ts_realtek_deinit();
        printk("[TM] ERR: ts_realtek_invoke failed: %d\n", ret);
        return -3;
    }

    printk("[TM] inference cycles: %u, output bytes: %u\n",
           (unsigned)ts_realtek_last_invoke_cycles(), (unsigned)out_actual_len);

    ts_realtek_deinit();
    return 0;
}

/**
 * @brief  App task: run one TinyMotion inference on the embedded demo window.
 */
static void motion_main_task(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    printk("[TM] Test case: %s (expected -> %s)\n", DEMO_CASE_LABEL,
           (DEMO_CASE_EXPECTED_CLS == MOTION_CLASS_UNKNOWN) ? "unknown (rejected)" :
           MOTION_CLASS_NAMES[DEMO_CASE_EXPECTED_CLS]);

    /* 1) Preprocess: z-score + int8 quantization (pure C, no feature lib). */
    uint32_t t0 = k_cycle_get_32();
    motion_preprocess_quantize(g_demo_window, s_input_q);
    uint32_t t_pre = k_cyc_to_us_floor32(k_cycle_get_32() - t0);
    printk("[TM] preprocess time: %u us\n", (unsigned)t_pre);

    /* 2) Inference: int8 CNN via ts_realtek (375B int8 in -> float softmax out). */
    int infer_ret = ts_run_inference_int8(s_input_q, MOTION_TOTAL_RAW,
                                          s_probs, MOTION_N_CLASSES * sizeof(float));
    if (infer_ret != 0)
    {
        printk("[TM] ERR: tflite model infer failed, ret = %d\n", infer_ret);
        return;
    }

    /* 3) Decode: s_probs already holds float softmax probs. Print as 0.xxxx. */
    for (int c = 0; c < MOTION_N_CLASSES; ++c)
    {
        int p_i = (int)(s_probs[c] * 10000.0f + 0.5f);
        printk("  probs[%d] %s = 0.%04d\n", c, MOTION_CLASS_NAMES[c], p_i);
    }

    /* 4) Classify with open-set (confidence) gate. */
    float conf = 0.0f;
    int cls = motion_classify(s_probs, &conf);
    int conf_i = (int)(conf * 10000.0f + 0.5f);
    const char *result_str;
    if (cls == MOTION_CLASS_UNKNOWN)
    {
        result_str = "unknown (rejected)";
        printk("[TM] RESULT: unknown (rejected), max conf = 0.%04d\n", conf_i);
    }
    else
    {
        result_str = MOTION_CLASS_NAMES[cls];
        printk("[TM] RESULT: class = %s, conf = 0.%04d\n",
               MOTION_CLASS_NAMES[cls], conf_i);
    }

    if (cls == DEMO_CASE_EXPECTED_CLS)
    {
        printk("[TM] PASS: got expected result '%s'\n", result_str);
    }
    else
    {
        printk("[TM] FAIL: expected '%s', got '%s'\n", DEMO_CASE_LABEL, result_str);
    }
}

int main(void)
{
    printk("TinyML motion offline sample on %s\n", CONFIG_BOARD_TARGET);

    /* Bring up external PSRAM before any ts_malloc (see src/common). */
    tinyml_psram_init();

    k_thread_create(&s_motion_thread, s_motion_stack, MOTION_TASK_STACK_SIZE,
                    motion_main_task, NULL, NULL, NULL,
                    MOTION_TASK_PRIORITY, 0, K_NO_WAIT);
    k_thread_name_set(&s_motion_thread, "motion");

    return 0;
}
