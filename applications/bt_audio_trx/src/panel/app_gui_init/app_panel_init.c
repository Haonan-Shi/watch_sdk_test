/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if CONFIG_REALTEK_APP_GUI
#include "app_gui.h"
#include "section.h"
#include "io_dlps.h"
#include "rtl876x.h"
#include "os_sched.h"
#include "trace.h"
#include "trace.h"
#include "fmc_api.h"
#include "fmc_api_ext.h"
#include "feature_check.h"
#include "app_panel_init.h"
#include "gui_server.h"
#include "app_gui_bt_policy.h"
#include "fmc_api.h"
#include "pm.h"
#include "app_module_psram.h"

#define DASHBOARD_SPIC0_FREQ_MHZ           (160)

#if F_GUI_BENCHMARK_SUPPORT
#include "benchmark_common.h"
#endif

uint32_t app_panel_get_cpu_freq(void)
{
#if TARGET_RTL8763EWE_VP || TARGET_RTL8763EW_VC || TARGET_RTL8763EWE
    return 100;
#elif  CONFIG_SOC_SERIES_RTL8773D
    return 160;
#elif CONFIG_SOC_SERIES_RTL87X3G
    return 200;
#else
    return 100;
#endif
}

void app_flash_resource_init(void)
{
    /*flash set*/
#if (CONFIG_SOC_SERIES_RTL8773E == 1)
    bool ret = fmc_flash_try_high_speed_mode(FMC_SPIC_ID_0, FMC_FLASH_NOR_4_BIT_DTR_MODE);
#else
    bool ret = fmc_flash_try_high_speed_mode(FMC_SPIC_ID_0, FMC_FLASH_NOR_4_BIT_MODE);
#endif
    APP_PRINT_INFO1("app_flash_resource_init: set flash mode result %d", ret);

    fmc_flash_set_seq_trans(FMC_SPIC_ID_0, true);

    uint32_t spic0_freq = 0;
    ret = fmc_flash_nor_clock_switch(FMC_SPIC_ID_0, DASHBOARD_SPIC0_FREQ_MHZ, &spic0_freq);
    APP_PRINT_INFO1("app_flash_resource_init: set flash clock result %d", ret);
}

bool app_psram_check_support(void)
{
    uint8_t pkg_id = feature_check_get_pkg_id();
    APP_PRINT_INFO1("app_psram_check_support: pkg_id %d", pkg_id);
#if CONFIG_SOC_SERIES_RTL8773D
    return false;
#elif CONFIG_SOC_SERIES_RTL8773E
    if (pkg_id == 0x02 || pkg_id == 0x04) // refer to platform_check.h   8773ewe-vp  8773ewp
    {
        return true;
    }
    else
    {
        return false;
    }
#else
    if (pkg_id == 0x11)  // refer to platform_check.h   8763ewe-vp
    {
        return true;
    }
    else
    {
        return false;
    }
#endif
}

void app_psram_resource_init(void)
{
    /*psram set*/
    if (app_psram_check_support() == true)
    {
        APP_PRINT_INFO0("app_psram_resource_init: IC support psram, enable init process");
        app_psram_init();
    }
}
#if 0
bool app_components_init(void)
{
    bool gui_check_flag = false;
    touch_pin_config(TOUCH_I2C_SCL, TOUCH_I2C_SDA, TOUCH_INT, TOUCH_RST);
    gui_check_flag = drv_touch_init();
    if (gui_check_flag == false)
    {
        APP_PRINT_ERROR0("app_components_init: gui check failed");
        return false;
    }

#if (LCD_INTERFACE == LCD_INTERFACE_QSPI)
    lcd_pin_config(LCD_RST);
#if (TARGET_LCD_DEVICE == LCD_DEVICE_ST77916)
    lcd_bl_pin_config(LCD_BL);
#endif
#elif (LCD_INTERFACE == LCD_INTERFACE_8080)
    lcd_pin_config(LCD_RST, LCD_POWER_EN, LCD_8080_BL);
#elif (LCD_INTERFACE == LCD_INTERFACE_LCDC_QSPI)
    lcd_pin_config(LCD_RST);
#endif
#if (TARGET_LCD_DEVICE == LCD_DEVICE_ST7801)
    lcd_vci_en_pin_config(VCI_EN);
#elif (TARGET_LCD_DEVICE == LCD_DEVICE_SH8601Z)
    lcd_avdd_en_pin_config(LCD_AVDD_EN);
#endif
#if (ENABLE_TE_FOR_LCD == 1)
    lcd_te_pin_config(LCD_TE);
#endif
    drv_lcd_init();
    return true;
}
#endif

void app_gui_init(void)
{
    APP_PRINT_INFO0("app_gui_init: APP GUI init");
    bt_mgr_cback_register(app_gui_bt_policy_cback);
    gui_server_init();
}

#endif


/*-----------------------------------------------------------*/
