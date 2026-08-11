/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <zephyr/devicetree.h>
#include "trace.h"
#include "rtl876x_gdma.h"
#include "rtl876x_pinmux.h"
#include "hal_gpio.h"
#include "app_timer.h"
#include "app_dlps.h"

#include "app_main.h"
#include "section.h"
#include "hal_gpio.h"
#include "hal_gpio_int.h"
#include "app_charger.h"
#include "app_charger_cfg.h"
#include "app_cfg.h"

#include "fmc_api.h"
#include "fmc_api_ext.h"


#include "app_mmi.h"
#include "dlps_util.h"

#include "pm.h"
#include "app_charger.h"
#include "app_auto_power_off.h"
#include "os_timer.h"

#include "os_sync.h"
#include "io_dlps.h"
#include "hal_adp.h"
#if F_APP_QDECODE_SUPPORT
#include "app_qdec.h"
#endif

#if F_APP_WIFI_UART_CMD
#include "app_wifi_uart.h"
#endif

#include "app_console.h"

typedef enum
{
    APP_TIMER_POWER_DOWN_WDG = 0x00,
    APP_TIMER_PROFILING_DLPS = 0x01,
} T_APP_DLPS_TIMER;

#define POWER_DOWN_WDG_TIMER        500
#define POWER_DOWN_WDG_CHK_TIMES    40
#define PROFILING_DLPS_TIMER_MS     20*1000

static uint32_t dlps_bitmap; /**< dlps locking bitmap */
static uint8_t app_dlps_timer_id = 0;
static uint8_t timer_idx_power_down_wdg = 0;
static uint8_t timer_idx_profiling_dlps = 0;
static uint32_t pd_wdg_chk_times = 0;

RAM_TEXT_SECTION void app_dlps_enable(uint32_t bit)
{
    uint32_t s;

    if (dlps_bitmap & bit)
    {
        APP_PRINT_TRACE3("app_dlps_enable: %08x %08x -> %08x", bit, dlps_bitmap,
                         (dlps_bitmap & ~bit));
    }

    s = os_lock();
    dlps_bitmap &= ~bit;
    os_unlock(s);
}

RAM_TEXT_SECTION void app_dlps_disable(uint32_t bit)
{
    uint32_t s;

    if ((dlps_bitmap & bit) == 0)
    {
        APP_PRINT_TRACE3("app_dlps_disable: %08x %08x -> %08x", bit, dlps_bitmap,
                         (dlps_bitmap | bit));
    }

    s = os_lock();
    dlps_bitmap |= bit;
    os_unlock(s);
}

RAM_TEXT_SECTION bool app_dlps_check_callback(void)
{
    static uint32_t dlps_bitmap_pre;
    bool dlps_enter_en = false;
    POWERMode lps_mode = power_mode_get();
    bool is_keep_hq = false;

    if (!app_cfg_const.enable_dlps)
    {
        return false;
    }

    io_dlps_set_vio_power(is_keep_hq);



    if ((app_cfg_const.enable_dlps) && (dlps_bitmap == 0))
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
    POWERMode lps_mode = power_mode_get();
    uint32_t i;

    if ((lps_mode == POWER_POWERDOWN_MODE) || (lps_mode == POWER_SHIP_MODE))
    {
        DBG_DIRECT("app_dlps_enter_callback: lps_mode %d", lps_mode);
    }

#if DT_NODE_HAS_STATUS(DT_NODELABEL(psram0), okay)
    fmc_psram_enter_lpm(FMC_SPIC_ID_1, FMC_PSRAM_LPM_HALF_SLEEP_MODE);
#endif
#if DT_NODE_HAS_STATUS(DT_NODELABEL(psram1), okay)
    fmc_psram_enter_lpm(FMC_SPIC_ID_3, FMC_PSRAM_LPM_HALF_SLEEP_MODE);
#endif

    if (lps_mode == POWER_DLPS_MODE || lps_mode == POWER_LPS_MODE)
    {
        if (app_db.device_state != APP_DEVICE_STATE_OFF)
        {

#if F_APP_QDECODE_SUPPORT
            if (app_cfg_const.wheel_support)
            {
                app_qdec_pad_enter_dlps_config();
            }
#endif
        }
    }
    else if (lps_mode == POWER_POWERDOWN_MODE)
    {

#if F_APP_LINEIN_SUPPORT
        if (app_cfg_const.line_in_support)
        {
            hal_gpio_irq_disable(app_cfg_const.line_in_pinmux);
        }
#endif

#if F_APP_QDECODE_SUPPORT
        if (app_cfg_const.wheel_support)
        {
            app_qdec_enter_power_down_cfg();
        }
#endif

    }

}

void app_dlps_exit_callback(void)
{

#if DT_NODE_HAS_STATUS(DT_NODELABEL(psram0), okay)
    fmc_psram_exit_lpm(FMC_SPIC_ID_1, FMC_PSRAM_LPM_HALF_SLEEP_MODE);
#endif
#if DT_NODE_HAS_STATUS(DT_NODELABEL(psram1), okay)
    fmc_psram_exit_lpm(FMC_SPIC_ID_3, FMC_PSRAM_LPM_HALF_SLEEP_MODE);
#endif

    /* add print dlps wake up reason if needed */
    //dlps_utils_print_wake_up_info();

    //POWER_POWERDOWN_MODE and LPM_HIBERNATE_MODE will reboot directly and not execute exit callback

    if (app_cfg_const.enable_data_uart)
    {
        //console_uart_exit_low_power(POWER_DLPS_MODE);
        app_console_send_msg(IO_MSG_CONSOLE_BINARY_TX, NULL);
    }

#if F_APP_WIFI_UART_CMD
    app_wifi_uart_msg_send(IO_WIFI_UART_EXIT_DLPS, NULL);
#endif

#if F_APP_QDECODE_SUPPORT
    if (app_cfg_const.wheel_support)
    {
        if (app_cfg_const.qdec_y_pha_pinmux != 0xFF && app_cfg_const.qdec_y_phb_pinmux != 0xFF)
        {
            if (System_WakeUpInterruptValue(app_cfg_const.qdec_y_pha_pinmux) ||
                System_WakeUpInterruptValue(app_cfg_const.qdec_y_phb_pinmux))
            {
                app_dlps_restore_pad(app_cfg_const.qdec_y_pha_pinmux);
                app_dlps_restore_pad(app_cfg_const.qdec_y_phb_pinmux);
                app_qdec_wakeup_handle();
            }
        }
        app_qdec_pad_exit_dlps_config();
    }
#endif

}

static bool app_dlps_platform_pm_check(void)
{
    uint8_t platform_pm_error_code = power_get_error_code();

    APP_PRINT_INFO1("app_dlps_platform_pm_check, ERR Code:%d", platform_pm_error_code);

    return (platform_pm_error_code == PM_ERROR_WAKEUP_TIME);
    //pmu ctrl, must no error
}

static void app_dlps_timeout_cb(uint8_t timer_evt, uint16_t param)
{
    switch (timer_evt)
    {
    case APP_TIMER_POWER_DOWN_WDG:
        {
            app_stop_timer(&timer_idx_power_down_wdg);

            pd_wdg_chk_times++;
            if (pd_wdg_chk_times == POWER_DOWN_WDG_CHK_TIMES)
            {
                app_auto_power_off_disable(AUTO_POWER_OFF_MASK_ALREADY_POWER_OFF);
                app_dlps_enable(0xFFFF);
            }

            if (app_dlps_platform_pm_check() && app_db.device_state == APP_DEVICE_STATE_OFF)
            {
                pd_wdg_chk_times = 0;
                power_stop_all_non_excluded_timer();
                os_timer_dump();
            }
            else
            {
                /* Because timer_idx_power_down_wdg registers the exclude handle of PM,
                   so it cannot become an auto reload timer, which will trigger assert. */
                app_start_timer(&timer_idx_power_down_wdg, "power_down_wdg",
                                app_dlps_timer_id, APP_TIMER_POWER_DOWN_WDG, 0, false,
                                POWER_DOWN_WDG_TIMER);
            }
        }
        break;

    case APP_TIMER_PROFILING_DLPS:
        {
            uint32_t wakeup_count;
            uint32_t total_wakeup_time;
            uint32_t total_sleep_time;
            uint32_t btmac_wakeup_count, last_wakeup_clk, last_sleep_clk;

            power_get_statistics(&wakeup_count, &total_wakeup_time, &total_sleep_time);
            dlps_utils_get_btmac_lpm_statics(&btmac_wakeup_count, &last_wakeup_clk, &last_sleep_clk);
            TEST_PRINT_INFO6("(LPS) WakeupCount: %d, PowerMode: %d, ErrorCode: 0x%x, RefuseReason: 0x%x. BTMAC WakeupCount: %d, ErrorCode: 0x%x.",
                             wakeup_count, power_mode_get(), dlps_util_get_platform_error_code(),
                             dlps_utils_get_platform_refuse_reason(),
                             btmac_wakeup_count, dlps_util_get_btmac_error_code());

            /* Because timer_idx_profiling_dlps registers the exclude handle of PM,
            so it cannot become an auto reload timer, which will trigger assert. */
            app_start_timer(&timer_idx_profiling_dlps, "profiling_dlps", app_dlps_timer_id,
                            APP_TIMER_PROFILING_DLPS, 0, false, PROFILING_DLPS_TIMER_MS);

        }
        break;

    default:
        break;
    }
}

void app_dlps_power_off(void)
{
    if (app_cfg_const.enable_power_off_to_dlps_mode)
    {
        power_mode_set(POWER_DLPS_MODE);
    }
    else
    {
        power_mode_set(POWER_POWERDOWN_MODE);

        app_start_timer(&timer_idx_power_down_wdg, "power_down_wdg",
                        app_dlps_timer_id, APP_TIMER_POWER_DOWN_WDG, 0, false,
                        POWER_DOWN_WDG_TIMER);

        app_auto_power_off_disable(AUTO_POWER_OFF_MASK_ALREADY_POWER_OFF);
        app_timer_register_pm_excluded(&timer_idx_power_down_wdg);
    }
}

void app_dlps_enable_auto_poweroff_stop_wdg_timer(void)
{
    pd_wdg_chk_times = 0;
    app_auto_power_off_enable(AUTO_POWER_OFF_MASK_ALREADY_POWER_OFF,
                              app_cfg_const.timer_auto_power_off);
    app_stop_timer(&timer_idx_power_down_wdg);
}

void app_dlps_stop_power_down_wdg_timer(void)
{
    pd_wdg_chk_times = 0;
    app_stop_timer(&timer_idx_power_down_wdg);
}

void app_dlps_start_power_down_wdg_timer(void)
{
    if (app_db.device_state != APP_DEVICE_STATE_ON)
    {
        app_start_timer(&timer_idx_power_down_wdg, "power_down_wdg",
                        app_dlps_timer_id, APP_TIMER_POWER_DOWN_WDG, 0, false,
                        POWER_DOWN_WDG_TIMER);
    }
}

RAM_TEXT_SECTION uint32_t app_dlps_get_dlps_bitmap(void)
{
    return dlps_bitmap;
}

ISR_TEXT_SECTION void app_dlps_pad_wake_up_polarity_invert(uint8_t pinmux)
{
    if (pinmux != 0xFF)
    {
        uint8_t gpio_level = hal_gpio_get_input_level(pinmux);

        Pad_WakeupPolarityValue(pinmux,
                                gpio_level ? PAD_WAKEUP_POL_LOW : PAD_WAKEUP_POL_HIGH);
    }
}

void app_dlps_restore_pad(uint8_t pinmux)
{
    Pad_ControlSelectValue(pinmux, PAD_PINMUX_MODE);
    System_WakeUpPinDisable(pinmux);

    if (System_WakeUpInterruptValue(pinmux) == 1)
    {
        P_GPIO_CBACK cb = NULL;
        uint32_t context = 0;

        //Edge trigger will mis-detect when wake up
        hal_gpio_get_isr_callback(pinmux, &cb, &context);

        if (cb)
        {
            APP_PRINT_INFO0("app_dlps_restore_pad: cb");
            cb(context);
        }
    }
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

    bt_power_mode_set(BTPOWER_DEEP_SLEEP);
    app_dlps_power_off();
}
