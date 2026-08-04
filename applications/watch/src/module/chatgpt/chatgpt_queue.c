/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "trace.h"
#include "version.h"
#include "chatgpt_port.h"
#include "chatgpt_queue.h"
#include "ble_transport.h"
#include "string.h"

chatgpt_queue_t *chatgpt_scene_list = NULL;


chatgpt_queue_t *chatgpt_queue_add_node(chatgpt_queue_t *p_list_head, chatgpt_queue_t *p_node)
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
 * @param chatgpt_queue_t *p_list_head: list head pointer
 *        uint8_t *p_data : data need to be add
 *        uint16_t length: data length for p_data
 * @return chatgpt_queue_t *: new list head
 * @note  new list head need to be handle
 *
 * @example
 * chatgpt_queue_t *p_chatgpt_list = NULL;
 * uint8_t data[20] = {};
 * p_chatgpt_list = chatgpt_queue_add_data(p_chatgpt_list, data, sizeof(data));
 */
chatgpt_queue_t *chatgpt_queue_add_data(chatgpt_queue_t *p_list_head, uint8_t *p_data,
                                        uint16_t length)
{
    if (p_data == NULL)
    {
        APP_PRINT_INFO0("[ChatGPT] audio recv data p_data == NULL");
        return p_list_head;
    }

    if (p_list_head == NULL)
    {
        p_list_head = chatgpt_malloc(sizeof(chatgpt_queue_t) + length);
        if (p_list_head == NULL)
        {
            APP_PRINT_INFO0("[ChatGPT] audio recv data p_list_head == NULL malloc fail");
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
        chatgpt_queue_t *p_node = chatgpt_malloc(sizeof(chatgpt_queue_t) + length);
        if (p_node == NULL)
        {
            APP_PRINT_INFO0("[ChatGPT] chatgpt_malloc failed ====> ");
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
 * @param chatgpt_queue_t *p_list_head: list head pointer
 *        uint16_t node_index: node index to be remove
 * @return chatgpt_queue_t *: new list head
 *
 * @example chatgpt_queue_t *p_chatgpt_list = NULL;
 *          uint8_t data[20] = {};
 *          p_chatgpt_list = chatgpt_queue_add_data(p_chatgpt_list, data, sizeof(data));
 *          p_chatgpt_list = chatgpt_queue_remove_node(p_chatgpt_list, 0);// p_list_head is null
 */
chatgpt_queue_t *chatgpt_queue_remove_node(chatgpt_queue_t *p_list_head, uint16_t node_index)
{
    if (p_list_head == NULL)
    {
        return NULL;
    }

    chatgpt_queue_t *p_target_node = NULL;

    uint16_t index = 0;
    for (chatgpt_queue_t *p_iterator = p_list_head;; p_iterator = p_iterator->p_next_node)
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
            chatgpt_free(p_list_head);
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

        chatgpt_free(p_target_node);
        p_target_node = NULL;
    }
    return p_list_head;
}

//not debug
void chatgpt_queue_clear(chatgpt_queue_t **p_list)
{

    if (NULL == p_list)
    {
        return;
    }
    chatgpt_queue_t *p_list_head = *p_list;
    if (p_list_head == NULL)
    {
        return;
    }

    for (;;)
    {
        if (p_list_head->p_prev_node == p_list_head)
        {
            chatgpt_free(p_list_head);
            p_list_head = NULL;

            break;
        }

        chatgpt_queue_t *p_temp_node = p_list_head;

        p_list_head->p_prev_node->p_next_node = p_list_head->p_next_node;
        p_list_head->p_next_node->p_prev_node = p_list_head->p_prev_node;

        p_list_head->p_prev_node = NULL;
        p_list_head = p_list_head->p_next_node;

        chatgpt_free(p_temp_node);
        p_temp_node = NULL;
    }

    *p_list = NULL;
}
/**
 * @brief return the queue last node
 * @param chatgpt_queue_t *p_list_head: list head pointer
 * @return chatgpt_queue_t *: the last node ref
 *
 * @example  chatgpt_queue_t* p_last_node = chatgpt_queue_indexof_last(g_chatgpt_ble_transport_list);
 *
 */
chatgpt_queue_t *chatgpt_queue_indexof_last(chatgpt_queue_t *p_list_head)
{
    if (p_list_head == NULL)
    {
        return NULL;
    }

    return p_list_head->p_prev_node;
}

/**
 * @brief remove the queue last node
 * @param chatgpt_queue_t *p_list_head: list head pointer
 * @return chatgpt_queue_t *: the new list queue head
 *
 * @example  chatgpt_queue_t *p_chatgpt_list = NULL;
 *           p_chatgpt_list = chatgpt_queue_remove_last_node(p_chatgpt_list);
 *
 */
chatgpt_queue_t *chatgpt_queue_remove_last_node(chatgpt_queue_t *p_list_head)
{
    if (p_list_head == NULL)
    {
        return NULL;
    }

    chatgpt_queue_t *p_target_node = NULL;
    p_target_node = p_list_head->p_prev_node;

    if (p_target_node == p_list_head)
    {
        chatgpt_free(p_list_head);
        p_list_head = NULL;
        return NULL;
    }

    p_target_node->p_next_node->p_prev_node = p_target_node->p_prev_node;
    p_target_node->p_prev_node->p_next_node = p_target_node->p_next_node;

    chatgpt_free(p_target_node);
    p_target_node = NULL;
    return p_list_head;
}

/**
 * @brief remove the queue first node
 * @param chatgpt_queue_t *p_list_head: list head pointer
 * @return chatgpt_queue_t *: the new list queue head
 *
 * @example  chatgpt_queue_t *p_chatgpt_list = NULL;
 *           p_chatgpt_list = chatgpt_queue_remove_first_node(p_chatgpt_list);
 *
 */
chatgpt_queue_t *chatgpt_queue_remove_first_node(chatgpt_queue_t *p_list_head)
{
    return chatgpt_queue_remove_node(p_list_head, 0);
}

/**
 * @brief index the queue list by node index
 * @param chatgpt_queue_t *p_list_head: list head pointer
 *        uint16_t node_index: node index
 * @return chatgpt_queue_t *: the new list queue head
 *
 * @example  chatgpt_queue_t *p_chatgpt_list = NULL;
 *           p_chatgpt_list = chatgpt_queue_indexof(p_chatgpt_list, 6);
 *
 */
chatgpt_queue_t *chatgpt_queue_indexof(chatgpt_queue_t *p_list_head, uint16_t node_index)
{

    if (p_list_head == NULL)
    {
        return NULL;
    }

    uint16_t index = 0;
    chatgpt_queue_t *p_iterator = NULL;
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

void chatgpt_queue_printf(chatgpt_queue_t *p_list_head)
{
    if (p_list_head == NULL)
    {
        APP_PRINT_INFO0("[ChatGPT] queue is null!");
        return;
    }

    uint16_t index = 0;
    for (chatgpt_queue_t *p_iterator = p_list_head;; p_iterator = p_iterator->p_next_node)
    {
        APP_PRINT_INFO2("[ChatGPT] queue index %d, data %b", index, TRACE_BINARY(p_iterator->data_length,
                                                                                 p_iterator->p_data));
        if (p_iterator->p_next_node == p_list_head)
        {
            return;
        }
        index++;
    }
}

/**===============================================================================
*
*                           Scene Handler
*
*================================================================================*/

_chatgpt_scene_thread *chatgpt_scene_find(uint8_t m_id)
{
    if (chatgpt_scene_list == NULL)
    {
        APP_PRINT_INFO0("[ChatGPT] chatgpt_scene_find list is null!");
        return NULL;
    }

    uint16_t index = 0;
    for (chatgpt_queue_t *p_iterator = chatgpt_scene_list;; p_iterator = p_iterator->p_next_node)
    {
        uint16_t l_scene_data_length = sizeof(_chatgpt_scene_thread);
        _chatgpt_scene_thread *p_scene = (_chatgpt_scene_thread *)(p_iterator->p_data);
        if ((p_iterator->data_length >= l_scene_data_length) && (p_scene->message_id == m_id))
        {
            APP_PRINT_INFO0("[chatgpt] chatgpt_scene_find success!");
            return (_chatgpt_scene_thread *)p_iterator->p_data;
        }
        else
        {
            APP_PRINT_INFO0("[chatgpt] chatgpt_scene_find failed!");
        }

        if (p_iterator->p_next_node == chatgpt_scene_list)
        {
            return NULL;
        }

        index++;
    }

    return NULL;
}

uint8_t chatgpt_scene_create(uint16_t action_type, uint8_t rsp_type, chatgpt_rev_cb appcb)
{
    //every scene will get a message id
    static uint8_t message_id = 0;
    uint8_t m_id = message_id;

    if (chatgpt_scene_find(m_id) != NULL)
    {
        APP_PRINT_INFO0("[ChatGPT] chatgpt_scene_create list already exist!");
        return 0xff;
    }

    uint16_t l_scene_data_length = sizeof(_chatgpt_scene_thread);
    _chatgpt_scene_thread l_scene_data = {0};

    l_scene_data.action_type = action_type;
    l_scene_data.rsp_data_type1 = rsp_type;
    l_scene_data.pAppRevCB = appcb;

    l_scene_data.message_id = m_id;
    chatgpt_scene_list = chatgpt_queue_add_data(chatgpt_scene_list, (uint8_t *)&l_scene_data,
                                                l_scene_data_length);
    message_id++;
    if (message_id == 0xff)//0xff is user for create fail event
    {
        message_id = 0;
    }

    return m_id;
}

bool chatgpt_scene_delete(uint8_t m_id)
{
    if (chatgpt_scene_list == NULL)
    {
        APP_PRINT_INFO0("[ChatGPT] chatgpt_scene_delete list is null!");
        return 0;
    }

    uint16_t index = 0;
    for (chatgpt_queue_t *p_iterator = chatgpt_scene_list;; p_iterator = p_iterator->p_next_node)
    {
        APP_PRINT_INFO2("[ChatGPT] chatgpt_scene_delete queue index %d, data %b", index,
                        TRACE_BINARY(p_iterator->data_length,
                                     p_iterator->p_data));
        if (p_iterator->p_next_node == chatgpt_scene_list)
        {
            return 0;
        }

        _chatgpt_scene_thread *p_scene_data  = (_chatgpt_scene_thread *)p_iterator->p_data;

        uint16_t l_scene_data_length = sizeof(_chatgpt_scene_thread);
        if ((p_iterator->data_length == l_scene_data_length)
            && (p_scene_data->message_id == m_id))
        {
            //chatgpt_queue_remove_node(g_chatgpt_scene_list, index);
            break;
        }

        index++;
    }

    chatgpt_queue_remove_node(chatgpt_scene_list, index);

    return 1;
}
