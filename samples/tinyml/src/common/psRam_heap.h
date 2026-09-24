/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*
 * psRam_heap - self-contained heap-4 style memory manager backed by external
 * PSRAM.  Ported from the vendor SDK tinyml_edge mem_module.
 *
 * Rationale:
 *   The ts_realtek inference engine allocates BOTH the tensor arena and the
 *   ~80KB MicroProfiler (kMaxEvents=4096) through the caller-supplied malloc
 *   pair registered with ts_realtek_register_heap().  Together with the model
 *   / input-data buffers streamed over UART this exceeds the ~100KB on-chip
 *   DATA_ON heap, so allocations must come from PSRAM instead.
 *
 *   Rather than route ts_malloc() to newlib (which the RTK HAL wraps onto the
 *   small on-chip os_mem DATA_ON heap), this module manages its own heap over
 *   a large ucHeap[] buffer placed in the PSRAM1_MCU memory-region (see the
 *   board overlay).  ts_malloc()/ts_free() (ts_mem.c) route here, so the arena
 *   and profiler both land in PSRAM.
 *
 *   configTOTAL_psRAM_HEAP_SIZE is also reported to the host tool via the
 *   GET_MEM_SIZE (0x41) command in uart_packet_parser.c.
 */

#ifndef TINYML_EDGE_PSRAM_HEAP_H
#define TINYML_EDGE_PSRAM_HEAP_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Total managed PSRAM heap (bytes).  Must fit inside the PSRAM1_MCU region
 * (2560 KB) declared in the board overlay. */
#define configTOTAL_psRAM_HEAP_SIZE   (300u * 1024u)

/* heap-4 allocator API. */
void   *psRamPortMalloc(size_t xWantedSize);
void    psRamFree(void *pv);
void   *psRamPortCalloc(size_t xNum, size_t xSize);
size_t  psRamGetFreeHeapSize(void);
size_t  psRamGetMinimumEverFreeHeapSize(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* TINYML_EDGE_PSRAM_HEAP_H */
