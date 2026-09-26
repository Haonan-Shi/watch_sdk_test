/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 *****************************************************************************************
 * @file     main.c
 * @brief    Offline streaming KWS inference with a wake-word model (Zephyr port).
 *
 * This sample mirrors the vendor SDK tinyml_kws demo. It drives a streaming
 * keyword-spotting model with a STATIC PCM clip (kws_sample_pcm.c) instead of a
 * live microphone. The clip is fed to the mel feature frontend frame-by-frame;
 * the model is invoked once per 30ms step (3 x 10ms) and keeps its own
 * ring-buffer state between invokes.
 *
 * Model: streaming wake-word model (see model_tflite.cpp header comment).
 *   Input : int8[1,3,40]  scale=0.1019608  zero_point=-128
 *   Output: uint8[1,1]    wake-word probability (dequantized by ts_realtek)
 *   Frontend feeds RAW uint16 mel; quantize via uint16 * 0.383113 - 128.
 *   Each invoke = 3 frames x 10ms = 30ms of audio.
 *
 * Compared to the bare-metal SDK version, the KWS work runs in a dedicated
 * Zephyr thread, logging goes through printk(), timing uses the kernel cycle
 * counter, and the tensor arena is allocated from the C library heap.
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
#include "kws_frontend.h"
#include "ts_mem.h"
#include "app_flags.h"
#include "tinyml_psram.h"

/*============================================================================*
 *                              Macros
 *============================================================================*/
#define KWS_TASK_PRIORITY        5
/* 20KB is enough for TFLM here; stack overflow was ruled out (32KB still
 * crashed with the same MPU fault). Keep 20KB to leave RAM for debug configs. */
#define KWS_TASK_STACK_SIZE      (20 * 1024)

/* Streaming model parameters (from model_tflite.cpp header comment):
 *   Input shape [1, 3, 40]: 3 time-steps (first_conv stride=3) x 40 mel channels.
 *   Each invoke advances the stream by 3 x 10ms = 30ms. */
#define KWS_FRAMES_PER_STEP      3
#define KWS_INPUT_BYTES          (KWS_FRAMES_PER_STEP * KWS_NUM_CHANNELS)      /* 120 */
#define KWS_SAMPLES_PER_STEP     (KWS_FRAMES_PER_STEP * KWS_SAMPLES_PER_TICK)  /* 480 */

/* Input quantization - reproduces the exact training-time path.
 *
 * The TFLM microfrontend (kws_frontend) outputs RAW uint16 mel energies.
 * Training scales raw uint16 by 0.0390625 (= 1/25.6); the int8 model then
 * quantizes with scale=0.1019608, zero_point=-128. On-device we apply both:
 *   int8 = round(uint16 * 0.0390625 / 0.1019608) - 128
 *        = round(uint16 * 0.383113) - 128 */
#define KWS_INP_SCALE_INV        (0.0390625f / 0.1019607857f)  /* ~= 0.383113 */
#define KWS_INP_ZP               (-128)

/*============================================================================*
 *                              External data
 *============================================================================*/
/* Static PCM test clip (16 kHz, mono, int16) - see kws_sample_pcm.c. */
extern const int16_t      kws_sample_pcm[];
extern const unsigned int kws_sample_pcm_len;

/*============================================================================*
 *                              Variables
 *============================================================================*/
/* Mel feature frontend state (kept out of the thread stack). */
static KwsFrontendState s_fe;

/* Model input: 3 frames x 40 channels = 120 int8 bytes. */
static int8_t s_features_int8[KWS_INPUT_BYTES];

static K_THREAD_STACK_DEFINE(s_kws_stack, KWS_TASK_STACK_SIZE);
static struct k_thread s_kws_thread;

/*============================================================================*
 *                              Functions
 *============================================================================*/

/* Quantize one uint16 mel feature to int8. */
static inline int8_t quantize_feature(uint16_t v)
{
    int32_t q = (int32_t)((float)v * KWS_INP_SCALE_INV + 0.5f) + KWS_INP_ZP;
    if (q < -128) { q = -128; }
    if (q >  127) { q =  127; }
    return (int8_t)q;
}

/* -----------------------------------------------------------------------
 * ts_realtek engine: init once, invoke every 30ms step.
 * ----------------------------------------------------------------------- */
static int kws_engine_init(void)
{
    ts_realtek_register_heap(ts_malloc, ts_free);

    int sret = ts_realtek_set_arena_size(KWS_ARENA_SIZE);
    printk("[TS] set_arena_size(%uKB) ret=%d\n", KWS_ARENA_SIZE / 1024u, sret);

    /* DIAG: pure flat-buffer parse, no heap/HW/auth. If this prints sane
     * numbers, the embedded model buffer is intact and the crash is later. */
    ts_realtek_model_info_t mi;
    int miret = ts_realtek_get_model_info(get_model_pointer(), get_model_size(), &mi);
    printk("[TS] model_info ret=%d in_type=%d out_type=%d in_bytes=%u out_bytes=%u\n",
           miret, (int)mi.input_type, (int)mi.output_type,
           mi.input_bytes, mi.output_bytes);

    printk("[TS] >> register_model\n");
    if (ts_realtek_register_model(get_model_pointer(), get_model_size()) != 0)
    {
        printk("[TS] ERR: ts_realtek_register_model failed\n");
        return -1;
    }
    /* DIAG: probe the Realtek ROM random path in isolation.
     * ts_realtek_init() begins with secure_boot_insert_random_delay() ->
     * platform_random() (ROM 0xc424), which spins up a HW timer (TIM_Cmd) as
     * an entropy source. The MPU fault (blx r9=0xaaaaaaaa inside ROM TIM_Cmd)
     * is suspected to originate here: an uninitialized ROM-RAM function pointer
     * that the full SDK's platform-patch install sets up but this port does not.
     * If the crash reproduces at THIS call, platform_random is the culprit.
     * If it prints a value and the crash is still after ">> init", it is not. */
    extern uint32_t platform_random(uint32_t max);
    printk("[TS] >> probe platform_random\n");
    uint32_t rnd = platform_random(0xFFFFFFFFu);
    printk("[TS] << platform_random ret=0x%08x\n", rnd);

    printk("[TS] >> init\n");   /* crash after THIS line == inside ts_realtek_init() */
    if (ts_realtek_init() != 0)
    {
        printk("[TS] ERR: ts_realtek_init failed\n");
        return -2;
    }
    printk("[TS] << init OK\n");
    return 0;
}

/* Run one streaming inference step.
 * input: int8[KWS_INPUT_BYTES] = 3 quantized mel frames.
 * Returns wake-word probability in [0, 1], or -1.0f on error. */
static float kws_engine_invoke(const int8_t *input)
{
    float out_f = 0.0f;
    uint32_t out_actual = 0;
    int ret = ts_realtek_invoke(input, KWS_INPUT_BYTES,
                                &out_f, sizeof(out_f), &out_actual);
    if (ret != 0)
    {
        printk("[TS] ERR: invoke failed: %d\n", ret);
        return -1.0f;
    }
    return out_f;  /* already dequantized by ts_realtek_invoke */
}

/**
 * @brief  Offline streaming KWS task: static PCM -> mel frontend -> TFLite-Micro.
 *
 * Feeds the embedded clip through the model one 30ms step at a time and
 * reports the wake-word probability trajectory. No microphone is used.
 */
static void kws_main_task(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    static const char *labels[KWS_NUM_CLASSES] = KWS_LABELS;

    /* Initialize TFLM microfrontend (matches training: 40ch, 30ms win, 10ms step, PCAN). */
    if (KwsFrontendInit(&s_fe) != 1)
    {
        printk("[KWS] ERR: KwsFrontendInit failed\n");
        return;
    }

    /* TFLite-Micro engine - initialised once, streaming state preserved across invokes. */
    if (kws_engine_init() != 0)
    {
        printk("[KWS] ERR: kws_engine_init failed\n");
        return;
    }

    uint32_t num_steps = kws_sample_pcm_len / KWS_SAMPLES_PER_STEP;
    printk("KWS offline start: %dHz, %d samples, %d steps (30ms/step)\n",
           KWS_SAMPLE_RATE, (int)kws_sample_pcm_len, (int)num_steps);

    uint8_t consecutive_hits = 0;
    int     best_score_pct   = 0;
    bool    woke             = false;

    for (uint32_t step = 0; step < num_steps; step++)
    {
        const int16_t *step_pcm = kws_sample_pcm + step * KWS_SAMPLES_PER_STEP;

        /* Extract 3 mel feature vectors (one per 10ms frame) and quantize. */
        for (int f = 0; f < KWS_FRAMES_PER_STEP; f++)
        {
            size_t n_read = 0;
            KwsFrontendOutput out = KwsFrontendProcess(
                                        &s_fe,
                                        step_pcm + f * KWS_SAMPLES_PER_TICK,
                                        KWS_SAMPLES_PER_TICK, &n_read);

            if (out.size == 0)
            {
                /* Frontend not ready yet (initial fill); fill zeros for this frame. */
                for (int ch = 0; ch < KWS_NUM_CHANNELS; ch++)
                {
                    s_features_int8[f * KWS_NUM_CHANNELS + ch] = (int8_t)KWS_INP_ZP;
                }
            }
            else
            {
                for (int ch = 0; ch < KWS_NUM_CHANNELS; ch++)
                {
                    s_features_int8[f * KWS_NUM_CHANNELS + ch] =
                        quantize_feature(out.values[ch]);
                }
            }
            (void)n_read;
        }

        /* Run one streaming inference step. */
        uint32_t t1    = k_cycle_get_32();
        float    prob  = kws_engine_invoke(s_features_int8);
        uint32_t t_inf = k_cyc_to_us_floor32(k_cycle_get_32() - t1);

        if (prob < 0.0f) { continue; }

        int score_pct = (int)(prob * 100.0f + 0.5f);
        if (score_pct > best_score_pct) { best_score_pct = score_pct; }

        if (prob >= KWS_THRESHOLD)
        {
            consecutive_hits++;
            if (consecutive_hits >= KWS_REQUIRED_HITS)
            {
                printk("[KWS] *** WAKE UP*** %s (%d%%) step=%d inf=%dus\n",
                       labels[KWS_HIT_INDEX], score_pct, (int)step, (int)t_inf);
                woke = true;
                consecutive_hits = 0;
            }
            else
            {
                printk("[KWS] pre-hit %d/%d %s (%d%%) step=%d inf=%dus\n",
                       (int)consecutive_hits, (int)KWS_REQUIRED_HITS,
                       labels[KWS_HIT_INDEX], score_pct, (int)step, (int)t_inf);
            }
        }
        else
        {
            consecutive_hits = 0;
            if (score_pct >= (int)(KWS_THRESHOLD * 100.0f / 2))
            {
                /* Log moderate scores for tuning (above half-threshold). */
                printk("[KWS] %d%% step=%d inf=%dus\n", score_pct, (int)step, (int)t_inf);
            }
        }
    }

    printk("[KWS] done: %d steps, best=%d%%, wake=%s\n",
           (int)num_steps, best_score_pct, woke ? "YES" : "NO");

    /* Release inference resources. */
    ts_realtek_deinit();
    KwsFrontendFree(&s_fe);
}

int main(void)
{
    printk("TinyML KWS offline sample on %s\n", CONFIG_BOARD_TARGET);

    /* Bring up external PSRAM before any ts_malloc (see src/common). */
    tinyml_psram_init();

    k_thread_create(&s_kws_thread, s_kws_stack, KWS_TASK_STACK_SIZE,
                    kws_main_task, NULL, NULL, NULL,
                    KWS_TASK_PRIORITY, 0, K_NO_WAIT);
    k_thread_name_set(&s_kws_thread, "kws");

    return 0;
}
