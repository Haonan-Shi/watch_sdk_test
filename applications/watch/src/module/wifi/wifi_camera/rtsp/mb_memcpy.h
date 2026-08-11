/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef MB_MEMCPY_H
#define MB_MEMCPY_H

#include <stddef.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// Global custom memcpy function pointer
// Set via mb_init_ex() to use ARM ABI optimized memcpy
typedef void *(*mb_memcpy_fn_t)(void *dest, const void *src, size_t n);
extern mb_memcpy_fn_t g_mb_memcpy_fn;

// Wrapper function that uses custom memcpy if set, otherwise falls back to standard memcpy
static inline void *mb_memcpy_impl(void *dest, const void *src, size_t n)
{
    if (g_mb_memcpy_fn)
    {
        return g_mb_memcpy_fn(dest, src, n);
    }
    return memcpy(dest, src, n);
}

// Macro to replace memcpy in performance-critical paths
#define MB_MEMCPY(dest, src, n) mb_memcpy_impl(dest, src, n)

#ifdef __cplusplus
}
#endif

#endif // MB_MEMCPY_H
