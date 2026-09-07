/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef fwus_h
#define fwus_h

#include "profile_server_ext.h"
#include "fmna_constants_platform.h"

#define GATT_UUID128_DATA_CTRL_POINT                    0xDE, 0xB0, 0x01, 0x7F, 0x4A, 0x6A, 0xF1, 0xA4, 0x25, 0x42, 0x9B, 0x6D, 0x01, 0x00, 0x11, 0x94

#define FMNA_FWUS_DATA_CTRL_INDEX                       2
#define FMNA_FWUS_DATA_CTRL_CCCD_INDEX                  FMNA_FWUS_DATA_CTRL_INDEX + 1

typedef struct
{
    uint8_t opcode;
    T_WRITE_TYPE write_type;
    uint16_t len;
    uint8_t *p_value;
} T_FWUS_WRITE_MSG;

typedef struct
{
    uint8_t notification_indification_index;
    uint8_t cccbits;
} T_FWUS_NOTIFY_MSG;

typedef union
{
    T_FWUS_NOTIFY_MSG nofity_indicate_update;
    T_FWUS_WRITE_MSG write;
} T_FWUS_UPSTREAM_MSG_DATA;

typedef struct
{
    uint8_t                 conn_id;
    uint16_t                conn_handle;
    uint16_t                cid;

    T_SERVICE_CALLBACK_TYPE msg_type;
    T_FWUS_UPSTREAM_MSG_DATA msg_data;
} T_FWUS_CALLBACK_DATA;

T_SERVER_ID firmware_update_add_service(void *p_func);

#endif
