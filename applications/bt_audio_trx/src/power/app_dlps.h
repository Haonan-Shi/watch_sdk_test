/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_DLPS_H_
#define _APP_DLPS_H_

#include <stdint.h>
#include <stdbool.h>
#include "rtl876x_pinmux.h"
#include "app_msg.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/** @defgroup APP_DLPS App Dlps
  * @brief this file handle device dlps mode related process
  * @{
  */

/*============================================================================*
 *                              Macros
 *============================================================================*/
/** @defgroup APP_DLPS_Exported_Macros App Dlps Macros
   * @{
   */
#define APP_DLPS_ENTER_CHECK_UART_RX            0x00000001
#define APP_DLPS_ENTER_CHECK_UART_TX            0x00000002
#define APP_DLPS_ENTER_CHECK_GPIO               0x00000004
#define APP_DLPS_ENTER_CHECK_DUT_TEST_MODE      0x00000018
#define APP_DLPS_ENTER_CHECK_PLAYBACK           0x00000020
#define APP_DLPS_ENTER_CHECK_USB                0x00000040
#define APP_DLPS_ENTER_CHECK_LINEIN             0x00000080
#define APP_DLPS_ENTER_CHECK_WAIT_RESET         0x00000100
#define APP_DLPS_ENTER_CHECK_OTA_TOOLING_PARK   0x00000200
#define APP_DLPS_ENTER_CHECK_BOX_BAT            0x00000400
#define APP_DLPS_ENTER_CHECK_RF_XTAL            0x00000800
#define APP_DLPS_ENTER_CHECK_SPP_CAPTURE        0x00001000
#define APP_DLPS_ENTER_CHECK_CMD                0x00002000
#define APP_DLPS_ENTER_CHECK_SPI                0x00004000
#define APP_DLPS_ENTER_CHECK_DISPLAY            0x00008000
#define APP_DLPS_ENTER_CHECK_TOUCH              0x00010000
#define APP_DLPS_ENTER_CHECK_XMIT               0x00020000
#define APP_DLPS_ENTER_CHECK_INIT               0x00040000
#define APP_DLPS_ENTER_CHECK_SD_CARD            0x00080000
#define APP_DLPS_ENTER_CHECK_PWM                0x00100000
/** End of APP_DLPS_Exported_Macros
    * @}
    */
/*============================================================================*
 *                              Functions
 *============================================================================*/
/** @defgroup APP_DLPS_Exported_Functions App Dlps Functions
    * @{
    */
/* @brief  enable dlps by clear specific bit mask
*
* @param  bit one bit that show whether dlps is allowed or not by specific function uint
* @return none
*/
void app_dlps_enable(uint32_t bit);

/* @brief  disable dlps by set specific bit mask
*
* @param  bit one bit that show whether dlps is allowed or not by specific function uint
* @return none
*/
void app_dlps_disable(uint32_t bit);

/**
    * @brief  When app is about to enter dlps mode, you need to check via this callback first.
    * @param  void
    * @return bool
    */
bool app_dlps_check_callback(void);

/**
    * @brief   Need to handle message in this callback function,when App enter dlps mode
    * @param  void
    * @return void
    */
void app_dlps_enter_callback(void);

/**
    * @brief  Need to handle message in this callback function,when App exit dlps mode.
    * @param  void
    * @return void
    */
void app_dlps_exit_callback(void);


/**
    * @brief  dlps related process when power off.
    * @param  void
    * @return void
    */
void app_dlps_power_off(void);

/**
    * @brief  stop power down timer and enable already poweroff mask.
    * @param  void
    * @return void
    */
void app_dlps_enable_auto_poweroff_stop_wdg_timer(void);

/**
    * @brief  stop power down timer.
    * @param  void
    * @return void
    */
void app_dlps_stop_power_down_wdg_timer(void);

/**
    * @brief  start power down timer.
    * @param  void
    * @return void
    */
void app_dlps_start_power_down_wdg_timer(void);

/**
    * @brief  check whether do direct power on or not when wakeup from power down mode.
    * @param  void
    * @return bool
    */

void app_dlps_restore_pad(uint8_t pinmux);

/**
    * @brief  invert pad wake up polarity
    * @param  pinmux
    * @return void
    */
void app_dlps_pad_wake_up_polarity_invert(uint8_t pinmux);

/**
    * @brief  dlps module init.
    * @param  void
    * @return void
    */
void app_dlps_init(void);

/**
    * @brief  get dlps bitmap.
    * @param  void
    * @return dlps bitmap
    */
uint32_t app_dlps_get_dlps_bitmap(void);

/**
    * @brief  set power mode.
    * @param  void
    * @return void
    */
void app_dlps_power_mode_set(void);

/**
    * @brief  check dlps bit.
    * @param  uint32_t bits
    * @return bool
    */
bool app_dlps_check_enter_bits(uint32_t bit);


/** @} */ /* End of group APP_DLPS_Exported_Functions */
/** End of APP_DLPS
* @}
*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _APP_DLPS_H_ */
