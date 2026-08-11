/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* minimal_build_zephyr.h
 * C API wrapper for minimal_build RTP->JPEG reassembler for Zephyr/C cortex-m
 */
#ifndef MINIMAL_BUILD_ZEPHYR_H
#define MINIMAL_BUILD_ZEPHYR_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Error codes */
enum
{
    MB_OK = 0,
    MB_ERR_NOT_INIT = 1,
    MB_ERR_ARG = 2,
    MB_ERR_OOM = 3,
    MB_ERR_INTERNAL = 100
};

/* allocator interface (optional) */
typedef void *(*mb_malloc_fn)(size_t sz, void *user_ctx);
typedef void (*mb_free_fn)(void *ptr, void *user_ctx);

/* custom memcpy interface (optional, for ARM ABI optimized memcpy) */
typedef void *(*mb_memcpy_fn)(void *dest, const void *src, size_t n);

/* frame callback
   - data/len: JPEG frame buffer and size
   - is_valid: 1 = frame passed validation (complete/repairable); 0 = has issues (app may discard)
   - user_ctx: context provided at registration time

   Memory ownership:
   - Copy mode: data is allocated by the library; caller must free via mb_free_frame_buffer()
   - Zero-copy mode (mb_init_ex with zero_copy=true):
       * data points to an internal buffer and is ONLY valid during the callback
       * DO NOT call mb_free_frame_buffer() in zero-copy mode
       * If you need data after callback, copy it yourself
*/
typedef void (*mb_frame_cb_t)(const uint8_t *data, uint32_t len, int is_valid, void *user_ctx);

/* initialize the engine. max_frame_buf limits the maximum per-frame allocation.
   If malloc_fn/free_fn are NULL, libc malloc/free are used.

   Simple version: uses default copy mode */
int mb_init(size_t max_frame_buf,
            mb_malloc_fn malloc_fn,
            mb_free_fn free_fn,
            void *user_alloc_ctx);

/* Extended initialization with zero-copy and custom memcpy support.

   Parameters:
   - max_frame_buf: Maximum frame buffer size (0 = use default)
   - malloc_fn/free_fn: Custom allocators (NULL = use libc malloc/free)
   - user_alloc_ctx: Context passed to malloc_fn/free_fn
   - zero_copy: Enable zero-copy mode (true = no allocation/copy in callback)
   - memcpy_fn: Custom memcpy function (NULL = use standard memcpy)

   Zero-copy mode behavior:
   - Callback receives pointer to internal live555 buffer
   - Data is ONLY valid during callback execution
   - DO NOT call mb_free_frame_buffer() in zero-copy mode
   - Saves ~1.5ms per frame (no malloc + no memcpy)

   Custom memcpy:
   - Used for all internal memory copies
   - Can use ARM ABI optimized memcpy for better performance
   - Useful even in zero-copy mode (for internal live555 copies)
*/
int mb_init_ex(size_t max_frame_buf,
               mb_malloc_fn malloc_fn,
               mb_free_fn free_fn,
               void *user_alloc_ctx,
               int zero_copy,
               mb_memcpy_fn memcpy_fn);

void mb_deinit();

int mb_set_frame_callback(mb_frame_cb_t cb, void *user_ctx);

/* Process a received RTP raw packet. Caller retains ownership of data and may free it after return. */
int mb_process_packet(const uint8_t *data, uint32_t len);

/* Free a frame buffer received in the frame callback. */
void mb_free_frame_buffer(const uint8_t *data);

#ifdef __cplusplus
}
#endif

#endif // MINIMAL_BUILD_ZEPHYR_H
