/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "board.h"
#include "trace.h"
#include "pm.h"
#include "io_dlps.h"
#include "section.h"
#include "rtl876x_uart.h"
#include "indirect_access.h"
#include "console_uart.h"


static uint32_t dlps_bitmap;

RAM_TEXT_SECTION void app_dlps_enable(uint32_t bit)
{
    if (dlps_bitmap & bit)
    {
        APP_PRINT_TRACE3("app_dlps_enable: %08x %08x -> %08x", bit, dlps_bitmap,
                         (dlps_bitmap & ~bit));
    }
    dlps_bitmap &= ~bit;
}

RAM_TEXT_SECTION void app_dlps_disable(uint32_t bit)
{
    if ((dlps_bitmap & bit) == 0)
    {
        APP_PRINT_TRACE3("app_dlps_disable: %08x %08x -> %08x", bit, dlps_bitmap,
                         (dlps_bitmap | bit));
    }

    dlps_bitmap |= bit;
}

RAM_TEXT_SECTION bool app_dlps_check_callback(void)
{

    static uint16_t dlps_bitmap_pre;
    bool dlps_enter_en = false;
    if (dlps_bitmap == 0)
    {
        dlps_enter_en = true;
    }

    if (dlps_bitmap_pre != dlps_bitmap)
    {
        APP_PRINT_WARN2("app_dlps_check_callback: dlps_bitmap_pre 0x%x dlps_bitmap 0x%x", dlps_bitmap_pre,
                        dlps_bitmap);
    }
    dlps_bitmap_pre = dlps_bitmap;

    return dlps_enter_en;
}

/**
    * @brief   Need to handle message in this callback function,when App enter dlps mode
    * @param  void
    * @return void
    */
RAM_TEXT_SECTION void app_dlps_enter_callback(void)
{
    POWERMode lps_mode = power_mode_get();

    if ((lps_mode == POWER_POWERDOWN_MODE) || (lps_mode == POWER_SHIP_MODE))
    {
        DBG_DIRECT("app_dlps_enter_callback: lps_mode %d", lps_mode);
    }

}

RAM_TEXT_SECTION void app_dlps_exit_callback(void)
{
    APP_PRINT_INFO4("dump aon reg. 0x126 = 0x%x, 0x128 = 0x%x, 0x130 = 0x%x, 0x132 = 0x%x",
                    btaon_fast_read_safe(0x126), btaon_fast_read_safe(0x128), btaon_fast_read_safe(0x130),
                    btaon_fast_read_safe(0x132));

    console_uart_exit_low_power(power_mode_get());
}

void app_dlps_init(void)
{
    /* register of call back function */
    if (power_check_cb_register(app_dlps_check_callback) != 0)
    {
        APP_PRINT_ERROR0("app_dlps_init: dlps_check_cb_reg failed");
    }

    power_stage_cb_register(app_dlps_enter_callback, POWER_STAGE_STORE);
    power_stage_cb_register(app_dlps_exit_callback, POWER_STAGE_RESTORE);
}