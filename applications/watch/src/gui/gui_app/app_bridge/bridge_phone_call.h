/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *               Define to prevent recursive inclusion
 *============================================================================*/
#ifndef _BRIDGE_PHONE_CALL_H_
#define _BRIDGE_PHONE_CALL_H_

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
#define PHONE_CALL_NUMBER_MAX_LEN  32
#define PHONE_CALL_NAME_MAX_LEN    32
#define PHONE_CALL_VOLUME_MAX      15

/* gui topic - kept as legacy strings to match HoneyGUI-auto-generated
 * gui_msg_subscribe() calls in app_phone_ui.c. Payload for both is unified
 * to T_PHONE_CALL_STATE; consumers read whichever fields they care about. */
#define GUI_TOPIC_PHONE_NUMBER                 "phone/number"
#define GUI_TOPIC_PHONE_CALLER_ID              "phone/caller_id"

/*app topic*/
#define EVENT_BUS_TOPIC_PHONE_CALL_ALL_TOPIC      "phone_call/*"
#define EVENT_BUS_TOPIC_PHONE_CALL_REQ_STATE      "phone_call/req/state"
#define EVENT_BUS_TOPIC_PHONE_CALL_CMD_DIAL       "phone_call/cmd/dial"
#define EVENT_BUS_TOPIC_PHONE_CALL_CMD_ANSWER     "phone_call/cmd/answer"
#define EVENT_BUS_TOPIC_PHONE_CALL_CMD_END        "phone_call/cmd/end"
#define EVENT_BUS_TOPIC_PHONE_CALL_CMD_MUTE       "phone_call/cmd/mute"
#define EVENT_BUS_TOPIC_PHONE_CALL_CMD_UNMUTE     "phone_call/cmd/unmute"
#define EVENT_BUS_TOPIC_PHONE_CALL_CMD_VOL_UP     "phone_call/cmd/vol_up"
#define EVENT_BUS_TOPIC_PHONE_CALL_CMD_VOL_DOWN   "phone_call/cmd/vol_down"

/*============================================================================*
 *                         Types
 *============================================================================*/

typedef enum
{
    PHONE_CALL_STATE_IDLE     = 0x00,
    PHONE_CALL_STATE_INCOMING = 0x01,
    PHONE_CALL_STATE_OUTGOING = 0x02,
    PHONE_CALL_STATE_ACTIVE   = 0x03,
    PHONE_CALL_STATE_ENDED    = 0x04,
} T_PHONE_CALL_STATE_E;

typedef struct
{
    T_PHONE_CALL_STATE_E call_state;
    char     number[PHONE_CALL_NUMBER_MAX_LEN];
    uint8_t  number_len;
    char     caller_name[PHONE_CALL_NAME_MAX_LEN];
    uint8_t  caller_name_len;
    uint8_t  volume;        /* 0~PHONE_CALL_VOLUME_MAX */
} T_PHONE_CALL_STATE;

typedef struct
{
    char     number[PHONE_CALL_NUMBER_MAX_LEN];
    uint8_t  len;
} T_PHONE_CALL_DIAL_DATA;

/*============================================================================*
 *                         Constants
 *============================================================================*/

/*============================================================================*
 *                         Variables
 *============================================================================*/

/*============================================================================*
 *                         Functions
 *============================================================================*/

/**
 * @brief Send phone call command or data from GUI to App.
 *
 * @param  topic The topic of the message.
 * @param  data Pointer to the data to be sent.
 * @param  size Size of the data to be sent.
 * @return true if the message was sent successfully, false otherwise.
 */
bool phone_call_gui_to_app(const char *topic, void *data, uint32_t size);

/**
 * @brief Initialize the phone call bridge, including creating topics and subscribing to events.
 *        This function should be called in bridge_module_init().
 */
void bridge_phone_call_init(void);

#ifdef __cplusplus
}
#endif

#endif /* _BRIDGE_PHONE_CALL_H_ */
