/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*
 * ts_mem.c - unified ts_realtek heap adapter for the combined tinyml sample.
 *
 * Following the vendor SDK, these routines wrap a dedicated memory-management
 * unit (psRam_heap) instead of newlib.  On Zephyr newlib's malloc is wrapped
 * by the RTK HAL onto the small (~100KB) on-chip os_mem DATA_ON heap, which
 * cannot hold the tensor arena (+ MicroProfiler / feature buffers) of the
 * larger models.  psRam_heap manages a large heap in external PSRAM (see
 * psRam_heap.c), so routing ts_malloc/ts_free here places all inference
 * allocations in PSRAM.
 *
 * All four applications (bench / ic / kws / motion) share this adapter; the
 * PSRAM controllers must be brought up first via tinyml_psram_init() (called
 * from each app's main() before any allocation).
 *
 * This file also provides platform_random(), the single non-libc symbol the
 * prebuilt inference library expects from the platform.
 */

#include <stdint.h>
#include <string.h>
#include <zephyr/kernel.h>

#include "ts_mem.h"
#include "psRam_heap.h"

void *ts_malloc(uint32_t size)
{
    return psRamPortMalloc((size_t)size);
}

void *ts_calloc(uint32_t nblock, uint32_t size)
{
    return psRamPortCalloc((size_t)nblock, (size_t)size);
}

void *ts_realloc(void *mem, size_t size)
{
    /* The heap-4 allocator has no native realloc.  Emulate the libc contract:
     *   mem == NULL      -> behave like malloc(size)
     *   size == 0        -> behave like free(mem), return NULL
     *   otherwise        -> allocate, copy, free the old block.
     * The old block size is not tracked, so copy the requested size; callers
     * in the ts_realtek engine only ever grow-or-keep buffers they own. */
    void *new_ptr;

    if (mem == NULL)
    {
        return psRamPortMalloc(size);
    }

    if (size == 0U)
    {
        psRamFree(mem);
        return NULL;
    }

    new_ptr = psRamPortMalloc(size);
    if (new_ptr != NULL)
    {
        memcpy(new_ptr, mem, size);
        psRamFree(mem);
    }

    return new_ptr;
}

void ts_free(void *pt)
{
    if (pt != NULL)
    {
        psRamFree(pt);
    }
}

/*
 * platform_random - referenced by an unused fill path inside the prebuilt
 * ts_realtek library. Implemented with a small xorshift PRNG seeded from the
 * cycle counter so the sample has no dependency on the entropy driver.
 * Returns a value in [0, max) (or 0 when max == 0), matching the vendor ROM ABI.
 */
uint32_t platform_random(uint32_t max)
{
    static uint32_t state;

    if (state == 0U)
    {
        state = k_cycle_get_32() | 1U;
    }

    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;

    return (max == 0U) ? 0U : (state % max);
}
