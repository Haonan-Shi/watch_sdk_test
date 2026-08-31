/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_KEY_DETECT_H_
#define _APP_KEY_DETECT_H_

#include <stdint.h>
#include <stdbool.h>

void app_key_detect_init(void);
void app_key_detect_poll(void);
void app_key_detect_host_connected(void);
void app_key_detect_host_disconnected(void);
bool app_key_detect_get_host_status(void);
#endif /* _APP_KEY_DETECT_H_ */
