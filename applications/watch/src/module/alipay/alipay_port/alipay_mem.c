/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "string.h"
#include "board.h"
#include "os_timer.h"
#include "os_sched.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rtc.h"
#include "app_msg.h"
#include "rtl876x_wdg.h"
#include "os_sync.h"
//#include "gui_core.h"
#include "hub_task.h"
#include "platform_utils.h"
#include "vector_table.h"
#include "rtl876x_i2c.h"
#include "app_dlps.h"
#include "os_mem.h"
#include "app_main.h"
#include "profile_server_def.h"
#include "profile_server.h"
#include "section.h"
//#include "iotsec.h"
#include "tlsf.h"
#include "alipay_config.h"
#include <zephyr/devicetree.h>
#include "psram_section.h"
#if  CONFIG_ALIPAY

/**
* use os tlsf
*/

static tlsf_t alipay_tlsf = NULL;

/* alipay TLSF heap lives in the non-cacheable psram region (PSRAM1_NC): it is
 * shared across threads (shell / alipay_task / SDK internal) and must be
 * non-cacheable so reads always hit PSRAM and writes are immediately visible.
 * Defined as a variable in the PSRAM1_NC linker section instead of referencing
 * the raw dts address. The section is NOLOAD (not zeroed/copied at boot);
 * tlsf_create_with_pool() below initialises the pool at runtime, which only
 * runs after psram is powered up. */
#define ALIPAY_HEAP_SIZE   (200 * 1024)
static uint8_t alipay_heap_pool[ALIPAY_HEAP_SIZE] SECTION_PSRAM1_NC;

void csi_heap_init(void)
{
    alipay_tlsf = tlsf_create_with_pool((void *)alipay_heap_pool,
                                        sizeof(alipay_heap_pool));
    if (alipay_tlsf)
    {
        AliPay_LOG("[Alipay] tlsf heap init success!");
    }
    else
    {
        AliPay_LOG("[Alipay] tlsf heap init failed!");
    }
}

void *csi_realloc(void *mem, size_t size)
{
    return tlsf_realloc(alipay_tlsf, mem, size);
}

size_t csi_getUsedSize(void *pt)
{
    return tlsf_block_size(pt);
}

void *csi_malloc(uint32_t size)
{
    return tlsf_malloc(alipay_tlsf, size);
}

void *csi_calloc(uint32_t nblock, uint32_t size)
{
    void *pt = tlsf_malloc(alipay_tlsf, nblock * size);
    if (pt)
    {
        memset(pt, 0, nblock * size);
    }
    return pt;
}

void csi_free(void *pt)
{
    tlsf_free(alipay_tlsf, pt);
}

#if CONFIG_ALIPAY_TEST
/* alipay TLSF heap stats, for the `alipay mem` shell test command only.
 * Gated by CONFIG_ALIPAY_TEST so release builds (CONFIG_ALIPAY_TEST=n)
 * keep alipay_mem.c pure business code. alipay_tlsf is file-static here,
 * so the walker has to live in this file rather than alipay_test/. */

/* Walker callback for tlsf_walk_pool: classify each block as used or
 * free, accumulate totals and track the largest free chunk for
 * fragmentation feedback. */
typedef struct
{
    size_t used;
    size_t free;
    size_t largest_free;
} csi_heap_walk_ctx_t;

static void csi_heap_walker(void *ptr, size_t size, int used, void *user)
{
    (void)ptr;
    csi_heap_walk_ctx_t *ctx = (csi_heap_walk_ctx_t *)user;
    if (used)
    {
        ctx->used += size;
    }
    else
    {
        ctx->free += size;
        if (size > ctx->largest_free)
        {
            ctx->largest_free = size;
        }
    }
}

void csi_heap_stats_print(void)
{
    if (!alipay_tlsf)
    {
        AliPay_LOG("[mem] alipay_tlsf not initialized");
        return;
    }
    csi_heap_walk_ctx_t ctx = {0};
    tlsf_walk_pool(tlsf_get_pool(alipay_tlsf), csi_heap_walker, &ctx);
    AliPay_LOG("[mem] alipay_tlsf  total=%u  used=%u  free=%u  largest_free=%u",
               (unsigned)(ctx.used + ctx.free), (unsigned)ctx.used,
               (unsigned)ctx.free, (unsigned)ctx.largest_free);
}
#endif /* CONFIG_ALIPAY_TEST */

//void initial_module_transport()
//{
//}
#else



#endif // CONFIG_ALIPAY
