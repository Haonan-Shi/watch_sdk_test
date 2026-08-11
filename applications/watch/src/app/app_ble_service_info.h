/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_BLE_SERVICE_INFO_H_
#define _APP_BLE_SERVICE_INFO_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup APP_BLE_SERVICE_INFO BLE Service Auto-Registration
 *  @brief Auto-registration mechanism for BLE services
 *  @{
 */

/*============================================================================*
 *                              Macros
 *============================================================================*/

typedef struct
{
    const char *module_name;
    uint8_t     service_count;
} app_ble_service_info_t;

#define APP_BLE_SERVICE_INFO_SECTION   __attribute__((section(".app_ble_service_info")))
#define APP_BLE_SERVICE_INFO_USED      __attribute__((used))
#define APP_BLE_SERVICE_INFO_ALIGN     __attribute__((aligned(sizeof(void *))))

#define APP_BLE_SERVICE_INFO(_name, _count) \
    APP_BLE_SERVICE_INFO_USED const app_ble_service_info_t _app_ble_service_##_name \
    APP_BLE_SERVICE_INFO_SECTION APP_BLE_SERVICE_INFO_ALIGN = { #_name, _count }

/*============================================================================*
 *                              Functions
 *============================================================================*/

/**
 * @brief  Get total BLE service count from registered modules
 * @return total number of services registered via APP_BLE_SERVICE_INFO
 */
uint32_t app_ble_service_get_total(void);

/**
 * @brief  Print all registered BLE services (for debug)
 * @return void
 */
void app_ble_service_print_all(void);

#ifdef __cplusplus
}
#endif

#endif /* _APP_BLE_SERVICE_INFO_H_ */