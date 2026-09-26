/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*
 * ts_mem.h - Heap adapter for the ts_realtek inference engine on Zephyr.
 *
 * The ts_realtek library allocates its tensor arena through a caller-supplied
 * malloc/free pair (see ts_realtek_register_heap). On Zephyr we simply route
 * those calls to the C library heap (newlib), which also backs the direct
 * malloc/calloc/free calls made by libkws_feature.
 */

#ifndef TS_MEM_H_
#define TS_MEM_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void *ts_malloc(uint32_t size);
void *ts_calloc(uint32_t nblock, uint32_t size);
void *ts_realloc(void *mem, size_t size);
void  ts_free(void *pt);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* TS_MEM_H_ */
