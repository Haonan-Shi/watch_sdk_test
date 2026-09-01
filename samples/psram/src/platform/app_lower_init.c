/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <string.h>
#include <stdio.h>
#include <trace.h>
#include "dma_channel.h"
#include "fmc_api_ext.h"
#include <fmc_api.h>
#include "platform_utils.h"
#include "mpu.h"
#include <cmsis_core.h>

extern uint32_t sys_timestamp_get_us(void);
#define ZEPHYR_DMA_CHANNEL_MASK     (BIT10 | BIT11 | BIT12 | BIT13 | BIT14 | BIT15)

/* MPU memory attributes (see mpu.h):
 *   0x44 = Normal, Non-cacheable
 *   0xAA = Normal, Cacheable Write-Through
 *   0xFF = Normal, Cacheable Write-Back, Write-Allocate
 */
#define MPU_ATTR_NON_CACHEABLE  0x44
#define MPU_ATTR_CACHEABLE_WT   0xAA
#define MPU_ATTR_CACHEABLE_WB   0xFF

/* RBAR flag bits used by every region here: SH:0 (non-shareable), AP:1
 * (read/write, any privilege level), XN:0 (execution allowed). */
#define MPU_RBAR_FLAGS          (1 << 1)

/* Configure one MPU region straight from a dts memory-region node. limit is the
 * inclusive last byte (base + size - 1); the hardware forces limit[4:0]=0x1F. */
#define PSRAM_MPU_SET(node, region_num, attribute)                            \
    mpu_set_region(DT_REG_ADDR(DT_NODELABEL(node)) | MPU_RBAR_FLAGS,          \
                   DT_REG_ADDR(DT_NODELABEL(node))                            \
                   + DT_REG_SIZE(DT_NODELABEL(node)) - 1,                 \
                   (region_num), (attribute), true)

/* Resolve an ARMv8-M MAIR attribute byte to a short label. */
static const char *mpu_attr_name(uint8_t attr)
{
    switch (attr)
    {
    case MPU_ATTR_NON_CACHEABLE:
        return "non-cacheable";
    case MPU_ATTR_CACHEABLE_WT:
        return "cacheable-WT";
    case 0xFF:
        return "cacheable-WB-WA";
    default:
        return "other";
    }
}

/* Dump every MPU region straight from the hardware registers via printf, so the
 * before/after comparison shows up on the console (uart2). The SDK
 * mpu_config_print() prints the same info but only over the dedicated log-uart,
 * which the console capture does not see. Reading MPU->RBAR/RLAR/MAIR is the
 * ground truth regardless of how the regions were programmed. */
/* Only the low regions matter here: 0/1 are boot-owned, 2..5 are the psram
 * regions this sample programs. Dumping all 12 early in boot floods the console
 * (printf poll-out contends with the shell backend on uart2 and drops chars), so
 * keep it short and flush + pace each line so the before/after view is intact. */
#define MPU_DUMP_REGIONS 12

static void dump_mpu_regions(const char *tag)
{
    uint32_t nregions = (MPU->TYPE & MPU_TYPE_DREGION_Msk) >> MPU_TYPE_DREGION_Pos;
    uint32_t last = (nregions < MPU_DUMP_REGIONS) ? nregions : MPU_DUMP_REGIONS;

    printf("---- MPU regions R0..R%u (%s): %u total, MPU %s ----\n",
           (unsigned int)(last - 1), tag, (unsigned int)nregions,
           (MPU->CTRL & MPU_CTRL_ENABLE_Msk) ? "ENABLED" : "disabled");
    fflush(stdout);

    for (uint32_t i = 0; i < last; i++)
    {
        MPU->RNR = i;
        uint32_t rbar = MPU->RBAR;
        uint32_t rlar = MPU->RLAR;

        if ((rlar & MPU_RLAR_EN_Msk) == 0U)
        {
            printf("    R%-2u: disabled (free)\n", (unsigned int)i);
        }
        else
        {
            uint32_t base  = rbar & MPU_RBAR_BASE_Msk;
            uint32_t limit = (rlar & MPU_RLAR_LIMIT_Msk) | 0x1FU;
            uint32_t idx   = (rlar & MPU_RLAR_AttrIndx_Msk) >> MPU_RLAR_AttrIndx_Pos;
            uint32_t mair  = (idx < 4U) ? MPU->MAIR0 : MPU->MAIR1;
            uint8_t  attr  = (mair >> ((idx & 3U) * 8U)) & 0xFFU;

            printf("    R%-2u: 0x%08x-0x%08x attr_idx=%u attr=0x%02x (%s)\n",
                   (unsigned int)i, (unsigned int)base, (unsigned int)limit,
                   (unsigned int)idx, attr, mpu_attr_name(attr));
        }
        fflush(stdout);
        k_msleep(5);    /* pace output so it isn't dropped by tx contention */
    }
}

static int app_mpu_config(void)
{
    /* ARMv8-M MPU regions must NOT overlap (an access hitting overlapping
     * regions faults), so each psram chip gets two adjacent, non-overlapping
     * regions: 1 MB cacheable (*_for_mcu) + 1 MB non-cacheable (*_nc). The heap
     * area beyond them is left to the default (cacheable) background map.
     *
     * psram0 is on SPIC1 (0x22000000), psram1 is on SPIC3 (0x24000000).
     *
     * MPU region-number map on this SoC (confirmed by dump_mpu_regions() below):
     *   R0  = boot: peripherals (0x40000000)
     *   R1  = boot: sram        (0x20020000)
     *   R2  = boot: external-mem NC covering psram0+psram1 (0x22000000-0x33ffffff)
     *   R3  = boot: SPIC2       (0x60000000)  <-- DO NOT reuse, it is live
     *   R4..R11 = free
     * We overwrite R2 to re-attribute psram0 as cacheable (the boot region is
     * exactly what we are taking over) and use free regions R4/R5/R6 for the
     * rest. R3 is left untouched so SPIC2 keeps its boot attributes. */

    /* dump the pre-existing (boot) regions so the free/occupied slots are
     * visible on the console before we program anything. */
    dump_mpu_regions("before psram cfg");
    mpu_config_print();     /* same info over the log-uart */

    /* psram0: cacheable (R2, overwrites boot ext-mem region) + non-cacheable (R6) */
    PSRAM_MPU_SET(psram0_for_mcu, 2, MPU_ATTR_CACHEABLE_WT);
    PSRAM_MPU_SET(psram0_nc,      6, MPU_ATTR_NON_CACHEABLE);

    /* psram1: cacheable (R4) + non-cacheable (R5) */
    PSRAM_MPU_SET(psram1_for_mcu, 4, MPU_ATTR_CACHEABLE_WT);
    PSRAM_MPU_SET(psram1_nc,      5, MPU_ATTR_NON_CACHEABLE);

    dump_mpu_regions("after psram cfg");
    mpu_config_print();

    return 0;
}
static void app_io_resource_cfg(void)
{
    /*the set bits dma channel has been alloced to zephyr and dsp*/
    uint16_t dma_channel_masked = GDMA_channel_get_active_mask();
    dma_channel_cfg(dma_channel_masked | ZEPHYR_DMA_CHANNEL_MASK);
    /*the set bits hw timer can be created by hw_timer_create*/
    extern void hw_timer_channel_cfg(uint16_t hw_timer_mask);
    hw_timer_channel_cfg(BIT2 | BIT3);
}

void app_system_lower_init(void)
{
    /* psram SPIC clock switched to 280 MHz below via fmc_psram_clock_switch()
     * (fmc_api_ext.h): psram0=SPIC1, psram1=SPIC3, same rate the watch clk_mgr
     * high-freq table uses. Called after each psram is initialised. */
    uint32_t actual_mhz;

    app_mpu_config();

#if DT_NODE_HAS_STATUS(DT_NODELABEL(psram0), okay)
    if (fmc_psram_winbond_opi_init(FMC_SPIC_ID_1))
    {
        APP_PRINT_TRACE0("WB OPI psram0 init success!");
    }
    else
    {
        APP_PRINT_TRACE0("WB OPI psram0 init fail!");
    }
    fmc_set_spic_slpck_enable(FMC_SPIC_ID_1, true);
    actual_mhz = 0;
    fmc_psram_clock_switch(FMC_SPIC_ID_1, 280, &actual_mhz);
    printk("psram0/SPIC1 clock -> %u MHz\n", (unsigned int)actual_mhz);
#endif

#if DT_NODE_HAS_STATUS(DT_NODELABEL(psram1), okay)
    if (fmc_psram_winbond_opi_init(FMC_SPIC_ID_3))
    {
        APP_PRINT_TRACE0("WB OPI psram1 init success!");
    }
    else
    {
        APP_PRINT_TRACE0("WB OPI psram1 init fail!");
    }
    fmc_set_spic_slpck_enable(FMC_SPIC_ID_3, true);
    actual_mhz = 0;
    fmc_psram_clock_switch(FMC_SPIC_ID_3, 280, &actual_mhz);
    printk("psram1/SPIC3 clock -> %u MHz\n", (unsigned int)actual_mhz);
#endif

    app_io_resource_cfg();

    return;
}
