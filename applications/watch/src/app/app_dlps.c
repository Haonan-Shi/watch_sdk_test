/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <zephyr/devicetree.h>
#include "trace.h"
#include "app_dlps.h"
#include "app_main.h"
#include "section.h"
#include "app_cfg.h"
#include "app_mmi.h"
#include "app_audio_policy.h"
#include "pm.h"
#include "vector_table.h"
#include "io_dlps.h"
#include "os_timer.h"
#include "system_status_api.h"
#include "hub_task.h"
#include "fmc_api.h"
#include "fmc_api_ext.h"
#include "console_uart.h"
#include "app_cmd.h"

uint32_t dlps_bitmap = 0;                /**< dlps locking bitmap */

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
    //DBG_DIRECT("app_dlps_check_callback bitmap 0x%x", dlps_bitmap);
    if (dlps_bitmap == 0)
    {
        dlps_enter_en = true;
    }

    if ((dlps_bitmap_pre != dlps_bitmap) && !dlps_enter_en)
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
void app_dlps_enter_callback(void)
{
#if DT_NODE_HAS_STATUS(DT_NODELABEL(psram0), okay) && !CONFIG_APP_NANDBOOT
    fmc_psram_enter_lpm(FMC_SPIC_ID_1, FMC_PSRAM_LPM_HALF_SLEEP_MODE);
#endif
#if DT_NODE_HAS_STATUS(DT_NODELABEL(psram1), okay)
    fmc_psram_enter_lpm(FMC_SPIC_ID_3, FMC_PSRAM_LPM_HALF_SLEEP_MODE);
#endif
    POWERMode lps_mode = power_mode_get();
    DBG_DIRECT("app_dlps_enter_callback");
    if ((POWER_POWERDOWN_MODE == lps_mode) || (POWER_POWEROFF_MODE == lps_mode))
    {
        DBG_DIRECT("app_dlps_enter_callback: lps_mode %d", lps_mode);
    }
#if (F_APP_AUTO_SUPPORT == 1)
    if (console_cfg_const.enable_data_uart)
    {
        console_uart_enter_low_power(lps_mode);
    }
#endif
}

/**
    * @brief   Need to handle message in this callback function,when App exit dlps mode
    * @param  void
    * @return void
    */
void app_dlps_exit_callback(void)
{
#if DT_NODE_HAS_STATUS(DT_NODELABEL(psram0), okay) && !CONFIG_APP_NANDBOOT
    fmc_psram_exit_lpm(FMC_SPIC_ID_1, FMC_PSRAM_LPM_HALF_SLEEP_MODE);
#endif
#if DT_NODE_HAS_STATUS(DT_NODELABEL(psram1), okay)
    fmc_psram_exit_lpm(FMC_SPIC_ID_3, FMC_PSRAM_LPM_HALF_SLEEP_MODE);
#endif
    DBG_DIRECT("app_dlps_exit_callback");
//    sys_hall_get_dlps_aon_info();
#if (F_APP_AUTO_SUPPORT == 1)
    if (console_cfg_const.enable_data_uart || console_cfg_const.data_uart_rx_pinmux)
    {
        console_uart_exit_low_power(POWER_DLPS_MODE);
    }
#endif
}

bool app_dlps_check_enter_bits(uint32_t bit)
{
    return dlps_bitmap & bit;
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

#if CONFIG_APP_NANDBOOT
    fmc_psram_register_lpm_func();
#endif

}

/* When system is wakeup from dlps mode, peripheral will call this function.7
 * App will disable correspondent wakeup pinmux.
 */
//RAM_TEXT_SECTION void System_Handler(void)
//{
//    NVIC_DisableIRQ(System_IRQn);  //disable System_Handler Interrupt
//}
