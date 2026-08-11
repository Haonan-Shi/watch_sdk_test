/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef fns_h
#define fns_h

#include "bt_gatt_svc.h"
#include "fmna_constants_platform.h"

#define GATT_UUID128_PAIR_CTRL_POINT                   0x7A, 0x42, 0x04, 0x03, 0x73, 0x2F, 0xD4, 0xBE, 0xEF, 0x49, 0x3B, 0x94, 0x01, 0x00, 0x86, 0x4F
#define GATT_UUID128_CONF_CTRL_POINT                   0x7A, 0x42, 0x04, 0x03, 0x73, 0x2F, 0xD4, 0xBE, 0xEF, 0x49, 0x3B, 0x94, 0x02, 0x00, 0x86, 0x4F
#define GATT_UUID128_NON_OWNER_CTRL_POINT              0x7A, 0x42, 0x04, 0x03, 0x73, 0x2F, 0xD4, 0xBE, 0xEF, 0x49, 0x3B, 0x94, 0x03, 0x00, 0x86, 0x4F
#define GATT_UUID128_PAIRED_OWNER_INFO_CTRL_POINT      0x7A, 0x42, 0x04, 0x03, 0x73, 0x2F, 0xD4, 0xBE, 0xEF, 0x49, 0x3B, 0x94, 0x04, 0x00, 0x86, 0x4F

#define FMNA_FNS_PAIRING_CP_INDEX                       2
#define FMNA_FNS_CONFIG_CP_INDEX                        5
#define FMNA_FNS_NON_OWNER_CP_INDEX                     8
#define FMNA_FNS_PAIRED_OWNER_CP_INDEX                  11

#define FMNA_FNS_PAIRING_CP_CCCD_INDEX                  FMNA_FNS_PAIRING_CP_INDEX + 1
#define FMNA_FNS_CONFIG_CP_CCCD_INDEX                   FMNA_FNS_CONFIG_CP_INDEX + 1
#define FMNA_FNS_NON_OWNER_CP_CCCD_INDEX                FMNA_FNS_NON_OWNER_CP_INDEX + 1
#define FMNA_FNS_PAIRED_OWNER_CP_CCCD_INDEX             FMNA_FNS_PAIRED_OWNER_CP_INDEX + 1

#ifdef DEBUG
#define GATT_UUID128_DEBUG_CTRL_POINT                  0x7A, 0x42, 0x04, 0x03, 0x73, 0x2F, 0xD4, 0xBE, 0xEF, 0x49, 0x3B, 0x94, 0x05, 0x00, 0x86, 0x4F
#define FMNA_FNS_DEBUG_CP_INDEX                         14
#define FMNA_DEBUG_CP_CCCD_INDEX                        FMNA_FNS_DEBUG_CP_INDEX + 1
#endif //DEBUG

#define GATT_MSG_FNS_SERVER_READ           0x00
#define GATT_MSG_FNS_SERVER_WRTIE          0x01
#define GATT_MSG_FNS_SERVER_CCCD           0x02

typedef struct
{
    uint8_t opcode;
    T_WRITE_TYPE write_type;
    uint16_t len;
    uint8_t *p_value;
} TFNS_WRITE_MSG;

typedef struct
{
    uint8_t notification_indification_index;
    uint8_t cccbits;
} TFNS_NOTIFY_MSG;

typedef union
{
    TFNS_NOTIFY_MSG nofity_indicate_update;
    uint8_t read_value_index;
    TFNS_WRITE_MSG write;
} TFNS_UPSTREAM_MSG_DATA;

typedef struct
{
    uint16_t                conn_handle;
    uint8_t                 conn_id;
    T_SERVER_ID             service_id;
    TFNS_UPSTREAM_MSG_DATA  msg_data;
} T_FNS_CALLBACK_DATA;

typedef uint16_t (*P_FUN_FNS_SERVER_APP_CB)(uint8_t type, void *p_data);

T_SERVER_ID fns_reg_srv(P_FUN_FNS_SERVER_APP_CB app_cb, P_FUN_GATT_EXT_SEND_DATA_CB send_cb);

#endif
