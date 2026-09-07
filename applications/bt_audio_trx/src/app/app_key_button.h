/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_KEY_BUTTON_H_
#define _APP_KEY_BUTTON_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum
{
    KEY_ACTION_NONE,
    KEY_ACTION_SINGLE_CLICK,
    KEY_ACTION_LONG_PRESS,
    KEY_ACTION_VERY_LONG_PRESS,
    KEY_ACTION_ULTRA_LONG_PRESS,
} KEY_ACTION;

void app_key_gpio_button_init(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _APP_IPC_H_ */
