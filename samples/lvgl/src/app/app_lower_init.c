/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <os_sched.h>
#include <stdio.h>
#include <trace.h>
#include "fmc_api.h"
#include "os_mem.h"
#include "os_heap.h"
#include "os_msg.h"
#include "dma_channel.h"
#include "fmc_api_ext.h"
#include <rtl876x_rcc.h>
#include <fmc_api.h>
#include "platform_utils.h"
#include "rtl876x_pinmux.h"
#include "wdg.h"
#include "pm.h"
#include "board.h"
#include "system_status_api.h"
#include "os_task.h"
#include "cm55_setting.h"
#include "flash_map.h"
#include "mpu.h"
#include "address_map.h"

extern uint32_t sys_timestamp_get_us(void);


static int app_mpu_config(void)
{
    uint32_t mpu_cfg_rbar = SPIC1_MEM_BASE | (1 << 1); // SH:0, AP:1, XN:0
    uint32_t limit_addr = SPIC1_MEM_BASE + 0x400000; // PXN:0, EN:1
    uint8_t region_num = 2;
    uint8_t attribute = 0x44; // Non-Cacheable
    // uint8_t attribute = 0xFF; // Cacheable, write allocate, write back
    // uint8_t attribute = 0xAA; // Cacheable, write no-allocate, write through

    mpu_config_print();
    mpu_set_region(mpu_cfg_rbar, limit_addr, region_num, attribute, true);

    mpu_cfg_rbar = SPIC3_MEM_BASE | (1 << 1); // SH:0, AP:1, XN:0
    limit_addr = SPIC3_MEM_BASE + 0x400000; // PXN:0, EN:1
    region_num = 4;
    attribute = 0x44; // Non-Cacheable
    // attribute = 0xFF; // Cacheable, write allocate, write back
    // attribute = 0xAA; // Cacheable, write no-allocate, write through

    mpu_set_region(mpu_cfg_rbar, limit_addr, region_num, attribute, true);
    mpu_config_print();

    return 0;
}

static void Print_MPU_Config(void)
{
    //ARM_MPU_ClrRegion()
    //ARM_MPU_SetMemAttr(0x04, 0x55);
    //ARM_MPU_SetRegion();

    DBG_DIRECT("MPU Control: 0x%08X\n", MPU->CTRL);
    DBG_DIRECT("MAIR0: 0x%08X, MAIR1: 0x%08X\n", MPU->MAIR[0], MPU->MAIR[1]);

    for (uint32_t rnr = 0; rnr < 16; rnr++)
    {
        MPU->RNR = rnr;
        uint32_t rbar = MPU->RBAR;
        uint32_t rlar = MPU->RLAR;

        if (!(rlar & MPU_RLAR_EN_Msk))
        {
            continue;
        }

        uint32_t attrindx = (rlar & MPU_RLAR_AttrIndx_Msk) >> MPU_RLAR_AttrIndx_Pos;
        uint32_t mair_reg = (attrindx < 4) ? MPU->MAIR[0] : MPU->MAIR[1];
        uint8_t attr = (mair_reg >> ((attrindx % 4) * 8)) & 0xFF;

        DBG_DIRECT("Region %d:\n", rnr);
        DBG_DIRECT("  Base Address: 0x%08X\n", (rbar & MPU_RBAR_BASE_Msk));
        DBG_DIRECT("  Limit Address: 0x%08X\n", ((rlar & MPU_RLAR_LIMIT_Msk)) | 0x1F);
        DBG_DIRECT("  AttrIndx: %d\n", attrindx);
        DBG_DIRECT("  XN: %d, AP: %d, SH: %d\n",
                   (rbar & MPU_RBAR_XN_Msk) >> MPU_RBAR_XN_Pos,
                   (rbar & MPU_RBAR_AP_Msk) >> MPU_RBAR_AP_Pos,
                   (rbar & MPU_RBAR_SH_Msk) >> MPU_RBAR_SH_Pos);

        switch (attr)
        {
        case 0x04: DBG_DIRECT("Cache Policy: Non-cacheable\n"); break;
        case 0x44: DBG_DIRECT("Cache Policy: Non-cacheable\n"); break;
        case 0xAA: DBG_DIRECT("Cache Policy: Write-Back (no allocate)\n"); break;
        case 0x77:
        case 0xDD:
        case 0x55: DBG_DIRECT("Cache Policy: Write-Back (write allocate)\n"); break;
        default:   DBG_DIRECT("Cache Policy: Unknown (0x%02X)\n", attr & 0x0F);
        }
    }
}

static void spic0_nor_flash_speed_report(void)
{
#define SECTION_SIZE 1024
    static uint32_t volatile test_buffer[SECTION_SIZE];

    uint32_t color_buf = 0;

    uint32_t total_time = 0;
    uint32_t checksum = 0;
    for (uint32_t i = 0; i < 256; i++)
    {
        uint32_t old_stamp = sys_timestamp_get_us();
        memcpy((void *)test_buffer, (void *)(USER_DATA1_ADDR + i * SECTION_SIZE * 4),
               sizeof(uint32_t)*SECTION_SIZE);
        uint32_t new_stamp = sys_timestamp_get_us();
        total_time = total_time + new_stamp - old_stamp;
        for (uint32_t j = 0; j < 1024; j++)
        {
            checksum += test_buffer[j];
        }
    }
    DBG_DIRECT("[Checksum] %u", checksum);

    uint32_t time_ms = total_time / 1000;
    uint32_t time_us = total_time % 1000;

    DBG_DIRECT("[Nor Flash Read Speed Report] SystemCpuClock = %dM; Read 1M byte t=%d.%d ms",
               SystemCpuClock / 1000000, time_ms, time_us);
    DBG_DIRECT("[Nor Flash Read Speed Report] SystemCpuClock = %dM; %dMByte/S",
               SystemCpuClock / 1000000, 1000000 / total_time);

}

static void spic1_psram_speed_report(void)
{
#define SECTION_SIZE 1024
    static uint32_t volatile test_buffer[SECTION_SIZE];

    uint32_t color_buf = 0;

    uint32_t total_time = 0;
    uint32_t checksum = 0;
    for (uint32_t i = 0; i < 256; i++)
    {
        uint32_t old_stamp = sys_timestamp_get_us();
        memcpy((void *)test_buffer, (void *)(SPIC1_MEM_BASE + i * SECTION_SIZE * 4),
               sizeof(uint32_t)*SECTION_SIZE);
        uint32_t new_stamp = sys_timestamp_get_us();
        total_time = total_time + new_stamp - old_stamp;
        for (uint32_t j = 0; j < 1024; j++)
        {
            checksum += test_buffer[j];
        }
    }
    DBG_DIRECT("[Checksum] %u", checksum);

    uint32_t time_ms = total_time / 1000;
    uint32_t time_us = total_time % 1000;

    DBG_DIRECT("[SPIC1 PSRAM Read Speed Report] SystemCpuClock = %dM; Read 1M byte t=%d.%d ms",
               SystemCpuClock / 1000000, time_ms, time_us);
    DBG_DIRECT("[SPIC1 PSRAM Read Speed Report] SystemCpuClock = %dM; %dMByte/S",
               SystemCpuClock / 1000000, 1000000 / total_time);
}

static void spic3_psram_speed_report(void)
{
#define SECTION_SIZE 1024
    static uint32_t volatile test_buffer[SECTION_SIZE];

    uint32_t color_buf = 0;

    uint32_t total_time = 0;
    uint32_t checksum = 0;
    for (uint32_t i = 0; i < 256; i++)
    {
        uint32_t old_stamp = sys_timestamp_get_us();
        memcpy((void *)test_buffer, (void *)(SPIC3_MEM_BASE + i * SECTION_SIZE * 4),
               sizeof(uint32_t)*SECTION_SIZE);
        uint32_t new_stamp = sys_timestamp_get_us();
        total_time = total_time + new_stamp - old_stamp;
        for (uint32_t j = 0; j < 1024; j++)
        {
            checksum += test_buffer[j];
        }
    }
    DBG_DIRECT("[Checksum] %u", checksum);

    uint32_t time_ms = total_time / 1000;
    uint32_t time_us = total_time % 1000;

    DBG_DIRECT("[SPIC3 PSRAM Read Speed Report] SystemCpuClock = %dM; Read 1M byte t=%d.%d ms",
               SystemCpuClock / 1000000, time_ms, time_us);
    DBG_DIRECT("[SPIC3 PSRAM Read Speed Report] SystemCpuClock = %dM; %dMByte/S",
               SystemCpuClock / 1000000, 1000000 / total_time);
}




void app_system_lower_init(void)
{
    sys_hall_auto_sleep_in_idle(false);

    uint32_t cpu_freq;
    int32_t ret = pm_cpu_freq_set(200, &cpu_freq);
    if (ret != 0)
    {
        DBG_DIRECT("cpu freq config CLK_100MHZ fail ret %x real freq %dMHz", ret, cpu_freq);
    }
    else
    {
        DBG_DIRECT("cpu freq %dMHz ", cpu_freq);
    }

    uint32_t spic0_freq = 0;

    if (fmc_flash_try_high_speed_mode(FMC_SPIC_ID_0, FMC_FLASH_NOR_4_BIT_MODE))
    {
        DBG_DIRECT("flash switch 4bit success");
    }
    else
    {
        DBG_DIRECT("flash switch 4bit fail");
    }
    if (fmc_flash_nor_clock_switch(FMC_SPIC_ID_0, 200, &spic0_freq))
    {
        DBG_DIRECT("set flash clock 200M success");
    }
    else
    {
        DBG_DIRECT("set flash clock 200M fail");
    }


    if (fmc_psram_winbond_opi_init(FMC_SPIC_ID_1))
    {
        DBG_DIRECT("WB OPI psram init success!");
    }
    else
    {
        DBG_DIRECT("WB OPI psram init fail!");
    }

    extern bool fmc_psram_clock_switch(FMC_SPIC_ID spic_id, uint32_t required_mhz,
                                       uint32_t *actual_mhz);

    uint32_t spic1_freq = 0;
    if (fmc_psram_clock_switch(FMC_SPIC_ID_1, 280, &spic1_freq))
    {
        DBG_DIRECT("WB OPI psram switch to 280MHz success!");
    }
    else
    {
        DBG_DIRECT("WB OPI psram switch to 280MHz fail!");
    }

    if (fmc_psram_ap_memory_opi_init(FMC_SPIC_ID_3))
    {
        DBG_DIRECT("APM OPI psram init success!");
    }
    else
    {
        DBG_DIRECT("APM OPI psram init fail!");
    }

    uint32_t spic3_freq = 0;
    if (fmc_psram_clock_switch(FMC_SPIC_ID_3, 280, &spic1_freq))
    {
        DBG_DIRECT("APM OPI psram switch to %d MHz success! actual %d", 280, spic3_freq);
    }
    else
    {
        DBG_DIRECT("APM OPI psram switch to %dMHz fail! actual %d", 280, spic3_freq);
    }

    app_mpu_config();
    return;
}




