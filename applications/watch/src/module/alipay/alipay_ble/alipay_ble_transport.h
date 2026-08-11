/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _ALIPAY_BLE_TRANSPORT_H_
#define _ALIPAY_BLE_TRANSPORT_H_

#include "stdint.h"

typedef enum
{
    TCP_STATUS_DISABLE = 0,
    TCP_STATUS_ENABLE  = 1,
} tcp_status_;

typedef enum
{
    KEY_TCP_IP_ADDR = 1,
    KEY_TCP_CONNECT_EVENT = 2,
    KEY_TCP_LINK_CLOSE = 3,
    KEY_TCP_LINK_CLOSE_RET = 4,
    KEY_TCP_DNS_REQ = 5,
    KEY_TCP_DNS_RSP = 6,
    KEY_TCP_LINK_STATUS = 7,
    KEY_TCP_DATA_WRITE = 0x0A,

    KEY_TCP_DATA_AVALIABLE = 0x0B,
    KEY_TCP_DATA_READ_REQ = 0x0C,
    KEY_TCP_DATA_READ_RSP = 0x0D,
} tcp_sub_code;

void alipay_ble_send(uint16_t handle, uint8_t *data, uint16_t len);
void alipay_ble_send_completed_proc(uint8_t connect_id);
uint8_t alipay_tcp_data_rev(uint8_t key, uint8_t *p_data, uint16_t length);
tcp_status_ alipay_ble_get_tcp_agent_status(void);

#endif
