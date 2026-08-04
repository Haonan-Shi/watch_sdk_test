/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_MODULE_INIT_H_
#define _APP_MODULE_INIT_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @defgroup APP_MODULE_INIT App Module Auto-Initialization
  * @brief Auto-registration mechanism for app modules
  * @{
  */

/*============================================================================*
 *                              Macros
 *============================================================================*/

typedef void (*app_module_init_fn_t)(void);

#define APP_MODULE_INIT_SECTION       __attribute__((section(".app_module_init")))
#define APP_MODULE_INIT_USED          __attribute__((used))
#define APP_MODULE_INIT_ALIGN         __attribute__((aligned(sizeof(void *))))

#define APP_MODULE_INIT(fn) \
    APP_MODULE_INIT_USED const app_module_init_fn_t _app_module_init_##fn \
    APP_MODULE_INIT_SECTION APP_MODULE_INIT_ALIGN = fn


/*============================================================================*
 *                              Functions
 *============================================================================*/

/*============================================================================*
 *                              Functions
 *============================================================================*/

/**
 * @brief  Initialize all registered app modules
 * @note   This function iterates through all entries in .app_module_init
 *         section and calls each initialization function
 * @return void
 */
void app_module_init_all(void);

/**
 * @brief  Get the count of registered modules (for debug)
 * @return number of registered modules
 */
uint32_t app_module_get_count(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _APP_MODULE_INIT_H_ */