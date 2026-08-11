/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_LE_THROUGHPUT_H_
#define _APP_LE_THROUGHPUT_H_

#include <stdint.h>
#include "bt_gatt_svc.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup APP_LE_THROUGHPUT App LE Throughput Test Handler
  * @brief Application-level handler for the LE Throughput Test Service.
  * @{
  */

/*============================================================================*
 *                              Variables
 *============================================================================*/
extern T_SERVER_ID le_throughput_gatt_srv_id;

/*============================================================================*
 *                              Functions
 *============================================================================*/

/**
 * @brief Initialize the LE Throughput Test Service at the application level.
 *
 * Registers the service with the BLE stack and sets up the application
 * callback for handling test control signals and data.
 */
void app_le_throughput_init(void);

/** @} */ /* End of group APP_LE_THROUGHPUT */

#ifdef __cplusplus
}
#endif

#endif /* _APP_LE_THROUGHPUT_H_ */
