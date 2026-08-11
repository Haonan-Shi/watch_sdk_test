/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */


#include "os_mem.h"
#include "string.h"
#include "rtl_idu.h"
#include "rtl_idu_int.h"
#include "trace.h"
#include "address_map.h"
#include "rtl876x_rcc.h"
#include "test_img_idu.c"
#include <dma_channel.h>
#include "idu_test_source.h"

#include <zephyr/drivers/display.h>
#include <zephyr/device.h>
#include <stdio.h>

#define RGB565_BG       0x0
static const struct device *dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
static struct display_capabilities cfg;

static uint8_t high_speed_dma_channel_num = 0xFF;
static uint8_t low_speed_dma_channel_num = 0xFF;

uint8_t output[100 * 100 * 2];
void IDU_test(void)
{
    if (!device_is_ready(dev))
    {
        printf("Display device not ready");
        return;
    }
    printf("Display device is ready");

    display_get_capabilities(dev, &cfg);

    GDMA_channel_request(&high_speed_dma_channel_num, NULL, true);
    GDMA_channel_request(&low_speed_dma_channel_num, NULL, false);
    RCC_PeriphClockCmd(APBPeriph_IDU, APBPeriph_IDU_CLOCK, ENABLE);
    uint8_t *framebuffer = (uint8_t *)SPIC1_MEM_BASE;
    uint16_t *buf_u16 = (uint16_t *)framebuffer;
    DBG_DIRECT("buffer addr %x", framebuffer);
    uint32_t dc_width = cfg.x_resolution;
    uint32_t dc_height = cfg.y_resolution;
    for (int i = 0; i < dc_width * dc_height; i++)
    {
        buf_u16[i] = RGB565_BG;
    }
    struct display_buffer_descriptor desc_bg =
    {
        .width = dc_width,
        .height = dc_height,
    };
    display_write(dev, 0, 0, &desc_bg, framebuffer);
    DBG_DIRECT("RTL87x3G IDU demo start !!\r\n");

    uint8_t *img_src = (uint8_t *)_actest_img_idu;
    IDU_file_header *header = (IDU_file_header *)img_src;
    // DBG_DIRECT("IDU file width %d, height %d", header->raw_pic_width, header->raw_pic_height);
    IDU_decode_range range;
    range.start_column = 0;
    range.end_column = header->raw_pic_width - 1;
    range.start_line = 0;
    range.end_line = header->raw_pic_height - 1;
    range.target_stride = (range.end_column - range.start_column + 1) *
                          rtl_idu_get_actual_pixel_length((header->algorithm_type.pixel_bytes));
    IDU_DMA_config dma_cfg = {0};
    dma_cfg.output_buf = (uint32_t *)output;
    dma_cfg.RX_DMA_channel_num = low_speed_dma_channel_num;
    dma_cfg.TX_DMA_channel_num = high_speed_dma_channel_num;
    dma_cfg.TX_FIFO_INT_threshold = 8;
    dma_cfg.RX_FIFO_INT_threshold = 8;
    IDU_ERROR err = IDU_Decode((uint8_t *)img_src, &range, &dma_cfg);


    if (err != IDU_SUCCESS)
    {
        DBG_DIRECT("IDU decode error: %d", err);
        return;
    }
    uint16_t start_x = 150;
    uint16_t start_y = 150;
    uint32_t dest_offset = (start_y * dc_width + start_x) * 2;

    for (int i = 0; i < 100; i++)
    {
        memcpy(framebuffer + dest_offset + i * dc_width * 2, output + i * 100 * 2, 100 * 2);
    }
    struct display_buffer_descriptor desc_img =
    {
        .width = dc_width,
        .height = dc_height,
    };

    display_write(dev, 0, 0, &desc_img, framebuffer);
    uint32_t img_src_size = sizeof(_actest_img_idu) - 9;
    uint32_t img_decoded_size = sizeof(_actest_img_raw) - 1;
    float compression_rate = 100.0f * (1.0f - (float)img_src_size / img_decoded_size);
    DBG_DIRECT("Space Saved: %.2f %%\n", compression_rate);
    DBG_DIRECT("RTL87x3G IDU demo end !!\r\n");
}
