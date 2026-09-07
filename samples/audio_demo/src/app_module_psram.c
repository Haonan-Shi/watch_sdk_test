/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "trace.h"
#include "app_module_psram.h"
#include "fmc_api.h"
#include "fmc_api_ext.h"
#include "pm.h"
#include "rtl876x.h"
#if (CONFIG_SOC_SERIES_RTL8773E == 1)
#include "platform_utils.h"
#include "platform_ext.h"
#endif
#if CONFIG_REALTEK_APP_GUI
#include "app_gui.h"
#endif

//#define PSRAM_DMA_READ_WRITE_TEST
//#define PSRAM_READ_WRITE_TEST

#if ((TARGET_RTL87X3E == 1) || (CONFIG_SOC_SERIES_RTL8773E == 1))
#define LCD_PSRAM_INDEX      FMC_SPIC_ID_1
#elif (CONFIG_SOC_SERIES_RTL8773D == 1)
#define LCD_PSRAM_INDEX      FMC_SPIC_ID_4
#elif (CONFIG_SOC_SERIES_RTL87X3G == 1)
#define LCD_PSRAM_INDEX1      FMC_SPIC_ID_1
#define LCD_PSRAM_INDEX2      FMC_SPIC_ID_3
#endif

#if (CONFIG_SOC_SERIES_RTL8773E == 1)
#define LCD_PSRAM_FREQ_MHZ   160
#elif (CONFIG_SOC_SERIES_RTL87X3G == 1)
#define LCD_PSRAM_FREQ_MHZ   280
#else
#define LCD_PSRAM_FREQ_MHZ   80
#endif

extern bool fmc_psram_clock_switch(FMC_SPIC_ID spic_id, uint32_t required_mhz,
                                   uint32_t *actual_mhz);

#ifdef PSRAM_DMA_READ_WRITE_TEST
#include "rtl876x_rcc.h"
#include "rtl876x_gdma.h"

#define LCD_DMA_CHANNEL_NUM              lcd_dma_ch_num
#define LCD_DMA_CHANNEL_INDEX            DMA_CH_BASE(lcd_dma_ch_num)
#define LCD_DMA_CHANNEL_IRQ              DMA_CH_IRQ(lcd_dma_ch_num)
#endif

#ifdef PSRAM_DMA_READ_WRITE_TEST
static uint32_t color_data[64] = {0xf800f8, 0xf800f8, 0xf800f8, 0xf800f8, 0xf800f8, 0xf800f8, 0xf800f8, 0xf800f8};
static void dma_read_write_test(uint32_t psram_addr, uint32_t ram_buf_addr, bool is_read)
{
    uint32_t read_buf = 0;
    uint32_t read_addr = 0;

    RCC_PeriphClockCmd(APBPeriph_GDMA, APBPeriph_GDMA_CLOCK, ENABLE);
    /* Initialize GDMA peripheral */
    GDMA_InitTypeDef GDMA_InitStruct;
    GDMA_StructInit(&GDMA_InitStruct);
    GDMA_InitStruct.GDMA_ChannelNum          = LCD_DMA_CHANNEL_NUM;

    GDMA_InitStruct.GDMA_SourceDataSize      = GDMA_DataSize_Word;
    GDMA_InitStruct.GDMA_DestinationDataSize = GDMA_DataSize_Word;
    GDMA_InitStruct.GDMA_SourceMsize         = GDMA_Msize_8;
    GDMA_InitStruct.GDMA_DestinationMsize    = GDMA_Msize_8;

    GDMA_InitStruct.GDMA_DIR                 = GDMA_DIR_MemoryToMemory;

    if (is_read)
    {
        GDMA_InitStruct.GDMA_SourceInc           = DMA_SourceInc_Inc;
        GDMA_InitStruct.GDMA_DestinationInc      = DMA_DestinationInc_Fix;
        GDMA_InitStruct.GDMA_DestinationAddr     = ram_buf_addr;
        GDMA_InitStruct.GDMA_SourceAddr          = psram_addr;
    }
    else
    {
        GDMA_InitStruct.GDMA_SourceInc           = DMA_SourceInc_Fix;
        GDMA_InitStruct.GDMA_DestinationInc      = DMA_DestinationInc_Inc;
        GDMA_InitStruct.GDMA_DestinationAddr     = psram_addr;
        GDMA_InitStruct.GDMA_SourceAddr          = ram_buf_addr;
    }

    GDMA_InitStruct.GDMA_BufferSize          = 240 * 240 * 2 / 4;

    GDMA_Init(LCD_DMA_CHANNEL_INDEX, &GDMA_InitStruct);
    GDMA_INTConfig(LCD_DMA_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);

    uint32_t old_stamp = ((*(uint32_t *)(0x4005817C)) & 0x3FFFFFF);


    GDMA_Cmd(LCD_DMA_CHANNEL_NUM, ENABLE);

    while (GDMA_GetTransferINTStatus(LCD_DMA_CHANNEL_NUM) != SET);
    GDMA_ClearINTPendingBit(LCD_DMA_CHANNEL_NUM, GDMA_INT_Transfer);

    uint32_t new_stamp = ((*(uint32_t *)(0x4005817C)) & 0x3FFFFFF);
    uint32_t system_cpu_clock = pm_cpu_freq_get() * 1000000;
    uint32_t time_per_count = 1000000000 / system_cpu_clock; //use float parameter will be better
    uint32_t time = time_per_count * (new_stamp - old_stamp);
    uint32_t time_ms = time / 1000000;
    uint32_t time_us = time / 1000 - time_ms * 1000;

    APP_PRINT_TRACE2("dma_read_write_test: time %d.%d ms", time_ms, time_us);

    if (is_read)
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            APP_PRINT_TRACE2("dma_read_write_test: data[%d] 0x%x", i, color_data[i]);
        }
    }
    else
    {
        for (uint32_t i = 0; i < 240 * 240 * 2 / 4; i++)
        {
            read_addr = 0x4000000 + i * 4;
            read_buf = *(uint32_t volatile *)read_addr;

            if (read_buf != 0xf800f8)
            {
                APP_PRINT_ERROR2("dma_read_write_test: fail! addr 0x%p, buf 0x%x", read_addr, read_buf);
            }
        }
    }
}
#endif

void app_apm_qspi_psram_init(void)
{
#if ((TARGET_RTL87X3E == 1) || (CONFIG_SOC_SERIES_RTL8773E == 1))
    uint32_t psram_freq = 0;
    bool fmc_psram_ap_memory_qpi_init_ret = fmc_psram_ap_memory_qpi_init(FMC_SPIC_ID_1);

    bool fmc_psram_clock_switch_ret = fmc_psram_clock_switch(FMC_SPIC_ID_1, 80, &psram_freq);
#elif (CONFIG_SOC_SERIES_RTL8773D == 1)
    uint32_t psram_freq = 0;

    bool fmc_psram_ap_memory_qpi_init_ret = fmc_psram_spi_ap_memory_qpi_init(LCD_PSRAM_INDEX);

    bool fmc_psram_clock_switch_ret = fmc_psram_spi_clock_switch(LCD_PSRAM_INDEX, 80, &psram_freq);
#elif (CONFIG_SOC_SERIES_RTL87X3G == 1)
    uint32_t psram_freq = 0;
    bool fmc_psram_ap_memory_qpi_init_ret = fmc_psram_ap_memory_qpi_init(LCD_PSRAM_INDEX2);

    bool fmc_psram_clock_switch_ret = fmc_psram_clock_switch(LCD_PSRAM_INDEX2, LCD_PSRAM_FREQ_MHZ,
                                                             &psram_freq);
#endif

    APP_PRINT_INFO3("app_apm_qspi_psram_init: fmc_psram_ap_memory_qpi_init_ret %d, fmc_psram_clock_switch_ret %d, actual freq %d",
                    fmc_psram_ap_memory_qpi_init_ret, fmc_psram_clock_switch_ret, psram_freq);

#ifdef PSRAM_READ_WRITE_TEST
    app_apm_psram_read_write_test();
#endif
}

void app_apm_opi_psram_init(void)
{
#if ((TARGET_RTL87X3E == 1) || (CONFIG_SOC_SERIES_RTL8773E == 1))
    bool fmc_psram_ap_memory_opi_init_ret = fmc_psram_ap_memory_opi_init(FMC_SPIC_ID_1);
    uint32_t psram_freq = 0;
    bool fmc_psram_clock_switch_ret = fmc_psram_clock_switch(FMC_SPIC_ID_1, 80, &psram_freq);
#elif (CONFIG_SOC_SERIES_RTL8773D == 1)
    bool fmc_psram_ap_memory_opi_init_ret = fmc_psram_ap_memory_opi_init(FMC_SPIC_ID_1);
    uint32_t psram_freq = 0;
    bool fmc_psram_clock_switch_ret = fmc_psram_spi_clock_switch(LCD_PSRAM_INDEX, 80, &psram_freq);
#elif (CONFIG_SOC_SERIES_RTL87X3G == 1)
    bool fmc_psram_ap_memory_opi_init_ret = fmc_psram_ap_memory_opi_init(LCD_PSRAM_INDEX2);
    uint32_t psram_freq = 0;
    bool fmc_psram_clock_switch_ret = fmc_psram_clock_switch(LCD_PSRAM_INDEX2, LCD_PSRAM_FREQ_MHZ,
                                                             &psram_freq);
#endif

    APP_PRINT_INFO3("app_apm_opi_psram_init: fmc_psram_ap_memory_opi_init_ret %d, fmc_psram_clock_switch_ret %d, actual freq %d",
                    fmc_psram_ap_memory_opi_init_ret, fmc_psram_clock_switch_ret, psram_freq);

#ifdef PSRAM_READ_WRITE_TEST
    app_apm_psram_read_write_test();
#endif
#ifdef PSRAM_DMA_READ_WRITE_TEST
    dma_read_write_test(IMAGE_TEST240_BIN, (uint32_t)color_data, true);
#endif
}

#ifdef PSRAM_READ_WRITE_TEST
void app_apm_psram_read_write_test(void)
{
#if (CONFIG_SOC_SERIES_RTL87X3G == 1)
    uint32_t start_test_addr = SPIC1_MEM_BASE;
#else
    uint32_t start_test_addr = 0x4000000;
#endif
    uint32_t color_buf = 0;
    uint16_t color = RED;
    uint32_t read_buf = 0;
    uint32_t read_addr = 0;

    color_buf = (color >> 8) | (color << 8);
    color_buf = color_buf | color_buf << 16;

    uint32_t old_stamp = ((*(uint32_t *)(0x4005817C)) & 0x3FFFFFF);
    for (uint32_t i = 0; i < 240 * 240 * 2 / 4; i++)
    {
        *(uint32_t *)(start_test_addr + i * 4) = color_buf;
    }
    uint32_t new_stamp = ((*(uint32_t *)(0x4005817C)) & 0x3FFFFFF);
    uint32_t system_cpu_clock = pm_cpu_freq_get() * 1000000;
    uint32_t time_per_count = 1000000000 / system_cpu_clock; //use float parameter will be better
    uint32_t time = time_per_count * (new_stamp - old_stamp);
    uint32_t time1_ms = time / 1000000;
    uint32_t time1_us = time / 1000 - time1_ms * 1000;

    for (uint32_t i = 0; i < 240 * 240 * 2 / 4; i++)
    {
//        read_addr = start_test_addr + i * 4;
//        read_buf = *(uint32_t volatile *)read_addr;
//        if (read_buf != color_buf)
//        {
//            APP_PRINT_ERROR2("app_apm_psram_read_write_test: test 1 fail!! addr 0x%p, buf 0x%x", read_addr,
//                             read_buf);
//        }
    }

    old_stamp = ((*(uint32_t *)(0x4005817C)) & 0x3FFFFFF);
    time_per_count = 1000000000 / system_cpu_clock; //use float parameter will be better
    time = time_per_count * (old_stamp - new_stamp);
    uint32_t time2_ms = time / 1000000;
    uint32_t time2_us = time / 1000 - time2_ms * 1000;
    APP_PRINT_TRACE4("app_apm_psram_read_write_test: time1 %d.%d ms, time2 %d.%d ms", time1_ms,
                     time1_us, time2_ms, time2_us) ;

    for (uint32_t i = 0; i <  240 * 240 * 2 / 4; i++)
    {
        read_addr = start_test_addr + i * 4;
        read_buf = *(uint32_t volatile *)read_addr;
        if (read_buf != color_buf)
        {
            APP_PRINT_ERROR2("app_apm_psram_read_write_test: test 2 fail!! addr 0x%p, buf 0x%x", read_addr,
                             read_buf);
        }
    }
}
#endif

void app_wb_opi_psram_init(void)
{
#if (CONFIG_SOC_SERIES_RTL8773E == 1)
    ldo_aux2_power_control(ENABLE);
    platform_delay_us(150);
#endif

#if ((TARGET_RTL87X3E == 1) || (CONFIG_SOC_SERIES_RTL8773E == 1))
    uint32_t psram_freq = 0;
    bool fmc_psram_winbond_opi_init_ret = fmc_psram_winbond_opi_init(FMC_SPIC_ID_1);

    bool fmc_psram_clock_switch_ret = fmc_psram_clock_switch(FMC_SPIC_ID_1, LCD_PSRAM_FREQ_MHZ,
                                                             &psram_freq);
#elif (CONFIG_SOC_SERIES_RTL8773D == 1)
    uint32_t psram_freq = 0;

    bool fmc_psram_winbond_opi_init_ret = fmc_psram_spi_winbond_opi_init();

    bool fmc_psram_clock_switch_ret = fmc_psram_spi_clock_switch(LCD_PSRAM_INDEX, 80, &psram_freq);
#elif (CONFIG_SOC_SERIES_RTL87X3G == 1)
    uint32_t psram_freq1 = 0;
    bool fmc_psram_winbond_opi_init_ret1 = fmc_psram_winbond_opi_init(LCD_PSRAM_INDEX1);

    bool fmc_psram_clock_switch_ret1 = fmc_psram_clock_switch(LCD_PSRAM_INDEX1, LCD_PSRAM_FREQ_MHZ,
                                                              &psram_freq1);

    uint32_t psram_freq2 = 0;
    bool fmc_psram_winbond_opi_init_ret2 = fmc_psram_winbond_opi_init(LCD_PSRAM_INDEX2);

    bool fmc_psram_clock_switch_ret2 = fmc_psram_clock_switch(LCD_PSRAM_INDEX2, LCD_PSRAM_FREQ_MHZ,
                                                              &psram_freq2);
#endif

#if (CONFIG_SOC_SERIES_RTL87X3G == 1)
    APP_PRINT_INFO3("app_wb_opi_psram_init for spic1: fmc_psram_winbond_opi_init_ret1 %d, fmc_psram_clock_switch_ret1 %d, actual freq1 %d",
                    fmc_psram_winbond_opi_init_ret1, fmc_psram_clock_switch_ret1, psram_freq1);
    APP_PRINT_INFO3("app_wb_opi_psram_init for spic3: fmc_psram_winbond_opi_init_ret2 %d, fmc_psram_clock_switch_ret2 %d, actual freq2 %d",
                    fmc_psram_winbond_opi_init_ret2, fmc_psram_clock_switch_ret2, psram_freq2);
#else

    APP_PRINT_INFO3("app_wb_opi_psram_init: fmc_psram_winbond_opi_init_ret %d, fmc_psram_clock_switch_ret %d, actual freq %d",
                    fmc_psram_winbond_opi_init_ret, fmc_psram_clock_switch_ret, psram_freq);
#endif
#ifdef PSRAM_READ_WRITE_TEST
    app_wb_opi_psram_read_write_test();
    app_apm_psram_read_write_test();
#endif
#ifdef PSRAM_DMA_READ_WRITE_TEST
    dma_read_write_test(0x4000000, (uint32_t)color_data, true);
#endif
}

#ifdef PSRAM_READ_WRITE_TEST
void app_wb_opi_psram_read_write_test(void)
{
#if (CONFIG_SOC_SERIES_RTL87X3G == 1)
    uint32_t start_test_addr = SPIC1_MEM_BASE;
#else
    uint32_t start_test_addr = 0x4000000;
#endif
    uint32_t color_buf = 0;
    uint16_t color = BLUE;
    uint32_t read_buf = 0;
    uint32_t read_addr = 0;

    color_buf = (color >> 8) | (color << 8);
    color_buf = color_buf | color_buf << 16;

    uint32_t old_stamp = ((*(uint32_t *)(0x4005817C)) & 0x3FFFFFF);
    for (uint32_t i = 0; i < LCD_WIDTH * LCD_HIGHT * PIXEL_BYTES / 4; i++)
    {
        *(uint32_t *)(start_test_addr + i * 4) = color_buf;
    }
    uint32_t new_stamp = ((*(uint32_t *)(0x4005817C)) & 0x3FFFFFF);
    uint32_t system_cpu_clock = pm_cpu_freq_get() * 1000000;
    uint32_t time_per_count = 1000000000 / system_cpu_clock; //use float parameter will be better
    uint32_t time = time_per_count * (new_stamp - old_stamp);
    uint32_t time1_ms = time / 1000000;
    uint32_t time1_us = time / 1000 - time1_ms * 1000;

    for (uint32_t i = 0; i < LCD_WIDTH * LCD_HIGHT * PIXEL_BYTES / 4; i++)
    {
//        read_addr = 0xA000000 + i * 4;
//        read_buf = *(uint32_t volatile *)read_addr;
//        if (read_buf != color_buf)
//        {
//            app_wb_opi_psram_read_write_test("app_wb_opi_psram_read_write_test: test 1 fail!! addr 0x%p, buf 0x%x", read_addr,
//                             read_buf);
//        }
    }

    old_stamp = ((*(uint32_t *)(0x4005817C)) & 0x3FFFFFF);
    time_per_count = 1000000000 / system_cpu_clock; //use float parameter will be better
    time = time_per_count * (old_stamp - new_stamp);
    uint32_t time2_ms = time / 1000000;
    uint32_t time2_us = time / 1000 - time2_ms * 1000;
    APP_PRINT_TRACE4("app_wb_opi_psram_read_write_test: time1 %d.%d ms, time2 %d.%d ms",
                     time1_ms, time1_us, time2_ms, time2_us);

    for (uint32_t i = 0; i < LCD_WIDTH * 2 * PIXEL_BYTES / 4; i++)
    {
        read_addr = start_test_addr + i * 4;
        read_buf = *(uint32_t volatile *)read_addr;
        if (read_buf != color_buf)
        {
            APP_PRINT_ERROR2("app_wb_opi_psram_read_write_test: test 2 fail!! addr 0x%p, buf 0x%x", read_addr,
                             read_buf);
        }
    }
}
#endif

void app_psram_enter_dlps(void)
{
#if (CONFIG_SOC_SERIES_RTL87X3G == 1)
    fmc_psram_enter_lpm(LCD_PSRAM_INDEX1, FMC_PSRAM_LPM_HALF_SLEEP_MODE);
    fmc_psram_enter_lpm(LCD_PSRAM_INDEX2, FMC_PSRAM_LPM_HALF_SLEEP_MODE);
#else
    fmc_psram_enter_lpm(LCD_PSRAM_INDEX, FMC_PSRAM_LPM_HALF_SLEEP_MODE);
#endif
}

void app_psram_exit_dlps(void)
{
#if (CONFIG_SOC_SERIES_RTL87X3G == 1)
    fmc_psram_exit_lpm(LCD_PSRAM_INDEX1, FMC_PSRAM_LPM_HALF_SLEEP_MODE);
    fmc_psram_exit_lpm(LCD_PSRAM_INDEX2, FMC_PSRAM_LPM_HALF_SLEEP_MODE);
#else
    fmc_psram_exit_lpm(LCD_PSRAM_INDEX, FMC_PSRAM_LPM_HALF_SLEEP_MODE);
#endif
}

void app_psram_init(void)
{
#if (CONFIG_SOC_SERIES_RTL87X3G == 1)
    app_wb_opi_psram_init();
    //app_apm_opi_psram_init();
#else
    app_wb_opi_psram_init();
#endif
}
