/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_PERIPHERAL_GAP__
#define _APP_PERIPHERAL_GAP__

#ifdef __cplusplus
extern "C" {
#endif
/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <app_msg.h>
#include <profile_server.h>

/** @defgroup PERIPH_APP Peripheral Application
  * @brief Peripheral Application
  * @{
  */

/*============================================================================*
 *                              Variables
 *============================================================================*/


/*============================================================================*
 *                              Functions
 *============================================================================*/
void app_peripheral_gap_handle_conn_param_update_evt(uint8_t conn_id, uint8_t status,
                                                     uint16_t cause);

/** End of PERIPH_APP
* @}
*/

#ifdef __cplusplus
}
#endif

#endif
