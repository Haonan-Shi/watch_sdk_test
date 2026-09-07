/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_BLE_SERVICE_H_
#define _APP_BLE_SERVICE_H_

#include <stdint.h>
#if F_APP_BLE_HID_DEVICE_SUPPORT
#include "profile_server.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @defgroup APP_BLE_SERVICE App Ble Service
  * @brief App Ble Service
  * @{
  */

enum
{
    BLE_SEND_OK       = 0x00,
    BLE_NOT_CONN      = 0x01,
    BLE_NOT_READY     = 0x02,
    BLE_SEND_FAIL     = 0x03,
};

/*============================================================================*
 *                              Macros
 *============================================================================*/
/** @defgroup APP_BLE_SERVICE_Exported_Macros App Ble Service Macros
   * @{
   */
#define TX_ENABLE_CCCD_BIT                  0x01
#define TX_ENABLE_AUTHEN_BIT                0x02
#define TX_ENABLE_READY                     (TX_ENABLE_CCCD_BIT|TX_ENABLE_AUTHEN_BIT)
/** End of APP_BLE_SERVICE_Exported_Macros
    * @}
    */

/*============================================================================*
 *                              Functions
 *============================================================================*/
/** @defgroup APP_BLE_SERVICE_Exported_Functions App Ble Service Functions
    * @{
    */

/**
    * @brief  Ble Service transfer
    * @param  data ptr and len
    * @return send result
    */
uint8_t app_ble_service_transfer(uint8_t app_idx, uint8_t *data, uint32_t len);

uint8_t app_gatt_over_bredr_transfer(uint8_t app_idx, uint8_t *data, uint32_t len);

#if F_APP_BLE_HID_DEVICE_SUPPORT
uint16_t app_ble_service_hid_get_conn_handle(void);

bool app_ble_service_is_hid_service(T_SERVER_ID service_id);
#endif

/**
    * @brief  Ble Service module init
    * @param  void
    * @return void
    */
void app_ble_service_init(void);

/** @} */ /* End of group APP_BLE_SERVICE_Exported_Functions */

/** End of APP_BLE_SERVICE
* @}
*/



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _APP_BLE_SERVICE_H_ */
