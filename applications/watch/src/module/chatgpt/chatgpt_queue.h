/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _CHATGPT_QUEUE_H_
#define _CHATGPT_QUEUE_H_

#include <os_msg.h>
#include <os_task.h>
#include <gap.h>
#include <gap_le.h>
#include <gap_msg.h>
#include <communicate_task.h>
#include <app_msg.h>
#include <app_task.h>
#include "trace.h"
#include "communicate_protocol.h"
#include "communicate_parse.h"
#include "module_global_data.h"
#include "ble_transport.h"
#include "version.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct list_node
{
    struct list_node *p_next_node ;
    struct list_node *p_prev_node ;
    uint16_t data_length;
    uint8_t p_data[0];
} chatgpt_queue_t;

typedef void (*chatgpt_rev_cb)(const char *topic, void *data, uint16_t length);

typedef struct
{
    uint8_t message_id;
    //req param
    uint8_t rsp_data_type1;
    uint16_t action_type;

    //rsp param
    uint8_t action_status;
    uint8_t rsp_data_type2;

    uint8_t time_stamp[6];

    chatgpt_queue_t *send_list;
    chatgpt_queue_t *audio_rev_list;
    chatgpt_queue_t *text_rev_list;
    chatgpt_queue_t *image_rev_list;


    chatgpt_rev_cb pAppRevCB;
} _chatgpt_scene_thread;

void chatgpt_queue_printf(chatgpt_queue_t *p_list_head);
chatgpt_queue_t *chatgpt_queue_add_node(chatgpt_queue_t *p_list_head, chatgpt_queue_t *p_node);
chatgpt_queue_t *chatgpt_queue_indexof(chatgpt_queue_t *p_list_head, uint16_t node_index);
chatgpt_queue_t *chatgpt_queue_remove_node(chatgpt_queue_t *p_list_head, uint16_t node_index);
chatgpt_queue_t *chatgpt_queue_add_data(chatgpt_queue_t *p_list_head, uint8_t *p_data,
                                        uint16_t length);
chatgpt_queue_t *chatgpt_queue_remove_first_node(chatgpt_queue_t *p_list_head);
chatgpt_queue_t *chatgpt_queue_remove_last_node(chatgpt_queue_t *p_list_head);
chatgpt_queue_t *chatgpt_queue_indexof_last(chatgpt_queue_t *p_list_head);

void chatgpt_queue_clear(chatgpt_queue_t **p_list);

_chatgpt_scene_thread *chatgpt_scene_find(uint8_t m_id);
uint8_t chatgpt_scene_create(uint16_t action_type, uint8_t rsp_type, chatgpt_rev_cb appcb);
bool chatgpt_scene_delete(uint8_t m_id);
#ifdef __cplusplus
}
#endif

#endif

