/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _WALKIE_TALKIE_ADV_H_
#define _WALKIE_TALKIE_ADV_H_

#include "rtl876x.h"
#include "app_ble_gap.h"

#define WALKIE_TALKIE_ADV_LEN                       206
#define MANUFACTURE_LEN_OFFSET                      18
#define USER_NAME_OFFSET                            26
#define USER_NAME_LEN                               11
#define VOICE_NUM_OFFSET                            37
#define VOICE_DATA_OFFSET                           38

extern T_LE_ADV_CONN transmit_adv;
extern uint8_t transmit_adv_data[WALKIE_TALKIE_ADV_LEN];

bool walkie_talkie_transmitter_working(void);
bool walkie_talkie_adv_start(uint16_t duration_10ms);
bool walkie_talkie_adv_stop(int8_t app_cause);
void walkie_talkie_adv_set_param(void);
T_GAP_CAUSE walkie_talkie_disconnect(void);

#endif /* _WALKIE_TALKIE_ADV_H_ */
