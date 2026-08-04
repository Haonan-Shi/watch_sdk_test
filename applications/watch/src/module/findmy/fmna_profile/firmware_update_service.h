/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef fwus_h
#define fwus_h

#include "bt_gatt_svc.h"
#include "fmna_constants_platform.h"

#define GATT_UUID128_DATA_CTRL_POINT                    0xDE, 0xB0, 0x01, 0x7F, 0x4A, 0x6A, 0xF1, 0xA4, 0x25, 0x42, 0x9B, 0x6D, 0x01, 0x00, 0x11, 0x94

#define FMNA_FWUS_DATA_CTRL_INDEX                       2
#define FMNA_FWUS_DATA_CTRL_CCCD_INDEX                  FMNA_FWUS_DATA_CTRL_INDEX + 1

#define GATT_MSG_FWUS_SERVER_READ           0x00
#define GATT_MSG_FWUS_SERVER_WRTIE          0x01
#define GATT_MSG_FWUS_SERVER_CCCD           0x02

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
    uint16_t                conn_handle;
    uint8_t                 conn_id;
    T_SERVER_ID             service_id;
    uint16_t                cid;
    T_FWUS_UPSTREAM_MSG_DATA msg_data;
} T_FWUS_CALLBACK_DATA;

typedef uint16_t (*P_FUN_FWUS_SERVER_APP_CB)(uint8_t type, void *p_data);

T_SERVER_ID fwus_reg_srv(P_FUN_FWUS_SERVER_APP_CB app_cb, P_FUN_GATT_EXT_SEND_DATA_CB send_cb);

#endif
