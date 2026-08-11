/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_CONSOLE_H_
#define _APP_CONSOLE_H_

#include <stdint.h>
#include <stdbool.h>
#include "app_msg.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/** @defgroup APP_CONSOLE_UART APP CONSOLE UART.
  * @brief app console uart event handle and implementation
  * @{
  */

/*============================================================================*
 *                              Macros
 *============================================================================*/
/** @defgroup APP_CONSOLE_UART_Exported_Macros App Console UART Macros
    * @{
    */

/** End of APP_CONSOLE_UART_Exported_Macros
    * @}
    */

/*============================================================================*
 *                              Types
 *============================================================================*/
/** @defgroup APP_CONSOLE_UART_Exported_Types App Console UART Types
    * @{
    */

/**  @brief  App define global app data structure */

/** End of APP_CONSOLE_UART_Exported_Types
    * @}
    */

/*============================================================================*
 *                              Variables
 *============================================================================*/
/** @defgroup APP_CONSOLE_UART_Exported_Variables App Console UART Variables
    * @{
    */


/** End of APP_CONSOLE_UART_Exported_Variables
    * @}
    */

/*============================================================================*
 *                              Functions
 *============================================================================*/
/** @defgroup APP_CONSOLE_UART_Exported_Variables App Console UART Functions
    * @{
    */
void app_console_init(void);

bool app_console_send_msg(T_IO_CONSOLE subtype, void *param_buf);

/** End of APP_CONSOLE_UART_Exported_Variables
    * @}
    */

/** End of APP_CONSOLE_UART
* @}
*/


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _APP_CONSOLE_H_ */
