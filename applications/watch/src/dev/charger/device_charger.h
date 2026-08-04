/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _DEVICE_CHARGER_H_
#define _DEVICE_CHARGER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "app_msg.h"
#include "charger_api.h"

void device_charger_update_battery_state(void);
T_CHARGER_ERROR_CODE device_charger_read_error_code(void);
void device_charger_init(void);


#ifdef __cplusplus
}
#endif

#endif /* _DEVICE_CHARGER_H_ */
