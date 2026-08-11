/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _DFU_MAIN_H_
#define  _DFU_MAIN_H_

#include <stdint.h>
#include <stdbool.h>
#include "bt_gatt_svc.h"

/* when start dfu will modify the value depend on dfu image total length.
So this value is only need set based on the image size of 100KB */
#define NORMAL_OTA_TIMEOUT_TOTAL               240  //uint: second
//This value won't be modofy
#define NORMAL_OTA_TIMEOUT_WAIT4_CONN          60   //uint: second

extern bool is_normal_ota_mode;
extern void *normal_ota_total_timer_handle;
extern void *normal_ota_wait4_conn_timer_handle;

extern T_SERVER_ID rtk_dfu_service_id;

void dfu_timer_init(void);

void dfu_set_rand_addr(void);

void dfu_ble_init(void);

void dfu_main(void);

#endif
