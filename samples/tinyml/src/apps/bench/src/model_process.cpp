/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "trace.h"
#include "model_process.h"
#include "ts_mem.h"
#include "ts_realtek.h"
#include "uart_packet_parser.h"
#include "utils.h"

/* -------------------------------------------------------------------------
 * Forward declarations of host-side UART glue (defined elsewhere).
 * ---------------------------------------------------------------------- */
extern "C" void tf_result_buffer_add(const char *str);
extern "C" void tf_result_buffer_send_all(void);
extern "C" void tf_quant_param_buffer_add(const char *str);
extern "C" void tf_operator_buffer_add(const char *str);
extern "C" void tf_profiler_buffer_add(const char *str);

/* -------------------------------------------------------------------------
 * Tunables
 * ---------------------------------------------------------------------- */

/* Tensor arena size used for inference (first candidate in scan loop).
 *
 * Sized for the worst-case model the host actually exercises today:
 *   - kws.lite          :  ~ 30 KB
 *   - hey_realtek (LSTM):  ~ 80 KB
 *   - IC MobileNet INT8 : ~250 KB
 *
 * ts_malloc/ts_free are backed by the PSRAM heap (psRam_heap.c,
 * configTOTAL_psRAM_HEAP_SIZE = 1 MB, located in the PSRAM1_MCU region).
 * That heap is shared with model_buf / data_buf / UART queues and the
 * ~80 KB MicroProfiler, so the arena, profiler and streamed buffers all fit
 * without touching the small on-chip DATA_ON heap. */
#ifndef TINYML_EDGE_ARENA_BYTES
#define TINYML_EDGE_ARENA_BYTES (192u * 1024u)
#endif

/* Max bytes captured for the profiler text dump (per inference). */
#ifndef TINYML_EDGE_PROFILER_BUF_BYTES
#define TINYML_EDGE_PROFILER_BUF_BYTES 2048u
#endif

/* -------------------------------------------------------------------------
 * Trivial log shim used as the library's log callback.
 * ---------------------------------------------------------------------- */
extern "C" void ts_log(const char *s)
{
    DBG_DIRECT("%s", s);
}

/* -------------------------------------------------------------------------
 * Helpers
 * ---------------------------------------------------------------------- */

/* Format a float as "%d.%08d" using only integer math (the Realtek log
 * implementation does not support %f). Always positive sign in fractional
 * part; the integer part carries the sign. */
static void format_float_8frac(float v, char *out, size_t cap)
{
    int int_part  = (int)v;
    long frac_l   = (long)((double)(v - (float)int_part) * 100000000.0);
    if (frac_l < 0) { frac_l = -frac_l; }
    snprintf(out, cap, "%d.%08ld", int_part, frac_l);
}

/* Format a float as "%d.%02d" (used for inference result digits). */
static void format_float_2frac(float v, char *out, size_t cap)
{
    int ival      = (int)(v * 100.0f + (v >= 0.0f ? 0.5f : -0.5f));
    int int_part  = ival / 100;
    int frac_part = ival % 100;
    if (frac_part < 0) { frac_part = -frac_part; }
    snprintf(out, cap, "%d.%02d", int_part, frac_part);
}

static const char *dtype_name(int32_t dt)
{
    switch (dt)
    {
    case TS_REALTEK_DTYPE_FLOAT32: return "float";
    case TS_REALTEK_DTYPE_INT8:    return "int8";
    case TS_REALTEK_DTYPE_UINT8:   return "uint8";
    case TS_REALTEK_DTYPE_INT16:   return "int16";
    case TS_REALTEK_DTYPE_INT32:   return "int32";
    default:                       return "unknown";
    }
}

/* -------------------------------------------------------------------------
 * Public C API #1: GetQuantConfigInfoString
 *
 * Builds the human-readable input/output type+scale+zero_point string and
 * pushes it into the host's quant-param UART buffer.
 * ---------------------------------------------------------------------- */
#ifdef __cplusplus
extern "C" {
#endif

void GetQuantConfigInfoString(const char *model_buf, char *out_buf, size_t bufsize)
{
    if (model_buf == NULL || out_buf == NULL || bufsize == 0u)
    {
        DBG_DIRECT("[DEBUG] Invalid input to GetQuantConfigInfoString");
        return;
    }

    ts_realtek_model_info_t info;
    int rc = ts_realtek_get_model_info((const unsigned char *)model_buf,
                                       0u, /* size unused */
                                       &info);
    if (rc != TS_REALTEK_OK)
    {
        snprintf(out_buf, bufsize, "ERR: ts_realtek_get_model_info=%d", rc);
        DBG_DIRECT("[DEBUG] %s", out_buf);
        tf_quant_param_buffer_add(out_buf);
        return;
    }

    char in_scale[32], in_zp[32], out_scale[32], out_zp[32];
    if (info.input_type == TS_REALTEK_DTYPE_FLOAT32 || info.input_scale == 0.0f)
    {
        snprintf(in_scale, sizeof(in_scale), "0.00000");
        snprintf(in_zp,    sizeof(in_zp),    "0.00000");
    }
    else
    {
        format_float_8frac(info.input_scale, in_scale, sizeof(in_scale));
        snprintf(in_zp, sizeof(in_zp), "%d", (int)info.input_zero_point);
    }
    if (info.output_type == TS_REALTEK_DTYPE_FLOAT32 || info.output_scale == 0.0f)
    {
        snprintf(out_scale, sizeof(out_scale), "0.00000");
        snprintf(out_zp,    sizeof(out_zp),    "0.00000");
    }
    else
    {
        format_float_8frac(info.output_scale, out_scale, sizeof(out_scale));
        snprintf(out_zp, sizeof(out_zp), "%d", (int)info.output_zero_point);
    }

    snprintf(out_buf, bufsize - 1u,
             "\n        Input type: %s, Input scale: %s, Input zero point: %s"
             "\n        Output type: %s, Output scale: %s, Output zero point: %s\n",
             dtype_name(info.input_type),  in_scale,  in_zp,
             dtype_name(info.output_type), out_scale, out_zp);
    out_buf[bufsize - 1u] = '\0';

    DBG_DIRECT("-->%s", out_buf);
    tf_quant_param_buffer_add(out_buf);
}

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------
 * Public C API #2: PrintUniqueOperators
 *
 * Lists the unique builtin operators used by the model and pushes them
 * into the host's operator UART buffer.
 * ---------------------------------------------------------------------- */
extern "C" void PrintUniqueOperators(const void *tflite_model)
{
    if (!tflite_model) { return; }

    /* The library writes a comma-separated list straight into our buffer. */
    char ops[224] = {0};
    int rc = ts_realtek_get_unique_ops((const unsigned char *)tflite_model,
                                       0u, /* size unused */
                                       ops, sizeof(ops) - 1u);
    if (rc != TS_REALTEK_OK)
    {
        char err_msg[64];
        snprintf(err_msg, sizeof(err_msg),
                 "UniqueOps: ERR: ts_realtek_get_unique_ops=%d", rc);
        tf_operator_buffer_add(err_msg);
        return;
    }
    /* Defensive truncation to keep the wire payload <= 220 chars total. */
    if (strlen(ops) > 200u)
    {
        ops[200] = '\0';
    }
    char op_buf[256];
    snprintf(op_buf, sizeof(op_buf), "UniqueOps: [%s]", ops);
    tf_operator_buffer_add(op_buf);
}

/* -------------------------------------------------------------------------
 * Public C API #3: MinimalInferenceWithTime
 *
 * Registers heap+model, runs a single inference via ts_realtek, and pushes
 * the inference time + per-class probabilities into the host's UART
 * buffers. Output is always interpreted as float32 because the library
 * already dequantises INT8/UINT8 outputs internally.
 * ---------------------------------------------------------------------- */
extern "C" int MinimalInferenceWithTime(const unsigned char *model_buf,
                                        unsigned int         model_len,
                                        const void          *input,
                                        unsigned int         input_len)
{
    if (model_buf == NULL || model_len == 0u)
    {
        tf_result_buffer_add("ERR: model_buf is NULL or model_len is 0");
        return 1;
    }

    /* Wire up heap, log, model. */
    ts_realtek_register_heap(ts_malloc, ts_free);
    ts_realtek_register_log(ts_log);
    if (ts_realtek_register_model(model_buf, model_len) != TS_REALTEK_OK)
    {
        tf_result_buffer_add("ERR: ts_realtek_register_model failed");
        return 2;
    }

    /* Phase 1 - PROBE: find the largest arena that ts_malloc can satisfy
     * right now.  On success the arena stays allocated (no deinit) so the
     * interpreter created by init stays alive for inference below.  Only
     * failed attempts are deinit'd - re-init after deinit is fragile when
     * log callback changes happen in between (see tf_profiler_event_upload). */
    static const uint32_t k_arena_candidates[] =
    {
        TINYML_EDGE_ARENA_BYTES,
        192u * 1024u,
        128u * 1024u,
        96u * 1024u,
        64u * 1024u,
        32u * 1024,
    };
    int rc = TS_REALTEK_ERR_MALLOC;
    for (size_t a = 0; a < sizeof(k_arena_candidates) / sizeof(k_arena_candidates[0]); ++a)
    {
        ts_realtek_set_arena_size(k_arena_candidates[a]);
        rc = ts_realtek_init();
        if (rc == TS_REALTEK_OK)
        {
            DBG_DIRECT("ts_realtek_init: arena=%d OK", (int)k_arena_candidates[a]);
            break;  /* arena stays allocated for inference */
        }
        ts_realtek_deinit(); /* clean up failed attempt */
        if (rc != TS_REALTEK_ERR_MALLOC && rc != TS_REALTEK_ERR_ALLOC_FAIL)
        {
            break; /* non-memory error: arena size won't help */
        }
    }
    if (rc != TS_REALTEK_OK)
    {
        tf_result_buffer_add("ERR: no arena size succeeded");
        return 3;
    }

    /* Discover output element count from the model schema (no interpreter
     * access needed). */
    ts_realtek_model_info_t info;
    if (ts_realtek_get_model_info(model_buf, model_len, &info) != TS_REALTEK_OK)
    {
        ts_realtek_deinit();
        tf_result_buffer_add("ERR: ts_realtek_get_model_info failed");
        return 4;
    }
    const uint32_t num_classes = (info.output_elem_count > 0u) ? info.output_elem_count : 1u;

    /* The library always emits float32 output (auto-dequantised). */
    const uint32_t out_buf_bytes = num_classes * (uint32_t)sizeof(float);
    float *output_buf = (float *)ts_malloc(out_buf_bytes);
    if (!output_buf)
    {
        ts_realtek_deinit();
        tf_result_buffer_add("ERR: ts_malloc output_buf failed");
        return 5;
    }

    uint32_t actual_out_len = 0u;
    rc = ts_realtek_invoke(input, input_len, output_buf, out_buf_bytes, &actual_out_len);
    if (rc != TS_REALTEK_OK)
    {
        char err_msg[64];
        snprintf(err_msg, sizeof(err_msg), "ERR: ts_realtek_invoke failed: %d", rc);
        tf_result_buffer_add(err_msg);
        ts_free(output_buf);
        ts_realtek_deinit();
        return 6;
    }

    /* Inference time in CPU cycles. */
    {
        char time_buf[40];
        uint32_t cycles = ts_realtek_last_invoke_cycles();
        snprintf(time_buf, sizeof(time_buf), "InferTime: %lu cycles", (unsigned long)cycles);

        if (uart_tx_parser.inference_time_buffer)
        {
            ts_free(uart_tx_parser.inference_time_buffer);
            uart_tx_parser.inference_time_buffer = NULL;
            uart_tx_parser.inference_time_buf_size = 0;
        }
        const size_t need = strlen(time_buf) + 1u;
        uart_tx_parser.inference_time_buf_size = need;
        uart_tx_parser.inference_time_buffer   = (char *)ts_malloc(need);
        if (uart_tx_parser.inference_time_buffer)
        {
            memcpy(uart_tx_parser.inference_time_buffer, time_buf, need);
        }
    }

    /* Per-class probability list "[a.bc, ...]". */
    {
        const size_t list_cap = (size_t)num_classes * 12u + 8u;
        char *result_str = (char *)ts_malloc(list_cap);
        if (result_str)
        {
            result_str[0] = '\0';
            strcat(result_str, "[");
            for (uint32_t j = 0u; j < num_classes; ++j)
            {
                char tmp[16];
                format_float_2frac(output_buf[j], tmp, sizeof(tmp));
                if (j != 0u) { strcat(result_str, ", "); }
                strcat(result_str, tmp);
            }
            strcat(result_str, "]");
            tf_result_buffer_add(result_str);
            ts_free(result_str);
        }
        else
        {
            tf_result_buffer_add("ERR: malloc result_str failed");
        }
    }

    /* Capture arena_used_bytes BEFORE deinit. */
    {
        uint32_t used = 0u;
        if (ts_realtek_get_arena_used(&used) == TS_REALTEK_OK)
        {
            uart_tx_parser.arena_used_bytes = (int)used;
        }
    }

    ts_free(output_buf);
    ts_realtek_deinit();
    return 0;
}

/* -------------------------------------------------------------------------
 * Public C API #4: tf_profiler_event_upload
 *
 * Runs one profiled inference and pushes the formatted profiler dump into
 * the host's profiler UART buffer. The library emits multi-line text via
 * the registered log callback; we capture into a 2 KB ring-free buffer
 * with the same indentation post-processing as the original implementation
 * (first line at column 0, subsequent lines indented by 8 spaces).
 * ---------------------------------------------------------------------- */
static char  *g_profiler_log_buf       = NULL;
static size_t g_profiler_log_bufsize   = 0u;
static size_t g_profiler_log_offset    = 0u;
static int    g_profiler_log_line      = 0;
static bool   g_profiler_log_indent_pending = true;

static void profiler_capture_cb(const char *s)
{
    if (!g_profiler_log_buf || !s) { return; }
    static const char kIndent[]   = "        ";
    const size_t      kIndentLen  = 8u;
    const size_t      max_off     = (g_profiler_log_bufsize > 0u)
                                    ? g_profiler_log_bufsize - 1u : 0u;

    const size_t len = strlen(s);
    size_t       i   = 0u;
    while (i < len && g_profiler_log_offset < max_off)
    {
        /* Indent at the start of every line except the very first. */
        if (g_profiler_log_indent_pending && g_profiler_log_line > 0)
        {
            if (g_profiler_log_offset + kIndentLen >= max_off) { break; }
            memcpy(g_profiler_log_buf + g_profiler_log_offset, kIndent, kIndentLen);
            g_profiler_log_offset += kIndentLen;
            g_profiler_log_indent_pending = false;
        }
        char c = s[i++];
        g_profiler_log_buf[g_profiler_log_offset++] = c;
        if (c == '\n')
        {
            g_profiler_log_line++;
            g_profiler_log_indent_pending = true;
        }
    }
    g_profiler_log_buf[g_profiler_log_offset] = '\0';
}

extern "C" void tf_profiler_event_upload(const unsigned char *model_buf,
                                         const void          *input,
                                         unsigned int         input_len)
{
    if (!model_buf || !input || input_len == 0u) { return; }
    const float *float_input = static_cast<const float *>(input);

    /* Determine output size (library always emits float32 output). */
    ts_realtek_model_info_t info;
    if (ts_realtek_get_model_info(model_buf, 0u, &info) != TS_REALTEK_OK)
    {
        return;
    }
    const uint32_t num_classes = (info.output_elem_count > 0u) ? info.output_elem_count : 1u;
    const uint32_t out_buf_bytes = num_classes * (uint32_t)sizeof(float);

    /* Allocate the capture buffer for the profiler text and route the
     * library's log output through profiler_capture_cb for the duration
     * of the call. */
    g_profiler_log_bufsize        = TINYML_EDGE_PROFILER_BUF_BYTES;
    g_profiler_log_buf            = (char *)ts_malloc(g_profiler_log_bufsize);
    if (!g_profiler_log_buf)
    {
        g_profiler_log_bufsize = 0u;
        return;
    }
    g_profiler_log_offset         = 0u;
    g_profiler_log_line           = 0;
    g_profiler_log_indent_pending = false; /* first line is the title    */

    /* Output buffer for the actual inference result (discarded - we only
     * care about the timing tables here). */
    void *output_buf = ts_malloc(out_buf_bytes);
    if (!output_buf)
    {
        ts_free(g_profiler_log_buf);
        g_profiler_log_buf = NULL;
        return;
    }

    /* Wire heap + model under the normal log so errors from the probe
     * phase don't leak into the captured profiler text. */
    ts_realtek_register_heap(ts_malloc, ts_free);
    ts_realtek_register_log(ts_log);
    ts_realtek_register_model(model_buf, /* size unused */ 0u);

    /* Phase 1 - PROBE: find the largest arena that ts_malloc can satisfy
     * right now.  Silence the log callback so "arena alloc failed" from
     * failed attempts does not appear anywhere; restore after probing. */
    ts_realtek_register_log(NULL);  /* suppress probe noise */
    static const uint32_t k_arena_candidates[] =
    {
        TINYML_EDGE_ARENA_BYTES,
        192u * 1024u,
        128u * 1024u,
        96u * 1024u,
        64u * 1024u,
        32u * 1024,
    };
    uint32_t winning_arena = 0u;
    for (size_t a = 0; a < sizeof(k_arena_candidates) / sizeof(k_arena_candidates[0]); ++a)
    {
        ts_realtek_set_arena_size(k_arena_candidates[a]);
        int probe_rc = ts_realtek_init();
        if (probe_rc == TS_REALTEK_OK)
        {
            winning_arena = k_arena_candidates[a];
            break;  /* arena stays allocated - keeps the interpreter alive
                     * for invoke_with_profiler below.  Deinit on failure
                     * only, because re-init after deinit may fail depending
                     * on what log callback changes happen in between. */
        }
        ts_realtek_deinit();
        if (probe_rc != TS_REALTEK_ERR_MALLOC && probe_rc != TS_REALTEK_ERR_ALLOC_FAIL)
        {
            break;
        }
    }
    ts_realtek_register_log(ts_log);   /* restore log after probe */
    if (winning_arena != 0u)
    {
        DBG_DIRECT("profiler probe: arena=%d OK", (int)winning_arena);
    }
    if (winning_arena == 0u)
    {
        const char *err = "[ERR] profiler: no arena size succeeded\n";
        tf_profiler_buffer_add(err);
        ts_free(g_profiler_log_buf);
        g_profiler_log_buf     = NULL;
        g_profiler_log_bufsize = 0u;
        ts_free(output_buf);
        return;
    }

    /* Phase 2 - CAPTURE: switch to capture callback, then invoke once
     * with the known-good arena size.  The interpreter is already up
     * (kept from the probe above), so we skip ts_realtek_init(). */
    ts_realtek_register_log(profiler_capture_cb);

    /* Header line written after callback is installed. */
    {
        const char *title = "[Operator Total Time + Detailed Execution]\n";
        profiler_capture_cb(title);
    }

    uint32_t actual_out_len = 0u;
    int rc = ts_realtek_invoke_with_profiler(float_input, input_len,
                                             output_buf, out_buf_bytes,
                                             &actual_out_len);
    if (rc != TS_REALTEK_OK)
    {
        char err_msg[64];
        snprintf(err_msg, sizeof(err_msg),
                 "[ERR] ts_realtek_invoke_with_profiler: %d\n", rc);
        profiler_capture_cb(err_msg);
    }
    ts_realtek_deinit(); /* free the arena allocated by the winning probe */

    /* Restore the normal log shim for any subsequent ts_realtek call. */
    ts_realtek_register_log(ts_log);

    /* Flush captured text to host. */
    g_profiler_log_buf[g_profiler_log_offset] = '\0';
    tf_profiler_buffer_add(g_profiler_log_buf);

    ts_free(g_profiler_log_buf);
    g_profiler_log_buf     = NULL;
    g_profiler_log_bufsize = 0u;
    g_profiler_log_offset  = 0u;
    g_profiler_log_line    = 0;

    ts_free(output_buf);
}

/******************* (C) COPYRIGHT 2018 Realtek Semiconductor Corporation *****END OF FILE****/
