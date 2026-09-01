/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*
 * Demo 4: psram cache configuration - cacheable vs non-cacheable.
 *
 * app_system_lower_init() -> app_mpu_config() (app_lower_init.c) splits each
 * psram chip's first 2 MB into a 1 MB cacheable region (PSRAM0_MCU, MPU attr
 * 0xAA) and a 1 MB non-cacheable region (PSRAM0_NC, MPU attr 0x44). On this SoC
 * both map to the SAME address space, so cacheability is decided entirely by
 * which MPU region an address falls in.
 *
 * The two buffers below live in those two regions (same psram chip / SPIC1, so
 * the only difference is cacheability). We fill both, verify the data is
 * correct, then time repeated sequential reads of each. The cacheable buffer
 * should be markedly faster on the warm passes because it is served from the
 * cpu cache, while every non-cacheable access goes out to psram.
 *
 * Non-cacheable memory is what you want for buffers shared with another bus
 * master (DMA, DSP, ...) where you must not deal with stale cache lines; the
 * trade-off is exactly the read latency shown here.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <zephyr/kernel.h>

#include "psram_section.h"
#include "psram_demo.h"

#define CACHE_BUF_WORDS  2048            /* 8 KB, fits in L1 D-cache */
#define CACHE_PASSES     64

static uint32_t cacheable_buf[CACHE_BUF_WORDS]     SECTION_PSRAM0_MCU;
static uint32_t noncacheable_buf[CACHE_BUF_WORDS]  SECTION_PSRAM0_NC;

/* sum-read the buffer CACHE_PASSES times; returns elapsed microseconds and the
 * accumulated sum (via out_sum) so the loop can't be optimised away. */
static uint32_t timed_read(volatile uint32_t *buf, uint64_t *out_sum)
{
    uint64_t sum = 0;
    uint32_t c0 = k_cycle_get_32();

    for (int pass = 0; pass < CACHE_PASSES; pass++)
    {
        for (int i = 0; i < CACHE_BUF_WORDS; i++)
        {
            sum += buf[i];
        }
    }

    uint32_t c1 = k_cycle_get_32();

    *out_sum = sum;
    return k_cyc_to_us_floor32(c1 - c0);
}

int psram_cache_demo(void)
{
    int fail = 0;

    printf("\n[4] ---- PSRAM cache demo (cacheable vs non-cacheable) ----\n");
    printf("    cacheable_buf    @ %p (PSRAM0_MCU, MPU 0xAA)\n", (void *)cacheable_buf);
    printf("    noncacheable_buf @ %p (PSRAM0_NC,  MPU 0x44)\n", (void *)noncacheable_buf);

    /* NOLOAD regions -> fill at runtime */
    for (int i = 0; i < CACHE_BUF_WORDS; i++)
    {
        cacheable_buf[i] = (uint32_t)i;
        noncacheable_buf[i] = (uint32_t)i;
    }

    uint64_t expect = (uint64_t)CACHE_PASSES *
                      ((uint64_t)(CACHE_BUF_WORDS - 1) * CACHE_BUF_WORDS / 2);
    uint64_t sum_c = 0, sum_nc = 0;

    uint32_t us_c  = timed_read(cacheable_buf, &sum_c);
    uint32_t us_nc = timed_read(noncacheable_buf, &sum_nc);

    if (sum_c != expect || sum_nc != expect)
    {
        printf("    [FAIL] data mismatch: cacheable=%llu non-cacheable=%llu expect=%llu\n",
               (unsigned long long)sum_c, (unsigned long long)sum_nc,
               (unsigned long long)expect);
        fail++;
    }

    printf("    read %uKB x%d passes: cacheable=%u us, non-cacheable=%u us\n",
           (unsigned int)(sizeof(cacheable_buf) / 1024), CACHE_PASSES, us_c, us_nc);
    if (us_c > 0)
    {
        printf("    non-cacheable / cacheable = %u.%02ux slower\n",
               us_nc / us_c, (us_nc * 100 / us_c) % 100);
    }

    printf("    [%s] cache demo\n", fail ? "FAIL" : "PASS");
    return fail;
}
