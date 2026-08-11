/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef __CHATGPT_PORT_H__
#define __CHATGPT_PORT_H__

#include <stdint.h>
#include <stdbool.h>

/*===================================================================
*                     chatgpt log
*===================================================================*/

#define chatgpt_log(format, ...)  DBG_DIRECT(format, ##__VA_ARGS__)

#define EVENT_BUS_TOPIC_CHATGPT_ALL_TOPIC                       "cg/*"
#define EVENT_BUS_TOPIC_CHATGPT_OPUS_ENCODE_START               "cg/opus/start"
#define EVENT_BUS_TOPIC_CHATGPT_OPUS_ENCODE_DATA                "cg/opus/data"
#define EVENT_BUS_TOPIC_CHATGPT_OPUS_ENCODE_CLOSE               "cg/opus/close"
#define EVENT_BUS_TOPIC_CHATGPT_CMD_SEND                        "cg/cmd/send"
#define EVENT_BUS_TOPIC_CHATGPT_REV_TEXT_PREPARE                "cg/rev/text/pre"
#define EVENT_BUS_TOPIC_CHATGPT_REV_TEXT_DATA                   "cg/rev/text/data"
#define EVENT_BUS_TOPIC_CHATGPT_REV_TEXT_COMPLETE               "cg/rev/text/cpl"
#define EVENT_BUS_TOPIC_CHATGPT_REV_AUDIO_PREPARE               "cg/rev/au/pre"
#define EVENT_BUS_TOPIC_CHATGPT_REV_AUDIO_DATA                  "cg/rev/au/data"
#define EVENT_BUS_TOPIC_CHATGPT_REV_AUDIO_COMPLETE              "cg/rev/au/cpl"
#define EVENT_BUS_TOPIC_CHATGPT_REV_IMAGE_PREPARE               "cg/rev/img/pre"
#define EVENT_BUS_TOPIC_CHATGPT_REV_IMAGE_DATA                  "cg/rev/img/data"
#define EVENT_BUS_TOPIC_CHATGPT_REV_IMAGE_COMPLETE              "cg/rev/img/cpl"
#define EVENT_BUS_TOPIC_CHATGPT_VOICE_START                     "cg/vo/start"
#define EVENT_BUS_TOPIC_CHATGPT_VOICE_STOP                      "cg/vo/stop"
#define EVENT_BUS_TOPIC_CHATGPT_APP_CONFIRM_IND                 "cg/app/cfm_ind"


void *chatgpt_realloc(void *mem, uint32_t size);
void *chatgpt_malloc(uint32_t size);
void *chatgpt_calloc(uint32_t nblock, uint32_t size);
void chatgpt_free(void *pt);

bool chatgpt_port_data_send(uint8_t *data, uint16_t length);
uint8_t chatgpt_port_get_ble_state(void);
uint8_t chatgpt_port_ble_rev_proc(uint8_t key, uint8_t *pBuf, uint16_t length);
void chatgpt_port_ble_send_completed_proc(void);
#endif //__CHATGPT_PORT_H__
