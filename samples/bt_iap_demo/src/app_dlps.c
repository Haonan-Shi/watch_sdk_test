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

    DBG_DIRECT("app_dlps_enter_callback: lps_mode %d", lps_mode);
}

RAM_TEXT_SECTION void app_dlps_exit_callback(void)
{
    APP_PRINT_INFO0("app_dlps_exit_callback");

    console_uart_exit_low_power(power_mode_get());
}

void app_dlps_init(void)
{
    io_dlps_register();

    /* register of call back function */
    power_check_cb_register(app_dlps_check_callback);

    power_stage_cb_register(app_dlps_enter_callback, POWER_STAGE_STORE);
    power_stage_cb_register(app_dlps_exit_callback, POWER_STAGE_RESTORE);

    SYSBLKCTRL->u_208.BITS_208.r_DSP_CLK_SRC_EN = 0;
    bt_power_mode_set(BTPOWER_DEEP_SLEEP);
}
