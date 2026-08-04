/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_ble_service_info.h"
#include <stddef.h>
#include "trace.h"

/* Define section boundaries */
static const app_ble_service_info_t __app_ble_service_info_start[]
__attribute__((section(".app_ble_service_info.start")))
__attribute__((used)) = { 0 };

static const app_ble_service_info_t __app_ble_service_info_end[]
__attribute__((section(".app_ble_service_info.end")))
__attribute__((used)) = { 0 };

/**
 * @brief  Get total BLE service count from registered modules
 */
uint32_t app_ble_service_get_total(void)
{
    uint32_t total = 0;
    const app_ble_service_info_t *p = __app_ble_service_info_start + 1;
    const app_ble_service_info_t *end = __app_ble_service_info_end;

    // cppcheck-suppress comparePointers
    for (; p < end; p++)
    {
        if (p->module_name != NULL)
        {
            total += p->service_count;
        }
    }

    APP_PRINT_INFO1("[BLE Service] Total registered services: %d", total);
    return total;
}

/**
 * @brief  Print all registered BLE services (for debug)
 */
void app_ble_service_print_all(void)
{
    const app_ble_service_info_t *p = __app_ble_service_info_start + 1;
    const app_ble_service_info_t *end = __app_ble_service_info_end;

    APP_PRINT_INFO0("[BLE Service] Registered services:");
    // cppcheck-suppress comparePointers
    for (; p < end; p++)
    {
        if (p->module_name != NULL)
        {
            APP_PRINT_INFO2("[BLE Service]   - %s: %d services",
                            p->module_name, p->service_count);
        }
    }
}