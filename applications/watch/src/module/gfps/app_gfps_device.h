/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_GFPS_DEVICE_H_
#define _APP_GFPS_DEVICE_H_

#include "remote.h"
#include "btm.h"
#if CONFIG_REALTEK_GFPS_FEATURE_SUPPORT
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @defgroup APP_RWS_GFPS App Gfps
  * @brief App Gfps
  * @{
  */

/**
 * @brief app_gfps_device_handle_factory_reset
 *
 */
void app_gfps_device_handle_factory_reset(void);

/**
 * @brief app_gfps_device_handle_power_off
 *
 */
void app_gfps_device_handle_power_off(void);

/**
 * @brief app_gfps_device_handle_power_on
 *
 * @param is_pairing true: in pairing mode, false: not in pairing mode
 */
void app_gfps_device_handle_power_on(bool is_pairing);

/**
 * @brief app_gfps_device_handle_ble_link_disconnected
 *
 * @param local_disc_cause
 */
void app_gfps_device_handle_ble_link_disconnected(uint8_t local_disc_cause);
/** End of APP_RWS_GFPS
* @}
*/
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
#endif
