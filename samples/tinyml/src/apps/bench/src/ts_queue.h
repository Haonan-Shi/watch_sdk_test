/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*
 * Zephyr port of the vendor ts_queue.h.  The original pulled in BLE GAP / OS
 * headers (os_msg.h, gap*.h, app_msg.h) that are irrelevant to the doubly
 * linked byte-buffer queue; those includes are dropped here.
 */

#ifndef _CHATGPT_QUEUE_H_
#define _CHATGPT_QUEUE_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct list_node
{
    struct list_node *p_next_node;
    struct list_node *p_prev_node;
    uint16_t data_length;
    uint8_t p_data[0];
} ts_queue_t;

void ts_queue_printf(ts_queue_t *p_list_head);
ts_queue_t *ts_queue_add_node(ts_queue_t *p_list_head, ts_queue_t *p_node);
ts_queue_t *ts_queue_indexof(ts_queue_t *p_list_head, uint16_t node_index);
ts_queue_t *ts_queue_remove_node(ts_queue_t *p_list_head, uint16_t node_index);
ts_queue_t *ts_queue_add_data(ts_queue_t *p_list_head, uint8_t *p_data, uint16_t length);
ts_queue_t *ts_queue_remove_first_node(ts_queue_t *p_list_head);
ts_queue_t *ts_queue_remove_last_node(ts_queue_t *p_list_head);
ts_queue_t *ts_queue_indexof_last(ts_queue_t *p_list_head);

void ts_queue_clear(ts_queue_t **p_list);

#ifdef __cplusplus
}
#endif

#endif
