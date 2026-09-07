/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "trace.h"
#include "pm.h"
#include "io_dlps.h"
#include "section.h"
#include "rtl876x_uart.h"

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

}

void app_dlps_init(void)
{
    /* register of call back function */
    power_check_cb_register(app_dlps_check_callback);

    power_stage_cb_register(app_dlps_enter_callback, POWER_STAGE_STORE);
    power_stage_cb_register(app_dlps_exit_callback, POWER_STAGE_RESTORE);
}
