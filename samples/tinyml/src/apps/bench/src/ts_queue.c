/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "trace.h"
#include "version.h"

#include "ts_mem.h"
#include "ts_queue.h"
#include "string.h"

ts_queue_t *ts_queue_add_node(ts_queue_t *p_list_head, ts_queue_t *p_node)
{
    if (p_node == NULL)
    {
        return p_list_head;
    }

    if (p_list_head == NULL)
    {
        p_list_head = p_node;

        p_list_head->p_next_node = p_list_head;
        p_list_head->p_prev_node = p_list_head;
    }
    else
    {
        p_node->p_prev_node = p_list_head->p_prev_node;
        p_node->p_next_node = p_list_head;

        p_node->p_prev_node->p_next_node = p_node;
        p_node->p_next_node->p_prev_node = p_node;

        p_list_head = p_node;
    }

    return p_list_head;
}

/**
 * @brief add queue node for list head
 * @param ts_queue_t *p_list_head: list head pointer
 *        uint8_t *p_data : data need to be add
 *        uint16_t length: data length for p_data
 * @return ts_queue_t *: new list head
 * @note  new list head need to be handle
 *
 * @example
 * ts_queue_t *p_ts_list = NULL;
 * uint8_t data[20] = {};
 * p_ts_list = ts_queue_add_data(p_ts_list, data, sizeof(data));
 */
ts_queue_t *ts_queue_add_data(ts_queue_t *p_list_head, uint8_t *p_data, uint16_t length)
{
    if (p_data == NULL)
    {
        return p_list_head;
    }

    if (p_list_head == NULL)
    {
        p_list_head = ts_malloc(sizeof(ts_queue_t) + length);
        if (p_list_head == NULL)
        {
            return NULL;
        }

        p_list_head->data_length = length;
        // memcpy(p_list_head->p_data, ">", 1);
        memcpy(p_list_head->p_data, p_data, length);

        p_list_head->p_next_node = p_list_head;
        p_list_head->p_prev_node = p_list_head;
    }
    else
    {
        ts_queue_t *p_node = ts_malloc(sizeof(ts_queue_t) + length);
        if (p_node == NULL)
        {
            DBG_DIRECT("[ts] ts_malloc failed ====> ");
            return p_list_head;
        }

        p_node->data_length = length;
        // memcpy(p_node->p_data, ">", 1);
        memcpy(p_node->p_data, p_data, length);

        p_node->p_prev_node = p_list_head->p_prev_node;
        p_node->p_next_node = p_list_head;

        p_node->p_prev_node->p_next_node = p_node;
        p_node->p_next_node->p_prev_node = p_node;

        p_list_head = p_node;
    }

    return p_list_head;
}

/**
 * @brief remove node by node_index from list
 * @param ts_queue_t *p_list_head: list head pointer
 *        uint16_t node_index: node index to be remove
 * @return ts_queue_t *: new list head
 *
 * @example ts_queue_t *p_ts_list = NULL;
 *          uint8_t data[20] = {};
 *          p_ts_list = ts_queue_add_data(p_ts_list, data, sizeof(data));
 *          p_ts_list = ts_queue_remove_node(p_ts_list, 0);// p_list_head is null
 */
ts_queue_t *ts_queue_remove_node(ts_queue_t *p_list_head, uint16_t node_index)
{
    if (p_list_head == NULL)
    {
        return NULL;
    }

    ts_queue_t *p_target_node = NULL;

    uint16_t index = 0;
    for (ts_queue_t *p_iterator = p_list_head;; p_iterator = p_iterator->p_next_node)
    {
        if (index == node_index)
        {
            p_target_node = p_iterator;
            break;
        }

        if (p_iterator->p_next_node == p_list_head)
        {
            return p_list_head;
        }

        index++;
    }

    if (node_index == 0)
    {
        if (p_list_head->p_next_node == p_list_head)
        {
            ts_free(p_list_head);
            p_list_head = NULL;
            return NULL;
        }
        else
        {
            p_list_head = p_list_head->p_next_node;
        }
    }

    if (p_target_node != NULL)
    {
        p_target_node->p_next_node->p_prev_node = p_target_node->p_prev_node;
        p_target_node->p_prev_node->p_next_node = p_target_node->p_next_node;

        ts_free(p_target_node);
        p_target_node = NULL;
    }
    return p_list_head;
}

//not debug
void ts_queue_clear(ts_queue_t **p_list)
{

    if (NULL == p_list)
    {
        return;
    }
    ts_queue_t *p_list_head = *p_list;
    if (p_list_head == NULL)
    {
        return;
    }

    for (;;)
    {
        if (p_list_head->p_prev_node == p_list_head)
        {
            ts_free(p_list_head);
            p_list_head = NULL;

            break;
        }

        ts_queue_t *p_temp_node = p_list_head;

        p_list_head->p_prev_node->p_next_node = p_list_head->p_next_node;
        p_list_head->p_next_node->p_prev_node = p_list_head->p_prev_node;

        p_list_head->p_prev_node = NULL;
        p_list_head = p_list_head->p_next_node;

        ts_free(p_temp_node);
        p_temp_node = NULL;
    }

    *p_list = NULL;
}
/**
 * @brief return the queue last node
 * @param ts_queue_t *p_list_head: list head pointer
 * @return ts_queue_t *: the last node ref
 *
 * @example  ts_queue_t* p_last_node = ts_queue_indexof_last(g_ts_ble_transport_list);
 *
 */
ts_queue_t *ts_queue_indexof_last(ts_queue_t *p_list_head)
{
    if (p_list_head == NULL)
    {
        return NULL;
    }

    return p_list_head->p_prev_node;
}

/**
 * @brief remove the queue last node
 * @param ts_queue_t *p_list_head: list head pointer
 * @return ts_queue_t *: the new list queue head
 *
 * @example  ts_queue_t *p_ts_list = NULL;
 *           p_ts_list = ts_queue_remove_last_node(p_ts_list);
 *
 */
ts_queue_t *ts_queue_remove_last_node(ts_queue_t *p_list_head)
{
    if (p_list_head == NULL)
    {
        return NULL;
    }

    ts_queue_t *p_target_node = NULL;
    p_target_node = p_list_head->p_prev_node;

    if (p_target_node == p_list_head)
    {
        ts_free(p_list_head);
        p_list_head = NULL;
        return NULL;
    }

    p_target_node->p_next_node->p_prev_node = p_target_node->p_prev_node;
    p_target_node->p_prev_node->p_next_node = p_target_node->p_next_node;

    ts_free(p_target_node);
    p_target_node = NULL;
    return p_list_head;
}

/**
 * @brief remove the queue first node
 * @param ts_queue_t *p_list_head: list head pointer
 * @return ts_queue_t *: the new list queue head
 *
 * @example  ts_queue_t *p_ts_list = NULL;
 *           p_ts_list = ts_queue_remove_first_node(p_ts_list);
 *
 */
ts_queue_t *ts_queue_remove_first_node(ts_queue_t *p_list_head)
{
    return ts_queue_remove_node(p_list_head, 0);
}

/**
 * @brief index the queue list by node index
 * @param ts_queue_t *p_list_head: list head pointer
 *        uint16_t node_index: node index
 * @return ts_queue_t *: the new list queue head
 *
 * @example  ts_queue_t *p_ts_list = NULL;
 *           p_ts_list = ts_queue_indexof(p_ts_list, 6);
 *
 */
ts_queue_t *ts_queue_indexof(ts_queue_t *p_list_head, uint16_t node_index)
{

    if (p_list_head == NULL)
    {
        return NULL;
    }

    uint16_t index = 0;
    ts_queue_t *p_iterator = NULL;
    for (p_iterator = p_list_head;; p_iterator = p_iterator->p_next_node)
    {
        if (index == node_index)
        {
            return p_iterator;
        }

        if (p_iterator->p_next_node == p_list_head)
        {
            return NULL;
        }
        index++;
    }

    return NULL;
}

void ts_queue_printf(ts_queue_t *p_list_head)
{
    if (p_list_head == NULL)
    {
        DBG_DIRECT("[ts] queue is null!");
        return;
    }

    uint16_t index = 0;
    for (ts_queue_t *p_iterator = p_list_head;; p_iterator = p_iterator->p_next_node)
    {
        /* Original vendor code used the RTK-private "%b" TRACE_BINARY specifier
         * to hexdump the payload; printk has no equivalent, so log the length
         * instead (this debug helper is not on the hot path). */
        DBG_DIRECT("[ts] queue index %d, data_length %d", index, p_iterator->data_length);
        if (p_iterator->p_next_node == p_list_head)
        {
            return;
        }
        index++;
    }
}
