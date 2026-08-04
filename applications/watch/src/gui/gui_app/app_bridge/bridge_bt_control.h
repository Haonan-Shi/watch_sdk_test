/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *               Define to prevent recursive inclusion
 *============================================================================*/
#ifndef _BRIDGE_BT_CONTROL_H_
#define _BRIDGE_BT_CONTROL_H_

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include <stdbool.h>
#include <stdint.h>

/*============================================================================*
 *                         Macros
 *============================================================================*/
#define BT_CONTROL_MAX_BONDED      8
/* UTF-16 char count, must match MAX_DEVICE_NAME_NUM in app_bond.h to allow
 * 1:1 device_name copy from T_APP_BOND_DEVICE. */
#define BT_CONTROL_NAME_MAX_CHARS  20

/* GUI topic - kept as legacy strings to match HoneyGUI-auto-generated
 * gui_msg_subscribe() calls in app_control_center_ui.c. */
#define GUI_TOPIC_BT_PHONE_CONN          "bt/phone_conn"
#define GUI_TOPIC_BT_PHONE_DISCONN       "bt/phone_disconn"
#define GUI_TOPIC_BT_HEADPHONE_CONN      "bt/headphone_conn"
#define GUI_TOPIC_BT_HEADPHONE_DISCONN   "bt/headphone_disconn"
#define GUI_TOPIC_BT_INQUIRY_RESULT      "bt/inquiry_result"
#define GUI_TOPIC_BT_INQUIRY_CMPL        "bt/inquiry_cmpl"

/* GUI -> Bridge command/request namespace */
#define EVENT_BUS_TOPIC_BT_CONTROL_ALL_TOPIC          "bt_control/*"
#define EVENT_BUS_TOPIC_BT_CONTROL_REQ_STATE          "bt_control/req/state"
#define EVENT_BUS_TOPIC_BT_CONTROL_CMD_TOGGLE         "bt_control/cmd/toggle"
#define EVENT_BUS_TOPIC_BT_CONTROL_CMD_INQUIRY_START  "bt_control/cmd/inquiry_start"
#define EVENT_BUS_TOPIC_BT_CONTROL_CMD_INQUIRY_STOP   "bt_control/cmd/inquiry_stop"
#define EVENT_BUS_TOPIC_BT_CONTROL_CMD_CONNECT_PHONE  "bt_control/cmd/connect_phone"
#define EVENT_BUS_TOPIC_BT_CONTROL_CMD_CONNECT_BREDR  "bt_control/cmd/connect_bredr"
#define EVENT_BUS_TOPIC_BT_CONTROL_CMD_DISCONNECT     "bt_control/cmd/disconnect"
#define EVENT_BUS_TOPIC_BT_CONTROL_CMD_REMOVE_BOND    "bt_control/cmd/remove_bond"
#define EVENT_BUS_TOPIC_BT_CONTROL_CMD_SWAP_TO        "bt_control/cmd/swap_to"

/*============================================================================*
 *                         Types
 *============================================================================*/

typedef enum
{
    /* Values match app_bond.h::T_DEVICE_TYPE so app_db can be copied directly. */
    BT_DEV_KIND_PHONE     = 0x00,
    BT_DEV_KIND_EARPHONE  = 0x01,
    BT_DEV_KIND_PC        = 0x02,
    BT_DEV_KIND_DEFAULT   = 0x03,
    BT_DEV_KIND_NONE      = 0xFF,
} T_BT_DEV_KIND_E;

typedef struct
{
    uint8_t  bd_addr[6];
    uint16_t device_name[BT_CONTROL_NAME_MAX_CHARS]; /* UTF-16, mirrors T_APP_BOND_DEVICE */
    uint8_t  device_name_len;                         /* in UTF-16 chars */
    uint8_t  kind;          /* T_BT_DEV_KIND_E */
    uint8_t  exist;         /* 0/1 - bond record present */
    uint8_t  connected;     /* 0/1 - BR link active (live, from app_find_br_link) */
} T_BT_CONTROL_DEVICE;

typedef struct
{
    uint8_t             phone_connected;       /* slot[0] connected */
    uint8_t             headphone_connected;   /* any of slot[1..7] connected */
    uint8_t             headphone_count;       /* count of exist slots in [1..7] */
    T_BT_CONTROL_DEVICE bonded[BT_CONTROL_MAX_BONDED];
} T_BT_CONTROL_STATE;

/* Payload for bt_control/cmd/connect_phone, connect_bredr, disconnect */
typedef struct
{
    uint8_t bd_addr[6];
} T_BT_CONTROL_ADDR_DATA;

/* Payload for bt_control/cmd/toggle */
typedef struct
{
    uint8_t enable;       /* 0/1 */
} T_BT_CONTROL_TOGGLE_DATA;

/* Payload for bt_control/cmd/remove_bond */
typedef struct
{
    uint8_t index;        /* app_db.bond_device[] index */
} T_BT_CONTROL_REMOVE_DATA;

/*============================================================================*
 *                         Functions
 *============================================================================*/

/**
 * @brief  Send BT control command/request from GUI to App.
 * @return true if the message was queued successfully.
 */
bool bt_control_gui_to_app(const char *topic, void *data, uint32_t size);

/**
 * @brief  Get a read-only pointer to the bridge's cached BT control state.
 *         Used by GUI list-note design callbacks that render outside of any
 *         specific msg subscription (e.g. on view re-enter). Always non-NULL.
 *         The pointer remains valid for the program lifetime.
 */
const T_BT_CONTROL_STATE *bridge_bt_control_get_state(void);

/**
 * @brief  Initialize the BT control bridge: register topics, subscribe to
 *         App-side events and GUI-side commands.
 *         Called from bridge_module_init().
 */
void bridge_bt_control_init(void);

#ifdef __cplusplus
}
#endif

#endif /* _BRIDGE_BT_CONTROL_H_ */
