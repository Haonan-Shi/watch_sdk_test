/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_BLE_ADV_H_
#define _APP_BLE_ADV_H_

#include "rtl876x.h"
#include "app_ble_gap.h"

extern T_LE_ADV_CONN le_common_adv;

/**
    * @brief  start ble common advertising
    * @param  duration_ms advertising duration time
    * @return true  Command has been sent successfully.
    * @return false Command was fail to send.
    */
bool app_ble_common_adv_start(uint16_t duration_10ms);

/**
    * @brief  stop ble common advertising
    * @param  app_cause cause
    * @return true  Command has been sent successfully.
    * @return false Command was fail to send.
    */
bool app_ble_common_adv_stop(int8_t app_cause);

/**
    * @brief  init ble common advertising parameters
    * @param  void
    * @return void
    */
void app_ble_common_adv_set_param(void);

#endif /* _APP_BLE_ADV_H_ */
