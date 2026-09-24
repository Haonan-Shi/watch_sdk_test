/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "board.h"
#include "trace.h"
#include "pm.h"
#include "platform_utils.h"
#include "os_timer.h"
#include "io_dlps.h"
#include "section.h"
#include "power_test.h"
#include "rtl876x_uart.h"
#include "rtl876x_pinmux.h"
#ifdef CONFIG_SOC_SERIES_RTL87X3G
#include "indirect_access.h"
#endif

#if F_APP_PSRAM_ENABLE
#include "module_psram.h"
#endif

#ifdef CONFIG_SOC_SERIES_RTL8773D
typedef enum
{
    PLATFORM_POWEROFF       = 0,   /**< Power off  */
    PLATFORM_POWERDOWN      = 1,   /**< Power down */
    PLATFORM_DLPS_PFM       = 2,   /**< DLPS (PFM) */
    //PLATFORM_DLPS_RET       = 3,   /**< DLPS (RET) */     // RTL87x3D not support
    PLATFORM_LPS_PFM        = 4,   /**< LPS        */
    PLATFORM_ACTIVE         = 5,   /**< Active     */
    PLATFORM_POWER_MODE_MAX = 6
} PlatformPowerMode;
#else
typedef enum
{
    PLATFORM_POWEROFF                       = 0,   /**< Power off  */
    PLATFORM_POWERDOWN                      = 1,   /**< Power down */
    PLATFORM_DLPS_PFM                       = 2,   /**< DLPS (PFM) */
    PLATFORM_DLPS_RET                       = 3,   /**< DLPS (RET) */
    PLATFORM_LPS_PFM                        = 4,   /**< LPS        */
    PLATFORM_ACTIVE                         = 5,   /**< Active     */
    PLATFORM_POWER_MODE_MAX                 = 6
} PlatformPowerMode;
#endif

static uint32_t dlps_bitmap;                /**< dlps locking bitmap */

extern void platform_pm_set_power_mode(PlatformPowerMode pf_power_mode_user);

/**
* @brief Console uart use cpu mode tx
*
* @param data Pointer to the buffer to be tx
* @param len length of the buffer
* @retval none
*/
void console_uart_direct(uint8_t *data, uint32_t len)
{
    uint32_t blkcount  = len / UART_TX_FIFO_SIZE;
    uint32_t remainder = len % UART_TX_FIFO_SIZE;
    uint32_t i = 0;

    while (UART_GetFlagState(UART0, UART_FLAG_THR_TSR_EMPTY) != SET);
    for (i = 0; i < blkcount; ++i)
    {
        UART_SendData(UART0, data + UART_TX_FIFO_SIZE * i, UART_TX_FIFO_SIZE);
        /* wait tx fifo empty */
        while (UART_GetFlagState(UART0, UART_FLAG_THR_TSR_EMPTY) != SET);
    }

    while (UART_GetFlagState(UART0, UART_FLAG_THR_TSR_EMPTY) != SET);
    /* send left bytes */
    if (remainder)
    {
        UART_SendData(UART0, data + UART_TX_FIFO_SIZE * i, remainder);
        /* wait tx fifo empty */
        while (UART_GetFlagState(UART0, UART_FLAG_THR_TSR_EMPTY) != SET);
    }
}

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
    static uint8_t dlps_console_response = 1;
    POWERMode lps_mode = power_mode_get();

#if (F_APP_PSRAM_ENABLE == 1)
    app_psram_enter_dlps();
#endif

    /* prevent send response when test ble or BREDR*/
    // if (dlps_console_response)
    // {
    //     char buf[15] = {0};
    //     snprintf(buf, 15, "lps_mode %d", lps_mode);
    //     console_uart_direct((uint8_t *)buf, 15);
    //     dlps_console_response = 0;

    //     Pad_PowerOrShutDownValue(P2_1, PAD_SHUTDOWN);
    //     Pad_PowerOrShutDownValue(P2_4, PAD_SHUTDOWN);
    //     Pad_PowerOrShutDownValue(P2_0, PAD_SHUTDOWN);
    //     Pad_PowerOrShutDownValue(P3_0, PAD_SHUTDOWN);
    //     Pad_PowerOrShutDownValue(P3_1, PAD_SHUTDOWN);
    //     UART_INTConfig(UART0, UART_INT_RD_AVA | UART_INT_IDLE, DISABLE);
    // }

    DBG_DIRECT("app_dlps_enter_callback: lps_mode %d", lps_mode);
}

RAM_TEXT_SECTION void app_dlps_exit_callback(void)
{
    APP_PRINT_INFO4("dump aon reg. 0x126 = 0x%x, 0x128 = 0x%x, 0x130 = 0x%x, 0x132 = 0x%x",
                    btaon_fast_read_safe(0x126), btaon_fast_read_safe(0x128), btaon_fast_read_safe(0x130),
                    btaon_fast_read_safe(0x132));

#if (F_APP_PSRAM_ENABLE == 1)
    app_psram_exit_dlps();
#endif
}

void app_dlps_init(void)
{
    // io_dlps_register();

    /* register of call back function */
    power_check_cb_register(app_dlps_check_callback);

    power_stage_cb_register(app_dlps_enter_callback, POWER_STAGE_STORE);
    power_stage_cb_register(app_dlps_exit_callback, POWER_STAGE_RESTORE);

    SYSBLKCTRL->u_208.BITS_208.r_DSP_CLK_SRC_EN = 0;
    bt_power_mode_set(BTPOWER_DEEP_SLEEP);

    /*To aviod enter dlps after boot */
    power_mode_set(POWER_ACTIVE_MODE);

}

void power_test_set_mode(uint16_t action, uint8_t *buf)
{
    if (action < POWER_TEST_CMD_STATE_MAX_INDEX)
    {
        if (action == POWER_TEST_CMD_SET_BT_MAC_ACTIVE)
        {
            APP_PRINT_TRACE0("power test bt active");
            bt_power_mode_set(BTPOWER_ACTIVE);
            return;
        }
#ifdef CONFIG_SOC_SERIES_RTL8773D
        /* open dsp clock, pro3 can't close when enter power down because will access dsp register */
        SYSBLKCTRL->u_208.BITS_208.r_DSP_CLK_SRC_EN = 1;
#endif
        bt_power_mode_set(BTPOWER_DEEP_SLEEP);

        if (action == POWER_TEST_CMD_SET_LPS)
        {
            APP_PRINT_TRACE0("power test set lps");
            power_mode_set(POWER_LPS_MODE);
        }
        else if (action == POWER_TEST_CMD_SET_DLPS)
        {
            APP_PRINT_TRACE0("power test set dlps");
            power_mode_set(POWER_DLPS_MODE);
        }
#ifndef CONFIG_SOC_SERIES_RTL8773D
        else if (action == POWER_TEST_CMD_SET_DLPS_RET)
        {
            APP_PRINT_TRACE0("power test set dlps ret");
            platform_pm_set_power_mode(PLATFORM_DLPS_RET);
        }
#endif
        else if (action == POWER_TEST_CMD_SET_DLPS_PFM)
        {
            APP_PRINT_TRACE0("power test set dlps pfm");
            platform_pm_set_power_mode(PLATFORM_DLPS_PFM);
        }
        else if (action == POWER_TEST_CMD_SET_POWER_DOWN)
        {
            APP_PRINT_TRACE0("power test set power down");
            power_mode_set(POWER_POWERDOWN_MODE);
        }
        else if (action == POWER_TEST_CMD_SET_POWER_OFF)
        {
            APP_PRINT_TRACE0("power test set power off");
            power_mode_set(POWER_SHIP_MODE);
        }
        else if (action == POWER_TEST_CMD_SET_BT_MAC_SELLP)
        {
            APP_PRINT_TRACE0("power test set bt mac sleep");
        }

        if ((action == POWER_TEST_CMD_SET_POWER_DOWN) || (action == POWER_TEST_CMD_SET_POWER_OFF))
        {
            // power_stop_all_non_excluded_timer();
        }
    }
    else
    {
        APP_PRINT_TRACE0("power test mode cmd error");
    }
    if (buf != NULL)
    {
        free(buf);
    }
}
