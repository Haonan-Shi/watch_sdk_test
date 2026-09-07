/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef fns_h
#define fns_h

#include "profile_server_ext.h"
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
    uint8_t                 conn_id;
    uint16_t                conn_handle;
    uint16_t                cid;

    T_SERVICE_CALLBACK_TYPE msg_type;
    TFNS_UPSTREAM_MSG_DATA  msg_data;
} T_FNS_CALLBACK_DATA;

T_SERVER_ID findmy_network_add_service(void *p_func);

#endif
