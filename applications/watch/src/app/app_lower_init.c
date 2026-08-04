/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <zephyr/devicetree.h>
#include <string.h>
#include <os_sched.h>
#include <stdio.h>
#include <trace.h>
#include "fmc_api.h"
#include "os_mem.h"
#include "os_heap.h"
#include "os_msg.h"
#include "dma_channel.h"
#include "fmc_api_ext.h"
#include <fmc_api.h>
#include "platform_utils.h"
#include "wdg.h"
#include "pm.h"
#include "system_status_api.h"
#include "os_task.h"
#include "cm55_setting.h"
#include "flash_map.h"
#include "mpu.h"
#include "clk_mgr.h"

extern uint32_t sys_timestamp_get_us(void);
#define ZEPHYR_DMA_CHANNEL_MASK     (BIT10 | BIT11 | BIT12 | BIT13 | BIT14 | BIT15)

static int app_mpu_config(void)
{
    uint32_t mpu_cfg_rbar = SPIC1_MEM_BASE | (1 << 1); // SH:0, AP:1, XN:0
    uint32_t limit_addr = SPIC1_MEM_BASE + 0x400000; // PXN:0, EN:1
    uint8_t region_num = 2;
    // uint8_t attribute = 0xFF;
    uint8_t attribute = 0xAA;
    // uint8_t attribute = 0x44;

    mpu_config_print();
    mpu_set_region(mpu_cfg_rbar, limit_addr, region_num, attribute, true);

    mpu_cfg_rbar = SPIC3_MEM_BASE | (1 << 1); // SH:0, AP:1, XN:0
    limit_addr = SPIC3_MEM_BASE + DT_REG_SIZE(DT_NODELABEL(psram1_for_mcu)) - 1; // PXN:0, EN:1
    region_num = 4;
    //attribute = 0xFF;
    // attribute = 0x44;
    attribute = 0xAA;

    mpu_set_region(mpu_cfg_rbar, limit_addr, region_num, attribute, true);

    /* Last 1M of psram1 (psram1_nc) is a dedicated non-cacheable region.
     * Variables placed in the PSRAM1_NC linker section land here; the region
     * must be non-cacheable so cross-thread reads/writes stay coherent
     * (e.g. the alipay TLSF heap, shared across shell / alipay_task / SDK).
     * Address/size are read from the dts node so the MPU region tracks board
     * layout changes. Configuring the MPU by address is fine - the "use a
     * variable, not the raw address" rule applies to data access, not to
     * MPU setup. */
    mpu_cfg_rbar = DT_REG_ADDR(DT_NODELABEL(psram1_nc)) | (1 << 1);
    limit_addr   = DT_REG_ADDR(DT_NODELABEL(psram1_nc))
                   + DT_REG_SIZE(DT_NODELABEL(psram1_nc)) - 1;
    region_num = 5;
    attribute = 0x44;  /* non-cacheable */
    mpu_set_region(mpu_cfg_rbar, limit_addr, region_num, attribute, true);

    mpu_config_print();
    return 0;
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
    APP_PRINT_TRACE1("[Checksum] %u", checksum);

    uint32_t time_ms = total_time / 1000;
    uint32_t time_us = total_time % 1000;

    APP_PRINT_TRACE3("[Nor Flash Read Speed Report] SystemCpuClock = %dM; Read 1M byte t=%d.%d ms",
                     SystemCpuClock / 1000000, time_ms, time_us);
    APP_PRINT_TRACE2("[Nor Flash Read Speed Report] SystemCpuClock = %dM; %dMByte/S",
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
    APP_PRINT_TRACE1("[Checksum] %u", checksum);

    uint32_t time_ms = total_time / 1000;
    uint32_t time_us = total_time % 1000;

    APP_PRINT_TRACE3("[SPIC1 PSRAM Read Speed Report] SystemCpuClock = %dM; Read 1M byte t=%d.%d ms",
                     SystemCpuClock / 1000000, time_ms, time_us);
    APP_PRINT_TRACE2("[SPIC1 PSRAM Read Speed Report] SystemCpuClock = %dM; %dMByte/S",
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
    APP_PRINT_TRACE1("[Checksum] %u", checksum);

    uint32_t time_ms = total_time / 1000;
    uint32_t time_us = total_time % 1000;

    APP_PRINT_TRACE3("[SPIC3 PSRAM Read Speed Report] SystemCpuClock = %dM; Read 1M byte t=%d.%d ms",
                     SystemCpuClock / 1000000, time_ms, time_us);
    APP_PRINT_TRACE2("[SPIC3 PSRAM Read Speed Report] SystemCpuClock = %dM; %dMByte/S",
                     SystemCpuClock / 1000000, 1000000 / total_time);
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
    app_mpu_config();
    sys_hall_auto_sleep_in_idle(false);
#if !CONFIG_APP_NANDBOOT
    if (fmc_flash_try_high_speed_mode(FMC_SPIC_ID_0, FMC_FLASH_NOR_4_BIT_MODE))
    {
        APP_PRINT_TRACE0("flash switch 4bit success");
    }
    else
    {
        APP_PRINT_TRACE0("flash switch 4bit fail");
    }

    if (DT_REG_SIZE(DT_NODELABEL(flash)) > 16 * 1024 * 1024)
    {
        fmc_flash_set_4_byte_address_mode(FMC_SPIC_ID_0, true);
    }
#endif

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
#endif

    clk_mgr_init();
    app_io_resource_cfg();

    return;
}
