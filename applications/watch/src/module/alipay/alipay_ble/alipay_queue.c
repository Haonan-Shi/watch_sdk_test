/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

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
#include "os_mem.h"
#include "alipay_queue.h"
#include <string.h>
#include "alipay_mem.h"
//#include "iotsec.h"

#if CONFIG_ALIPAY

/**
 * @brief add queue node for list head
 * @param ali_queue_t *p_list_head: list head pointer
 *        uint8_t *p_data : data need to be add
 *        uint16_t length: data length for p_data
 * @return ali_queue_t *: new list head
 * @note  new list head need to be handle
 *
 * @example
 * ali_queue_t *p_alipay_list = NULL;
 * uint8_t data[20] = {};
 * p_alipay_list = ali_queue_add_node(p_alipay_list, data, sizeof(data));
 */
ali_queue_t *ali_queue_add_node(ali_queue_t *p_list_head, uint8_t *p_data, uint16_t length)
{
    if (p_data == NULL)
    {
        return p_list_head;
    }

    if (p_list_head == NULL)
    {
        p_list_head = csi_malloc(sizeof(ali_queue_t) + length);
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
        ali_queue_t *p_node = csi_malloc(sizeof(ali_queue_t) + length);
        if (p_node == NULL)
        {
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
 * @param ali_queue_t *p_list_head: list head pointer
 *        uint16_t node_index: node index to be remove
 * @return ali_queue_t *: new list head
 *
 * @example ali_queue_t *p_alipay_list = NULL;
 *          uint8_t data[20] = {};
 *          p_alipay_list = ali_queue_add_node(p_alipay_list, data, sizeof(data));
 *          p_alipay_list = ali_queue_remove_node(p_alipay_list, 0);// p_list_head is null
 */
ali_queue_t *ali_queue_remove_node(ali_queue_t *p_list_head, uint16_t node_index)
{
    if (p_list_head == NULL)
    {
        return NULL;
    }

    ali_queue_t *p_target_node = NULL;

    uint16_t index = 0;
    for (ali_queue_t *p_iterator = p_list_head;; p_iterator = p_iterator->p_next_node)
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
            csi_free(p_list_head);
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

        csi_free(p_target_node);
        p_target_node = NULL;
    }
    return p_list_head;
}

/**
 * @brief return the queue last node
 * @param ali_queue_t *p_list_head: list head pointer
 * @return ali_queue_t *: the last node ref
 *
 * @example  ali_queue_t* p_last_node = ali_queue_indexof_last(g_alipay_ble_transport_list);
 *
 */
ali_queue_t *ali_queue_indexof_last(ali_queue_t *p_list_head)
{
    if (p_list_head == NULL)
    {
        return NULL;
    }

    return p_list_head->p_prev_node;
}

/**
 * @brief remove the queue last node
 * @param ali_queue_t *p_list_head: list head pointer
 * @return ali_queue_t *: the new list queue head
 *
 * @example  ali_queue_t *p_alipay_list = NULL;
 *           p_alipay_list = ali_queue_remove_last_node(p_alipay_list);
 *
 */
ali_queue_t *ali_queue_remove_last_node(ali_queue_t *p_list_head)
{
    if (p_list_head == NULL)
    {
        return NULL;
    }

    ali_queue_t *p_target_node = NULL;
    p_target_node = p_list_head->p_prev_node;

    if (p_target_node == p_list_head)
    {
        csi_free(p_list_head);
        p_list_head = NULL;
        return NULL;
    }

    p_target_node->p_next_node->p_prev_node = p_target_node->p_prev_node;
    p_target_node->p_prev_node->p_next_node = p_target_node->p_next_node;

    csi_free(p_target_node);
    p_target_node = NULL;
    return p_list_head;
}

/**
 * @brief remove the queue first node
 * @param ali_queue_t *p_list_head: list head pointer
 * @return ali_queue_t *: the new list queue head
 *
 * @example  ali_queue_t *p_alipay_list = NULL;
 *           p_alipay_list = ali_queue_remove_first_node(p_alipay_list);
 *
 */
ali_queue_t *ali_queue_remove_first_node(ali_queue_t *p_list_head)
{
    return ali_queue_remove_node(p_list_head, 0);
}

/**
 * @brief index the queue list by node index
 * @param ali_queue_t *p_list_head: list head pointer
 *        uint16_t node_index: node index
 * @return ali_queue_t *: the new list queue head
 *
 * @example  ali_queue_t *p_alipay_list = NULL;
 *           p_alipay_list = ali_queue_indexof(p_alipay_list, 6);
 *
 */
ali_queue_t *ali_queue_indexof(ali_queue_t *p_list_head, uint16_t node_index)
{

    if (p_list_head == NULL)
    {
        return NULL;
    }

    uint16_t index = 0;
    ali_queue_t *p_iterator = NULL;
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

    //return NULL;
}

void ali_queue_printf(ali_queue_t *p_list_head)
{
    if (p_list_head == NULL)
    {
        APP_PRINT_INFO0("[Alipay] queue is null!");
        return;
    }

    uint16_t index = 0;
    for (ali_queue_t *p_iterator = p_list_head;; p_iterator = p_iterator->p_next_node)
    {
        APP_PRINT_INFO2("[Alipay] queue index %d, data %b", index, TRACE_BINARY(p_iterator->data_length,
                                                                                p_iterator->p_data));
        if (p_iterator->p_next_node == p_list_head)
        {
            return;
        }
        index++;
    }
}

#endif //CONFIG_ALIPAY
