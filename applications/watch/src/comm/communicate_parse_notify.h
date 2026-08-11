/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef __COMMUNICATE_PARSE_NOTIFY_H__
#define __COMMUNICATE_PARSE_NOTIFY_H__


#ifdef __cplusplus
extern "C" {
#endif



#include <stdbool.h>
#include "stdint.h"

typedef enum
{
    KEY_INCOMMING_CALL               = 0x01,
    KEY_INCOMMING_CALL_ACCEPT        = 0x02,
    KEY_INCOMMING_CALL_REFUSE        = 0x03,
    KEY_INCOMMING_MESSAGE            = 0x04,
    KEY_INCOMMING_CALL_REJECT        = 0X05,
    KEY_BATTERY_CHARGE_STATUS        = 0X06,

    KEY_INCOMMING_CALL_ID            = 0x07,
    KEY_RTL87x3B_MAC_ADDRESS_RETURN     = 0X10,
    KEY_RTL87x3B_STATE_RETURN           = 0X11,
    KEY_ANCS_INCOMMING_CALL_RETURN   = 0X12,
    KEY_RTL87x3B_CONN_INFO_RETURN       = 0X13,
} NOTIFY_KEY;

void resolve_Notify_command(uint8_t key, uint8_t *pValue, uint16_t length);



#ifdef __cplusplus
}
#endif

#endif //__COMMUNICATE_PARSE_NOTIFY_H__
