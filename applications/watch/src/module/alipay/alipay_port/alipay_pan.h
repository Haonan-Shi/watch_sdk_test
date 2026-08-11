/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _ALIPAY_PAN_H_
#define _ALIPAY_PAN_H_

#include "bt_pan.h"

typedef enum
{
    BT_STATUS_DISCONNECT,
    BT_STATUS_CONNECTED,
} e_pan_status;

void alipay_pan_init(void);
bool alipay_bt_pan_connect(void);
void app_gap_bt_cback_pan_handler(T_BT_PAN_EVENT event_type, void *event_buf, uint16_t buf_len);

e_pan_status alipay_get_network_status(void);

#endif //_ALIPAY_PAN_H_
