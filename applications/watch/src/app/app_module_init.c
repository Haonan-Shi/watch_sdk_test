/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_module_init.h"
#include "trace.h"

/* Define section boundaries - in separate sections to ensure correct ordering */
static const app_module_init_fn_t __app_module_init_start[]
__attribute__((section(".app_module_init.start")))
__attribute__((used)) = { 0 };

static const app_module_init_fn_t __app_module_init_end[]
__attribute__((section(".app_module_init.end")))
__attribute__((used)) = { 0 };

/**
 * @brief  Initialize all registered app modules
 */
void app_module_init_all(void)
{
    volatile const app_module_init_fn_t *p = __app_module_init_start + 1;
    volatile const app_module_init_fn_t *end = __app_module_init_end;

    APP_PRINT_INFO0("[ModuleInit] Starting app modules initialization");
    APP_PRINT_INFO1("[ModuleInit] __app_module_init_start: 0x%x", __app_module_init_start);
    APP_PRINT_INFO1("[ModuleInit] __app_module_init_end: 0x%x", __app_module_init_end);
    // cppcheck-suppress comparePointers
    for (; p < end; p++)
    {
        if (*p)
        {
            APP_PRINT_INFO1("[ModuleInit] Calling init function: 0x%x", *p);
            (*p)();
        }
    }
    APP_PRINT_INFO0("[ModuleInit] All app modules initialized");
}

uint32_t app_module_get_count(void)
{
    // cppcheck-suppress comparePointers
    return (__app_module_init_end - __app_module_init_start - 1);
}
