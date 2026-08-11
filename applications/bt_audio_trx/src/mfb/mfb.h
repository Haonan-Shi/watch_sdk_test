/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_MFB_H_
#define _APP_MFB_H_

#include <stdbool.h>
#include <stdint.h>
#include "key_button_8773g_zephyr.h"

void app_mfb_init(void);
T_GPIO_KEY app_mfb_get_level(void);
void app_mfb_register_callback(T_GPIO_KEY_CALLBACK user_callback);
#endif /* _APP_MFB_H_ */
