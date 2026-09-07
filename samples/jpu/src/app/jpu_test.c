/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */


#include "os_mem.h"
#include "string.h"
#include "rtl_hal_jpu.h"
#include "trace.h"
#include "address_map.h"
#include "rtl876x_rcc.h"
#include "test_img_jpu.c"

#include <zephyr/drivers/display.h>
#include <zephyr/device.h>
#include <stdio.h>

#define RGB565_BG       0x0
static const struct device *dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
static struct display_capabilities cfg;
void JPU_test(void)
{
    // malloc_init();
    hal_jpu_mem_init(malloc, free);
    RCC_PeriphClockCmd(APBPeriph_JPEG, APBPeriph_JPEG_CLOCK, ENABLE);

    if (!device_is_ready(dev))
    {
        printf("Display device not ready");
        return;
    }
    printf("Display device is ready");

    display_get_capabilities(dev, &cfg);
    uint8_t *buffer = (uint8_t *)SPIC1_MEM_BASE;
    uint16_t *buf_u16 = (uint16_t *)buffer;
    DBG_DIRECT("buffer addr %x", buffer);
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
    display_write(dev, 0, 0, &desc_bg, buffer);
    DBG_DIRECT("RTL87x3G JPU demo start !!\r\n");

    // extern void *hw_jpeg_load(void *input, int len, int *w, int *h, int *channel);
    // framebuffer = hw_jpeg_load((void *)_acflower, 222201, 300, 168, 3);

    JPU_DEC_PARAM dec_param;
    uint8_t *output = NULL;
    uint32_t dec_size = 0, dec_w = 0, dec_h = 0;

    JPU_ERROR err;

    memset(&dec_param, 0, sizeof(JPU_DEC_PARAM));
    dec_param.data = (void *)_actest_img_jpu;
    dec_param.size = sizeof(_actest_img_jpu) - 1;
    dec_param.frameFormat = PACKED_FORMAT_422_YUYV;
    dec_param.useWrapper = 1;
    // TODO: need more info from upper layer to determine output RGB type
    dec_param.rgbType = JPU_RGB565;
    // DBG_DIRECT("img 0x%x, sz %d", dec_param.data, dec_param.size);

    err = hal_jpu_decode(&dec_param, &output, &dec_size, &dec_w, &dec_h);
    if (err != JPU_SUCCESS)
    {
        DBG_DIRECT("img 0x%x, sz %d", dec_param.data, dec_param.size);
        DBG_DIRECT("decode jpeg failed, err: %d", err);
        return;
    }
    uint16_t start_x = 150;
    uint16_t start_y = 150;
    int dest_offset = (start_y * dc_width + start_x) * 2;
    for (int i = 0; i < dec_h; i++)
    {
        memcpy(buffer + dest_offset + i * dc_width * 2, output + i * dec_w * 2, 100 * 2);
    }


    struct display_buffer_descriptor desc_img =
    {
        .width = dc_width,
        .height = dc_height,
    };
    display_write(dev, 0, 0, &desc_img, buffer);

    DBG_DIRECT("RTL87x3G JPU demo end !!\r\n");
}
