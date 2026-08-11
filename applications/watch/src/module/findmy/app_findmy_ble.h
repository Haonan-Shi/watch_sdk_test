/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_FINDMY_BLE_H_
#define _APP_FINDMY_BLE_H_

#include "fmna_constants.h"
#include "fmna_adv_platform.h"
#include "app_ble_gap.h"

#define APP_FINDMY_FAST_ADV_TIMEOUT  20000
#define APP_FINDMY_SLOW_ADV_TIMEOUT  0

extern T_LE_ADV_CONN le_findmy_adv;

bool app_findmy_is_findmy_link(uint8_t conn_id);

bool app_findmy_is_findmy_adv(void);

void app_findmy_disconnect(void);

bool app_findmy_adv_start(uint16_t timeout_sec);

bool app_findmy_adv_stop(uint8_t app_cause);

void app_findmy_enter_pair_mode(void);

uint8_t app_findmy_adv_get_adv_handle(void);

void app_findmy_adv_init(void);

#endif /* app_findmy_ble_h */
