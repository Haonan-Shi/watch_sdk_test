/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*
 * Demo 2: psram used as a heap (both PSRAM0 and PSRAM1).
 *
 * psram_heap_init() (SoC osif_zephyr.c) registers heap_psram0 and
 * heap_psram1 into the SoC multi-heap under RAM_TYPE_PSRAM0 / RAM_TYPE_PSRAM1.
 * The T_OS_MEM_TYPE values OS_MEM_TYPE_PSRAM0 (0x07) / OS_MEM_TYPE_PSRAM1 (0x08)
 * map to those RAM_TYPEs, so os_mem_alloc(OS_MEM_TYPE_PSRAMx, ...) lands in the
 * corresponding psram heap.
 *
 * "psram static check" == inspecting the heap usage. os_mem_peek() covers the
 * psram heaps as well (RAM_TYPE_PSRAM0 / RAM_TYPE_PSRAM1 cases in
 * os_mem_peek_zephyr()), so psram_heap_peek() just asks it for the free bytes of
 * each heap. The heap total comes from the same devicetree node that
 * psram_heap_init() registered.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <zephyr/kernel.h>

#include "os_mem.h"
#include "psram_demo.h"

void psram_heap_peek(void)
{
#if DT_NODE_EXISTS(DT_NODELABEL(heap_psram0)) || DT_NODE_EXISTS(DT_NODELABEL(heap_psram1))
    static const struct
    {
        T_OS_MEM_TYPE type;
        const char *name;
        size_t heap_size;
    } tbl[] =
    {
#if DT_NODE_EXISTS(DT_NODELABEL(heap_psram0))
        { OS_MEM_TYPE_PSRAM0, "PSRAM0", DT_REG_SIZE(DT_NODELABEL(heap_psram0)) },
#endif
#if DT_NODE_EXISTS(DT_NODELABEL(heap_psram1))
        { OS_MEM_TYPE_PSRAM1, "PSRAM1", DT_REG_SIZE(DT_NODELABEL(heap_psram1)) },
#endif
    };

    for (int i = 0; i < ARRAY_SIZE(tbl); i++)
    {
        size_t free_bytes = os_mem_peek(tbl[i].type);

        printf("    %s heap: size=%u free=%u used=%u\n",
               tbl[i].name, (unsigned int)tbl[i].heap_size, (unsigned int)free_bytes,
               (unsigned int)(tbl[i].heap_size - free_bytes));
    }
#else
    printf("    (no heap_psram0 / heap_psram1 devicetree node - nothing to peek)\n");
#endif
}

static int exercise_heap(T_OS_MEM_TYPE type, const char *name)
{
    const size_t sz = 0x1000;
    int fail = 0;

    printf("    -- %s --\n", name);

    /* plain alloc: write / read back */
    uint8_t *p = os_mem_alloc(type, sz);

    if (p == NULL)
    {
        printf("    [FAIL] %s os_mem_alloc(0x%x)\n", name, (unsigned int)sz);
        return 1;
    }
    for (size_t i = 0; i < sz; i++)
    {
        p[i] = (uint8_t)(i & 0xFF);
    }
    for (size_t i = 0; i < sz; i++)
    {
        if (p[i] != (uint8_t)(i & 0xFF))
        {
            printf("    [FAIL] %s readback mismatch\n", name);
            fail++;
            break;
        }
    }

    /* zalloc: must come back zeroed */
    uint8_t *z = os_mem_zalloc(type, sz);

    if (z == NULL)
    {
        printf("    [FAIL] %s os_mem_zalloc\n", name);
        os_mem_free(p);
        return 1;
    }
    for (size_t i = 0; i < sz; i++)
    {
        if (z[i] != 0)
        {
            printf("    [FAIL] %s zalloc not zeroed at %u\n", name, (unsigned int)i);
            fail++;
            break;
        }
    }

    /* aligned alloc: pointer must honour the requested alignment */
    uint8_t *a = os_mem_aligned_alloc(type, sz, 32);

    if (a == NULL || ((uintptr_t)a & 31U) != 0U)
    {
        printf("    [FAIL] %s os_mem_aligned_alloc(32) -> %p\n", name, (void *)a);
        fail++;
    }

    printf("    %s: alloc=%p zalloc=%p aligned32=%p\n", name, (void *)p, (void *)z, (void *)a);

    os_mem_free(p);
    os_mem_free(z);
    if (a != NULL)
    {
        os_mem_aligned_free(a);
    }

    return fail;
}

int psram_heap_demo(void)
{
    int fail = 0;

    printf("\n[2] ---- PSRAM heap demo (PSRAM0 + PSRAM1) ----\n");
    printf("    before:\n");
    psram_heap_peek();

    fail += exercise_heap(OS_MEM_TYPE_PSRAM0, "PSRAM0");
    fail += exercise_heap(OS_MEM_TYPE_PSRAM1, "PSRAM1");

    printf("    after (all freed):\n");
    psram_heap_peek();

    printf("    [%s] heap demo\n", fail ? "FAIL" : "PASS");
    return fail;
}
