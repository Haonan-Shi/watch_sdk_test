/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _BLE_TRANSPORT_
#define _BLE_TRANSPORT_

#include "stdint.h"
#include "audio_type.h"
#include "chatgpt_port.h"

typedef enum _chat_gpt_subcode
{
    INVALID,
    CHATGPT_CMD_VOICE_START             = 1,
    CHATGPT_CMD_VOICE_DATA              = 2,
    CHATGPT_CMD_VOICE_END               = 3,
    CHATGPT_EVENT_WHISPER_DATA          = 4,
    CHATGPT_EVENT_GPT_DATA              = 5,
    CHATGPT_EVENT_GPT_VOICE_START       = 6,
    CHATGPT_EVENT_GPT_VOICE_DATA        = 7,
    CHATGPT_EVENT_GPT_VOICE_END         = 8,
    CHATGPT_CMD_GPT_DATA_RETRANSMIT     = 9,
    CHATGPT_CMD_GPT_VOICE_CANCEL        = 10,
    CHATGPT_CMD_GPT_VOICE_CTRL          = 11,
    CHATGPT_EVENT_GPT_DETECT_STOP       = 12,

    //new common protocol
    CHATGPT_CMD_ACTION_REQ_START        = 0x0D,
    CHATGPT_CMD_ACTION_REQ_DATA         = 0x0E,
    CHATGPT_CMD_ACTION_REQ_COMPLETE     = 0x0F,

    CHATGPT_EVENT_ACTION_RSP_PARAM      = 0x10,
    CHATGPT_EVENT_ACTION_RSP_DATA       = 0x11,
    CHATGPT_EVENT_ACTION_RSP_COMPLETE   = 0x12,

    CHATGPT_CMD_ACTION_CONFIRM          = 0x13,

} _chatgpt_subcode_t;


typedef enum
{
    CHATGPT_SCENE_SPEECH_RECOGNITION     = (0x01 << 0),
    CHATGPT_SCENE_AI_CHAT                = (0x01 << 1),
    CHATGPT_SCENE_GENERATE_PICTURE       = (0x01 << 2),
} e_chatgpt_action;

typedef enum
{
    CHATGPT_CONFIRM_SUCCESS = 0,
    CHATGPT_CONFIRM_RETRY   = 1,
    CHATGPT_CONFIRM_CONCEL  = 2,
} e_chatgpt_confirm_type;


typedef enum
{
    CHATGPT_RSP_TYPE_TEXT   = (0x01 << 0),
    CHATGPT_RSP_TYPE_VOICE  = (0x01 << 1),
    CHATGPT_RSP_TYPE_IMAGE  = (0x01 << 2),
    CHATGPT_RSP_TYPE_FILE   = (0x01 << 3),

} e_chatgpt_rsp_type;


void chatgpt_ble_send_completed_proc(void);
uint8_t chatgpt_ble_rev_proc(uint8_t key, uint8_t *pBuf, uint16_t length);

#endif//_BLE_TRANSPORT_
