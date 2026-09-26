/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*
 * ts_realtek.h  -  Public C interface for the ts_realtek TFLite-Micro inference library.
 *
 * Compiled with armclang (ARM Compiler 6), callable from ARMCC 5 projects.
 * All types are plain C; no C++ objects cross the boundary.
 *
 * Usage:
 *   1. ts_realtek_register_heap(my_malloc, my_free);
 *   2. ts_realtek_register_log(my_log_cb);            // optional
 *   3. ts_realtek_register_model(model_buf, model_size);
 *   4. ts_realtek_init();
 *   5. ts_realtek_invoke(input, input_len, output, output_len, &actual_len);
 *   6. ts_realtek_deinit();
 */

#ifndef TS_REALTEK_H
#define TS_REALTEK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* -------------------------------------------------------------------------
 * Return codes
 * ---------------------------------------------------------------------- */
typedef enum ts_realtek_err
{
    TS_REALTEK_OK              =  0,
    TS_REALTEK_ERR_NULL_ARG    = -1,  /* required argument is NULL             */
    TS_REALTEK_ERR_NO_HEAP     = -2,  /* heap not registered before init       */
    TS_REALTEK_ERR_NO_MODEL    = -3,  /* model not registered before init      */
    TS_REALTEK_ERR_BAD_MODEL   = -4,  /* tflite::GetModel returned null/invalid*/
    TS_REALTEK_ERR_ALLOC_FAIL  = -5,  /* AllocateTensors failed                */
    TS_REALTEK_ERR_INVOKE_FAIL = -6,  /* Invoke() returned error               */
    TS_REALTEK_ERR_NOT_INIT    = -7,  /* invoke called before init             */
    TS_REALTEK_ERR_BUF_SMALL   = -8,  /* output buffer too small               */
    TS_REALTEK_ERR_MALLOC      = -9,  /* internal malloc failed                */
    TS_REALTEK_ERR_STATE       = -10  /* unexpected internal state             */
} ts_realtek_err_t;

/* -------------------------------------------------------------------------
 * Callback types
 * All function pointers use the default ARM AAPCS C calling convention.
 * ---------------------------------------------------------------------- */

/** Log callback - called by the library for debug/info messages.
 *  msg is a null-terminated ASCII string.  Must not be NULL after register. */
typedef void (*ts_realtek_log_cb_t)(const char *msg);

/** Heap allocate callback - must return a pointer aligned to at least 8 bytes,
 *  or NULL on failure.  size is in bytes. */
typedef void *(*ts_realtek_malloc_cb_t)(uint32_t size);

/** Heap free callback. */
typedef void (*ts_realtek_free_cb_t)(void *ptr);

/* -------------------------------------------------------------------------
 * API
 * ---------------------------------------------------------------------- */

/**
 * Register a heap implementation.
 * MUST be called before ts_realtek_init().
 * @return TS_REALTEK_OK or TS_REALTEK_ERR_NULL_ARG
 */
int ts_realtek_register_heap(ts_realtek_malloc_cb_t malloc_cb,
                             ts_realtek_free_cb_t   free_cb);

/**
 * Register a log callback (optional).
 * If not registered, log output is silently discarded.
 */
int ts_realtek_register_log(ts_realtek_log_cb_t log_cb);

/**
 * Register the TFLite-Micro model flat-buffer.
 * buf must remain valid until ts_realtek_deinit() is called.
 * MUST be called before ts_realtek_init().
 * @return TS_REALTEK_OK or TS_REALTEK_ERR_NULL_ARG
 */
int ts_realtek_register_model(const unsigned char *buf, uint32_t size);

/**
 * Override the tensor arena size (in bytes) used by ts_realtek_init().
 * Must be called BEFORE ts_realtek_init(). If not called, the library
 * default (TS_REALTEK_ARENA_SIZE, 150 KB) is used.
 *
 * Guideline by model type:
 *   KWS / ADT small models  : 150 KB (default)
 *   IC MobileNet 96x96 f32  : >= 400 KB
 *
 * @return TS_REALTEK_OK or TS_REALTEK_ERR_NULL_ARG (if bytes == 0)
 */
int ts_realtek_set_arena_size(uint32_t bytes);

/**
 * Initialise the inference engine.
 * Calls tflite::GetModel, builds the op resolver and allocates tensors.
 * @return TS_REALTEK_OK on success, negative error code on failure.
 */
int ts_realtek_init(void);

/**
 * Destroy the inference engine and free internal resources.
 * After this call ts_realtek_init() may be called again.
 */
int ts_realtek_deinit(void);

/**
 * Run a single inference.
 *
 * @param input          Pointer to input data (float32 or int8 depending on model).
 * @param input_len      Size of input data in bytes.
 * @param output         Caller-supplied buffer for raw tensor bytes.
 * @param output_len     Size of caller's output buffer in bytes.
 * @param out_actual_len If non-NULL, receives the number of bytes written.
 *
 * The raw output tensor bytes are written verbatim (int8 or float32 per model).
 * The caller is responsible for interpreting the data according to the model's
 * output type and quantisation parameters.
 *
 * @return TS_REALTEK_OK on success, negative error code on failure.
 */
int ts_realtek_invoke(const void *input,
                      uint32_t    input_len,
                      void       *output,
                      uint32_t    output_len,
                      uint32_t   *out_actual_len);

/* -------------------------------------------------------------------------
 * Optional: query inference timing from the last call to ts_realtek_invoke().
 * Returns 0 if ts_realtek_invoke has not been called yet.
 * ---------------------------------------------------------------------- */
uint32_t ts_realtek_last_invoke_cycles(void);

/* -------------------------------------------------------------------------
 * Diagnostic / introspection APIs
 *
 * Added so that host applications (e.g. tinyml_host) no longer need to
 * include any tinyml/lite/ header to query model metadata, list
 * operators, or run a profiled inference.
 * ---------------------------------------------------------------------- */

/** Tensor element data type. Mirrors the subset of TfLiteType used by
 *  models supported by this library. */
typedef enum ts_realtek_dtype
{
    TS_REALTEK_DTYPE_UNKNOWN = 0,
    TS_REALTEK_DTYPE_FLOAT32 = 1,
    TS_REALTEK_DTYPE_INT8    = 2,
    TS_REALTEK_DTYPE_UINT8   = 3,
    TS_REALTEK_DTYPE_INT16   = 4,
    TS_REALTEK_DTYPE_INT32   = 5
} ts_realtek_dtype_t;

/** Quantisation parameters and shape info for a model's input/output tensor. */
typedef struct ts_realtek_model_info
{
    int32_t  input_type;          /* ts_realtek_dtype_t value                 */
    int32_t  output_type;         /* ts_realtek_dtype_t value                 */
    float    input_scale;         /* 0.0f if not quantised                    */
    int32_t  input_zero_point;    /* 0 if not quantised                       */
    float    output_scale;        /* 0.0f if not quantised                    */
    int32_t  output_zero_point;   /* 0 if not quantised                       */
    uint32_t input_elem_count;    /* total number of input  tensor elements   */
    uint32_t output_elem_count;   /* total number of output tensor elements   */
    uint32_t input_bytes;         /* raw input  tensor size in bytes          */
    uint32_t output_bytes;        /* raw output tensor size in bytes          */
} ts_realtek_model_info_t;

/**
 * Parse a TFLite flat-buffer and return input/output type, quantisation
 * parameters, element counts and byte sizes. Does NOT require
 * ts_realtek_init() to have been called - operates purely on the buffer.
 *
 * @return TS_REALTEK_OK or a negative error code.
 */
int ts_realtek_get_model_info(const unsigned char    *buf,
                              uint32_t                size,
                              ts_realtek_model_info_t *out);

/**
 * Parse a TFLite flat-buffer and write the comma-separated list of unique
 * builtin operator names used by the model into out_buf (NUL-terminated).
 * Does NOT require ts_realtek_init().
 *
 * @return TS_REALTEK_OK on success (even if buffer was truncated to fit),
 *         negative error code otherwise.
 */
int ts_realtek_get_unique_ops(const unsigned char *buf,
                              uint32_t             size,
                              char                *out_buf,
                              uint32_t             buf_size);

/**
 * Return the actual arena bytes used by the most recent successful
 * ts_realtek_init() (i.e. interpreter->arena_used_bytes()).
 * Requires ts_realtek_init() to have succeeded.
 *
 * @return TS_REALTEK_OK or TS_REALTEK_ERR_NOT_INIT.
 */
int ts_realtek_get_arena_used(uint32_t *bytes_out);

/**
 * Run a single inference with TFLite-Micro's MicroProfiler attached and
 * emit per-op timing tables (human-readable Log() and per-tag CSV) via
 * the currently registered log callback.
 *
 * Internally rebuilds the interpreter with a profiler attached, runs the
 * model once, prints results, then restores the normal interpreter so that
 * subsequent ts_realtek_invoke() calls continue to work.
 *
 * Requires ts_realtek_init() to have already succeeded.
 *
 * @return TS_REALTEK_OK on success, negative error code on failure.
 */
int ts_realtek_invoke_with_profiler(const void *input,
                                    uint32_t    input_len,
                                    void       *output,
                                    uint32_t    output_len,
                                    uint32_t   *out_actual_len);

#ifdef __cplusplus
}
#endif

#endif /* TS_REALTEK_H */