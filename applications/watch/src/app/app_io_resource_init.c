/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "rtl876x.h"
#include "app_io_resource_init.h"
#include "dma_channel.h"

static uint8_t lcd_dma_ch_num  = 0xA5;


void app_io_resource_request_init(void)
{
    if (!GDMA_channel_request(&lcd_dma_ch_num, NULL, true))
    {
        APP_PRINT_ERROR0("request dma channel for lcd fail!");
        return;
    }
    else
    {
        APP_PRINT_INFO1("lcd dma channel number = %d", lcd_dma_ch_num);
    }

}
