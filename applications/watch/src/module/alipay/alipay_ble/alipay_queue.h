/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _ALIPAY_QUEUE_H_
#define _ALIPAY_QUEUE_H_

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
#include "version.h"
#include "section.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct list_node
{
    struct list_node *p_next_node ;
    struct list_node *p_prev_node ;
    uint16_t data_length;
    uint8_t p_data[0];
} ali_queue_t;

void ali_queue_printf(ali_queue_t *p_list_head);
ali_queue_t *ali_queue_indexof(ali_queue_t *p_list_head, uint16_t node_index);
ali_queue_t *ali_queue_remove_node(ali_queue_t *p_list_head, uint16_t node_index);
ali_queue_t *ali_queue_add_node(ali_queue_t *p_list_head, uint8_t *p_data, uint16_t length);
ali_queue_t *ali_queue_remove_first_node(ali_queue_t *p_list_head);
ali_queue_t *ali_queue_remove_last_node(ali_queue_t *p_list_head);
ali_queue_t *ali_queue_indexof_last(ali_queue_t *p_list_head);

#ifdef __cplusplus
}
#endif

#endif

