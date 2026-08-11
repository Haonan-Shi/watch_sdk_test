/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _CHATGPT_APP_H_
#define _CHATGPT_APP_H_


#include "ble_transport.h"

#define SCREEN_WIDTH   25
#define SCREEN_HIGHT   15//400

#define SCREEN_UP_OFFSET  3
#define SCREEN_DOWN_OFFSET 3

#define LINE_HIGHT    32

typedef enum
{
    CHATGPT_STATUS_IDLE                     = 0,
    CHATGPT_STATUS_RECORDING                = 1,
    CHATGPT_STATUS_RECORD_END               = 2,
    CHATGPT_STATUS_RECORDING_DETECT_STOP    = 3,
    CHATGPT_STATUS_WHISPER_TXT               = 4,
    CHATGPT_STATUS_WAITING_TXT_RSP         = 5,
    CHATGPT_STATUS_REV_GPT_TEXT             = 6,
    CHATGPT_STATUS_REV_AUDIO                = 7,
    CHATGPT_STATUS_REV_IMAGE_PARAM          = 8,
    CHATGPT_STATUS_REV_IMAGE_DATA           = 9,
    CHATGPT_STATUS_REV_IMAGE_COMPLETE       = 10,
} _chatgpt_status;

typedef struct
{
    uint8_t  message_id;
    uint32_t line_num;
    uint32_t write_line_index;
    uint16_t write_row_offset;
    uint16_t line_width;
    uint32_t data_length;
    uint8_t *p_data;
    uint32_t buffer_length;
} _chatgpt_rev_data_t;



void chatgpt_app_voice_start(void);
void chatgpt_app_voice_stop(void);
_chatgpt_status chatgpt_app_get_status(void);

#endif// _CHATGPT_APP_H_
