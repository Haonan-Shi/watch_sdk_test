/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */


#include "os_mem.h"
#include "sample_image.h"
#include "string.h"
#include "rtl_ppe.h"
#include "trace.h"
#include "address_map.h"
#include "rtl876x_rcc.h"
#include <zephyr/drivers/display.h>
#include <zephyr/device.h>
#include <stdio.h>

#define RGB565_BG       0x0

static const struct device *dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
static struct display_capabilities cfg;
void PPE_test(void)
{
    RCC_PeriphClockCmd(APBPeriph_PPE, APBPeriph_PPE_CLOCK, ENABLE);

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
    DBG_DIRECT("RTL87x3G PPE demo start !!\r\n");

    ppe_buffer_t img, target;
    memset(&img, 0, sizeof(ppe_buffer_t));
    memset(&target, 0, sizeof(ppe_buffer_t));
    target.format = PPE_RGB565;
    target.address = (uint32_t)buffer;
    target.width = dc_width;
    target.height = dc_height;
    target.stride = target.width;
    target.win_x_min = 0;
    target.win_x_max = target.width - 1;
    target.win_y_min = 0;
    target.win_y_max = target.height - 1;

    img.format = PPE_RGB565;
    img.address = (uint32_t)disc2_data;
    img.width = 100;
    img.height = 100;
    img.stride = img.width;
    img.color_key_config.key_enable.key_enable = false;
    img.high_quality = true;
    img.const_color = 0xFFFFFFFF;
    img.opacity = 0x80;
    img.win_x_min = 0;
    img.win_x_max = target.width - 1;
    img.win_y_min = 0;
    img.win_y_max = target.height - 1;

    ppe_matrix_t inv_matrix;

    ppe_get_identity(&inv_matrix);

    // 1. Move to center of rotation (center of image)
    ppe_translate(-50, -50, &inv_matrix);

    // 2. Apply scaling (make it larger)
    ppe_scale(1.5f, 1.5f, &inv_matrix);

    // 3. Rotate 30 degrees
    ppe_rotate(30.0f, &inv_matrix);

    // 4. Move to final position on screen
    ppe_translate(-150, -200, &inv_matrix);

    ppe_rect_t rect = {.x1 = 0, .y1 = 0, .x2 = dc_width, .y2 = dc_height};
    PPE_Blit_Inverse(&target, &img, NULL, &inv_matrix, &rect, PPE_BLEND_PREMULTIPLY);
    PPE_Finish();

    struct display_buffer_descriptor desc_img =
    {
        .width = dc_width,
        .height = dc_height,
    };
    display_write(dev, 0, 0, &desc_img, buffer);
    DBG_DIRECT("RTL87x3G PPE demo end !!\r\n");
}
