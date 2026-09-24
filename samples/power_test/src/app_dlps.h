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

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @defgroup APP_DLPS App Dlps
  * @brief handle device dlps mode related process
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
#define APP_DLPS_ENTER_CHECK_APP                0x00000008
#define APP_DLPS_ENTER_CHECK_MFB_KEY            0x00000010
#define APP_DLPS_ENTER_CHECK_RSV1               0x00000020
#define APP_DLPS_ENTER_CHECK_RSV2               0x00000040
#define APP_DLPS_ENTER_CHECK_LED                0x00000080
#define APP_DLPS_ENTER_CHECK_ADAPTOR            0x00000100
#define APP_DLPS_ENTER_CHECK_QDEC               0x00000200
#define APP_DLPS_ENTER_CHECK_RSV3               0x00000400
#define APP_DLPS_ENTER_CHECK_PLAYBACK           0x00000800
#define APP_DLPS_ENTER_CHECK_USB                0x00001000
#define APP_DLPS_ENTER_CHECK_LINEIN             0x00002000
#define APP_DLPS_ENTER_CHECK_ADP_VOLTAGE        0x00004000
#define APP_DLPS_ENTER_CHECK_WAIT_RESET         0x00008000
/** End of APP_DLPS_Exported_Macros
    * @}
    */

/**
    * @brief  enable dlps by clearing a specific bit in the locking bitmap
    * @param  bit lock bit
    * @return none
    */
void app_dlps_enable(uint32_t bit);

/**
    * @brief  disable dlps by setting a specific bit in the locking bitmap
    * @param  bit lock bit
    * @return none
    */
void app_dlps_disable(uint32_t bit);

/**
    * @brief  dlps enter check callback
    * @return true if allowed to enter dlps
    */
bool app_dlps_check_callback(void);

/**
    * @brief  dlps enter callback
    * @return none
    */
void app_dlps_enter_callback(void);

/**
    * @brief  dlps exit callback
    * @return none
    */
void app_dlps_exit_callback(void);

/**
    * @brief  dlps module init
    * @return none
    */
void app_dlps_init(void);

/**
    * @brief  set the platform power mode according to a console action
    * @param  action power-mode action id
    * @param  buf    message buffer to free after handling
    * @return none
    */
void power_test_set_mode(uint16_t action, uint8_t *buf);

/** End of APP_DLPS
* @}
*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _APP_DLPS_H_ */
