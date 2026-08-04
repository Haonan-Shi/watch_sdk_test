/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "ble_transport.h"
#include "communicate_parse.h"
#include "chatgpt_queue.h"
#include "os_mem.h"
#include "app_mmi.h"
#include "audio_type.h"
#include "audio_track.h"
#include "audio.h"
#include "os_timer.h"
#include "chatgpt_port.h"
#include "string.h"
#include "chatgpt_voice.h"

/*============================================================================*
*                              Local Variables
*============================================================================*/
static uint8_t data_send_buffer[256] = {0};

/**===============================================================================
*
*                           Voice send
*
*================================================================================*/

uint8_t chatgpt_ble_buffer_send(_chatgpt_scene_thread *p_scence)
{
    if (p_scence == NULL)
    {
        chatgpt_log("[ChatGPT] chatgpt_ble_buffer_send p_scence NULL");
        return 0;
    }

    uint8_t chatgpt_cretids = 0;
    le_get_gap_param(GAP_PARAM_LE_REMAIN_CREDITS, &chatgpt_cretids);

    for (; chatgpt_cretids > 0;)
    {
        chatgpt_queue_t *p_last_node = chatgpt_queue_indexof_last(p_scence->send_list);
        if (p_last_node == NULL)
        {
            chatgpt_log("[ChatGPT] chatgpt_ble_buffer_send NULL");
            break;
        }

        if (RtkWristbandSys.gap_conn_state != GAP_CONN_STATE_CONNECTED)
        {
            chatgpt_log("[ChatGPT] chatgpt_ble_buffer_send not connected, state %d",
                        RtkWristbandSys.gap_conn_state);
        }
        else
        {
            chatgpt_log("[ChatGPT] chatgpt_ble_buffer_send data");
            chatgpt_port_data_send(p_last_node->p_data, p_last_node->data_length);
        }

        p_scence->send_list = chatgpt_queue_remove_last_node(p_scence->send_list);

        le_get_gap_param(GAP_PARAM_LE_REMAIN_CREDITS, &chatgpt_cretids);
    }

    return 1;

}


void chatgpt_ble_send_completed_proc(void)
{
    extern chatgpt_queue_t *chatgpt_scene_list;
    if (chatgpt_scene_list == NULL)
    {
        chatgpt_log("[ChatGPT] chatgpt_scene_list is null!");
        return;
    }

    uint16_t index = 0;
    for (chatgpt_queue_t *p_iterator = chatgpt_scene_list;; p_iterator = p_iterator->p_next_node)
    {
        chatgpt_log("[ChatGPT] queue index %d, data %b", index, TRACE_BINARY(p_iterator->data_length,
                                                                             p_iterator->p_data));

        _chatgpt_scene_thread *p_scene = (_chatgpt_scene_thread *)(p_iterator->p_data);
        chatgpt_queue_t *p_last_node = chatgpt_queue_indexof_last(p_scene->send_list);
        if (p_last_node == NULL)
        {
            return;
        }
        event_bus_publish(EVENT_BUS_TOPIC_CHATGPT_CMD_SEND, (void *)p_scene, sizeof(p_scene));

        if (p_iterator->p_next_node == chatgpt_scene_list)
        {
            return;
        }

        index++;
    }
}


uint8_t chatgpt_voice_data_send(_chatgpt_scene_thread *p_scene, uint8_t *pbuf,
                                uint16_t payload_length)
{
    if (p_scene == NULL)
    {
        chatgpt_log("[chatgpt] DSP encoder scene null");
        return 0;
    }
    uint16_t L1_payload_len = 0;
    uint16_t offset = 8;
    data_send_buffer[offset + 0] = WATCH_CHATGPT_COMMAND_ID;
    data_send_buffer[offset + 1] = 0x00;//version
    data_send_buffer[offset + 2] = CHATGPT_CMD_ACTION_REQ_DATA;//key
    //payload length
    data_send_buffer[offset + 3] = ((payload_length) >> 8) & 0xff;
    data_send_buffer[offset + 4] = ((payload_length) >> 0) & 0xff;

    //payload, contain one block
    //data_send_buffer[offset + 5] = payload_length;
    memcpy(data_send_buffer + offset + 5, pbuf, payload_length);

    //L1 payload length
    L1_payload_len = payload_length + 5 + offset;

    p_scene->send_list = chatgpt_queue_add_data(p_scene->send_list, data_send_buffer, L1_payload_len);

    event_bus_publish(EVENT_BUS_TOPIC_CHATGPT_CMD_SEND, (void *)p_scene, sizeof(p_scene));
    return 1;
}

/**===============================================================================
*
*                           Text receive for chatgpt
*
*================================================================================*/
uint8_t chatgpt_task_ble_rev_text_handle(_chatgpt_scene_thread *p_scene)
{
    if (p_scene == NULL)
    {
        chatgpt_log("[chatgpt] chatgpt_task_ble_rev_text_handle scene null");
        return 0;
    }

    if (p_scene->text_rev_list == NULL)
    {
        chatgpt_log("[chatgpt] chatgpt_task_ble_rev_text_handle text list null");
        return 0;
    }

    chatgpt_queue_t *p_last_node = chatgpt_queue_indexof_last(p_scene->text_rev_list);
    while (p_last_node != NULL)
    {
        ///chatgpt_format_rev_text(p_last_node->p_data, p_last_node->data_length, &p_rev_data);
        if (p_scene->pAppRevCB)
        {
            p_scene->pAppRevCB(EVENT_BUS_TOPIC_CHATGPT_REV_TEXT_DATA, p_last_node->p_data,
                               p_last_node->data_length);
        }
        p_scene->text_rev_list = chatgpt_queue_remove_last_node(p_scene->text_rev_list);

        p_last_node = chatgpt_queue_indexof_last(p_scene->text_rev_list);
        //chatgpt_log("[ChatGPT] p_last_node->data_length %d, p_rev_data->line_num %d", p_last_node->data_length, p_rev_data.line_num);
    }

    return 1;
}

/**===============================================================================
*
*                           Image Data Handler
*
*================================================================================*/

uint8_t chatgpt_task_ble_rev_image_handle(_chatgpt_scene_thread *p_scene)
{
    if (p_scene == NULL)
    {
        chatgpt_log("[chatgpt] chatgpt_task_ble_rev_image_handle scene null");
        return 0;
    }

    if (p_scene->image_rev_list == NULL)
    {
        chatgpt_log("[chatgpt] chatgpt_task_ble_rev_image_handle list null");
        return 0;
    }

    chatgpt_queue_t *p_last_node = chatgpt_queue_indexof_last(p_scene->image_rev_list);
    while (p_last_node != NULL)
    {
        if ((p_scene) && (p_scene->pAppRevCB))
        {
            p_scene->pAppRevCB(EVENT_BUS_TOPIC_CHATGPT_REV_IMAGE_DATA, p_last_node->p_data,
                               p_last_node->data_length);
        }

        p_scene->image_rev_list = chatgpt_queue_remove_last_node(p_scene->image_rev_list);
        p_last_node = chatgpt_queue_indexof_last(p_scene->image_rev_list);
    }

    return 1;
}


uint8_t chatgpt_action_rsp_param_proc(uint8_t key, uint8_t *pBuf, uint16_t length)
{
    chatgpt_log("[ChatGPT] chatgpt_action_rsp_param_proc key %d, msg id %d", key, pBuf[0]);

    uint8_t m_id = pBuf[0];
    _chatgpt_scene_thread *p_scence = chatgpt_scene_find(m_id);
    if (p_scence == NULL)
    {
        chatgpt_log("[ChatGPT] chatgpt_action_rsp_param_proc scene null");
        return 0;
    }

    if (length < 12)
    {
        chatgpt_log("[ChatGPT] Err: chatgpt_action_rsp_param_proc length invalid");
        return 0;
    }

    uint16_t action_type = (pBuf[7] << 8) | pBuf[8];
    uint8_t action_status = pBuf[9];
    uint8_t action_rsp_type = pBuf[10];

    if (action_type != p_scence->action_type)
    {
        chatgpt_log("[chatgpt] type error, action_type %d, p_scence->action_type %d!", action_type,
                    p_scence->action_type);
        return 0;
    }

    p_scence->rsp_data_type2 = action_rsp_type;
    p_scence->action_status = action_status;


    if (action_rsp_type == 0x01) //text
    {
        //parser text param and prepare to receive text
        if (p_scence->pAppRevCB)
        {
            p_scence->pAppRevCB(EVENT_BUS_TOPIC_CHATGPT_REV_TEXT_PREPARE, pBuf, length);
        }

        event_bus_publish(EVENT_BUS_TOPIC_CHATGPT_REV_TEXT_PREPARE, (void *)p_scence, sizeof(p_scence));
    }
    else if (action_rsp_type == 0x02)// voice
    {
        //parser voice param and prepare to receive voice
        if (p_scence->pAppRevCB)
        {
            p_scence->pAppRevCB(EVENT_BUS_TOPIC_CHATGPT_REV_AUDIO_PREPARE, pBuf, length);
        }
        //prepare for voice decoder
        event_bus_publish(EVENT_BUS_TOPIC_CHATGPT_REV_AUDIO_PREPARE, (void *)p_scence, sizeof(p_scence));
    }
    else if (action_rsp_type == 0x04)// picture
    {
        //parser pic param and prepare to receive picture
        if (p_scence->pAppRevCB)
        {
            p_scence->pAppRevCB(EVENT_BUS_TOPIC_CHATGPT_REV_IMAGE_PREPARE, pBuf, length);
        }

        //prepare for image
        event_bus_publish(EVENT_BUS_TOPIC_CHATGPT_REV_IMAGE_PREPARE, (void *)p_scence, sizeof(p_scence));
    }
    else if (action_rsp_type == 0x08)// file
    {

    }

    return 1;
}

uint8_t chatgpt_action_rsp_data_proc(uint8_t key, uint8_t *pBuf, uint16_t length)
{
    uint8_t m_id = pBuf[0];
    _chatgpt_scene_thread *p_scence = chatgpt_scene_find(m_id);
    if (p_scence == NULL)
    {
        chatgpt_log("[ChatGPT] chatgpt_action_rsp_data_proc scene null");
        return 0;
    }

    chatgpt_log("[ChatGPT] chatgpt_action_rsp_data_proc key %d, m_id %d, rsp_type %d", key, pBuf[0],
                p_scence->rsp_data_type2);

    if (p_scence->rsp_data_type2 == CHATGPT_RSP_TYPE_TEXT)
    {
        memcpy(p_scence->time_stamp, pBuf + 1, 6);

        uint16_t l_data_length = length - 8;
        p_scence->text_rev_list = chatgpt_queue_add_data(p_scence->text_rev_list, pBuf + 8,
                                                         l_data_length);

        event_bus_publish(EVENT_BUS_TOPIC_CHATGPT_REV_TEXT_DATA, (void *)p_scence, sizeof(p_scence));
    }


    if (p_scence->rsp_data_type2 == CHATGPT_RSP_TYPE_VOICE)
    {
        if (p_scence->pAppRevCB)
        {
            p_scence->pAppRevCB(EVENT_BUS_TOPIC_CHATGPT_REV_AUDIO_DATA, pBuf, length);
        }


        uint16_t l_data_length = length - 8;
        APP_PRINT_INFO2("[ChatGPT] audio recv data len = %d,  %b", l_data_length, TRACE_BINARY(10,
                        pBuf + 8));

        p_scence->audio_rev_list = chatgpt_queue_add_data(p_scence->audio_rev_list, pBuf + 8,
                                                          l_data_length);


        event_bus_publish(EVENT_BUS_TOPIC_CHATGPT_REV_AUDIO_DATA, (void *)p_scence, sizeof(p_scence));

    }

    if (p_scence->rsp_data_type2 == CHATGPT_RSP_TYPE_IMAGE)
    {
        uint16_t l_data_length = length - 8;
        p_scence->image_rev_list = chatgpt_queue_add_data(p_scence->image_rev_list, pBuf + 8,
                                                          l_data_length);

        event_bus_publish(EVENT_BUS_TOPIC_CHATGPT_REV_IMAGE_DATA, (void *)p_scence, sizeof(p_scence));

    }

    //if (p_current_action->rsp_data_type == CHATGPT_RSP_TYPE_FILE)
    {
    }


    return 1;
}

uint8_t chatgpt_action_rsp_complete_proc(uint8_t key, uint8_t *pBuf, uint16_t length)
{
    chatgpt_log("[ChatGPT] chatgpt_action_rsp_data_proc key %d, length %d", key, length);

    uint8_t m_id = pBuf[0];
    _chatgpt_scene_thread *p_scene = chatgpt_scene_find(m_id);
    if (p_scene == NULL)
    {
        chatgpt_log("[chatgpt] chatgpt_action_rsp_complete_proc scene null");
        return 0;
    }


    uint8_t rsp_type = pBuf[7];
    chatgpt_log("[ChatGPT] chatgpt_action_rsp_complete_proc rsp_type %d", rsp_type);
    if (rsp_type == CHATGPT_RSP_TYPE_TEXT)
    {
        if (p_scene->pAppRevCB)
        {
            p_scene->pAppRevCB(EVENT_BUS_TOPIC_CHATGPT_REV_TEXT_COMPLETE, pBuf, length);
        }

        event_bus_publish(EVENT_BUS_TOPIC_CHATGPT_REV_TEXT_COMPLETE, (void *)p_scene, sizeof(p_scene));
    }

    else if (rsp_type == CHATGPT_RSP_TYPE_VOICE)
    {
        if (p_scene->pAppRevCB)
        {
            p_scene->pAppRevCB(EVENT_BUS_TOPIC_CHATGPT_REV_AUDIO_COMPLETE, pBuf, length);
        }
        event_bus_publish(EVENT_BUS_TOPIC_CHATGPT_REV_AUDIO_COMPLETE, (void *)p_scene, sizeof(p_scene));
    }

    else if (rsp_type == CHATGPT_RSP_TYPE_IMAGE)
    {
        if (p_scene->pAppRevCB)
        {
            p_scene->pAppRevCB(EVENT_BUS_TOPIC_CHATGPT_REV_IMAGE_COMPLETE, pBuf, length);
        }
        event_bus_publish(EVENT_BUS_TOPIC_CHATGPT_REV_IMAGE_COMPLETE, (void *)p_scene, sizeof(p_scene));
    }

    else if (rsp_type == CHATGPT_RSP_TYPE_FILE)
    {
        // event_bus_publish(EVENT_BUS_TOPIC_CHATGPT_REV_FILE_COMPLETE, (void *)p_scene, sizeof(p_scene));
    }


    return 1;
}

/**===============================================================================
*
*                           BLE Protocol Data Receive
*
*================================================================================*/
uint8_t chatgpt_ble_rev_proc(uint8_t key, uint8_t *pBuf, uint16_t length)
{
    //new common command
    if (key == CHATGPT_EVENT_ACTION_RSP_PARAM)
    {
        chatgpt_action_rsp_param_proc(key, pBuf, length);
    }
    else if (key == CHATGPT_EVENT_ACTION_RSP_DATA)
    {
        chatgpt_action_rsp_data_proc(key, pBuf, length);
    }
    else if (key == CHATGPT_EVENT_ACTION_RSP_COMPLETE)
    {
        chatgpt_action_rsp_complete_proc(key, pBuf, length);
    }
    return 1;
}

/**===============================================================================
*
*                           Task massage Handler
*
*================================================================================*/
int32_t chatgpt_port_task_io_message(T_EVENT_BUS_EVENT_DATA *event_data)
{
    const char *topic = event_data->topic;

    chatgpt_log("[ChatGPT] chatgpt_port_task_io_message topic %s, data 0x%p, data_len %d",
                event_data->topic, event_data->data, event_data->data_len);

    if (strcmp(topic, EVENT_BUS_TOPIC_CHATGPT_CMD_SEND) == 0)
    {
        chatgpt_ble_buffer_send((_chatgpt_scene_thread *)event_data->data);
    }
    //new command

    //text
    if (strcmp(topic, EVENT_BUS_TOPIC_CHATGPT_REV_TEXT_PREPARE) == 0)
    {
        _chatgpt_scene_thread *p_scene = (_chatgpt_scene_thread *)event_data->data;
        chatgpt_queue_clear(&p_scene->text_rev_list);
    }

    if (strcmp(topic, EVENT_BUS_TOPIC_CHATGPT_REV_TEXT_DATA) == 0)
    {
        _chatgpt_scene_thread *p_scene = (_chatgpt_scene_thread *)event_data->data;
        chatgpt_task_ble_rev_text_handle(p_scene);
    }

    if (strcmp(topic, EVENT_BUS_TOPIC_CHATGPT_REV_TEXT_COMPLETE) == 0)
    {
        _chatgpt_scene_thread *p_scene = (_chatgpt_scene_thread *)event_data->data;
    }

    //audio
    if (strcmp(topic, EVENT_BUS_TOPIC_CHATGPT_REV_AUDIO_PREPARE) == 0)
    {
        _chatgpt_scene_thread *p_scene = (_chatgpt_scene_thread *)event_data->data;
        chatgpt_queue_clear(&p_scene->audio_rev_list);
        chatgpt_play_start(p_scene);
    }
    if (strcmp(topic, EVENT_BUS_TOPIC_CHATGPT_REV_AUDIO_DATA) == 0)
    {
        // _chatgpt_scene_thread *p_scene = (_chatgpt_scene_thread *)event_data->data;
        // chatgpt_task_ble_rev_audio_handle(p_scene);
    }

    if (strcmp(topic, EVENT_BUS_TOPIC_CHATGPT_REV_AUDIO_COMPLETE) == 0)
    {
        _chatgpt_scene_thread *p_scene = (_chatgpt_scene_thread *)event_data->data;
        chatgpt_set_voice_recv_status(true);
    }

    //image
    if (strcmp(topic, EVENT_BUS_TOPIC_CHATGPT_REV_IMAGE_PREPARE) == 0)
    {
        _chatgpt_scene_thread *p_scene = (_chatgpt_scene_thread *)event_data->data;
        chatgpt_queue_clear(&p_scene->image_rev_list);
    }

    if (strcmp(topic, EVENT_BUS_TOPIC_CHATGPT_REV_IMAGE_DATA) == 0)
    {
        _chatgpt_scene_thread *p_scene = (_chatgpt_scene_thread *)event_data->data;
        //handler image data
        chatgpt_task_ble_rev_image_handle(p_scene);
    }

    if (strcmp(topic, EVENT_BUS_TOPIC_CHATGPT_VOICE_START) == 0)
    {
        uint8_t m_id = *(uint8_t *)event_data->data;
        chatgpt_log("[chatgpt] CHATGPT_MSG_VOICE_START mid %d", m_id);
        _chatgpt_scene_thread *p_scene = chatgpt_scene_find(m_id);
        if (p_scene == NULL)
        {
            chatgpt_log("[chatgpt] CHATGPT_MSG_VOICE_START p_scene null");
            return -1;
        }

        //req command
        uint16_t offset = 8;
        data_send_buffer[offset + 0] = WATCH_CHATGPT_COMMAND_ID;
        data_send_buffer[offset + 1] = 0x00;//version
        data_send_buffer[offset + 2] = CHATGPT_CMD_ACTION_REQ_START;//key
        uint16_t length = 1/*Message ID*/ +
                          2/*action type*/ +
                          1/*rsp data type*/ +
                          1/*Codec Type*/ +
                          1/* channel */ +
                          4/*sample rate*/;
        data_send_buffer[offset + 3] = (length >> 8) & 0xff;
        data_send_buffer[offset + 4] = (length >> 0) & 0xff;
        data_send_buffer[offset + 5] = m_id;//massage id
        data_send_buffer[offset + 6] = (p_scene->action_type >> 8) & 0xff;//action type
        data_send_buffer[offset + 7] = (p_scene->action_type >> 0) & 0xff;
        data_send_buffer[offset + 8] = p_scene->rsp_data_type1;//rsp data type
        data_send_buffer[offset + 9] =
            0x00;//(p_scene->encoder_type == AUDIO_FORMAT_TYPE_OPUS)? 0x00:0xff;//codec type
        data_send_buffer[offset + 10] = 1;//channel num
        //16K
        data_send_buffer[offset + 11] = 0x00;//sample rate
        data_send_buffer[offset + 12] = 0x00;//sample rate
        data_send_buffer[offset + 13] = 0x3E;//sample rate
        data_send_buffer[offset + 14] = 0x80;//sample rate

        uint16_t L1_payload_len = offset + 5 + length;
        p_scene->send_list = chatgpt_queue_add_data(p_scene->send_list, data_send_buffer,
                                                    L1_payload_len);

        if (event_bus_publish(EVENT_BUS_TOPIC_CHATGPT_CMD_SEND, (void *)p_scene, sizeof(p_scene)) == 0)
        {
            chatgpt_record_init(p_scene);
            chatgpt_start_record();
        }
    }

    //new msg type
    if (strcmp(topic, EVENT_BUS_TOPIC_CHATGPT_VOICE_START) == 0)
    {

    }

    if (strcmp(topic, EVENT_BUS_TOPIC_CHATGPT_VOICE_STOP) == 0)
    {
        uint8_t m_id = *(uint8_t *)event_data->data;
        _chatgpt_scene_thread *p_scene = chatgpt_scene_find(m_id);
        if (p_scene == NULL)
        {
            chatgpt_log("[chatgpt] CHATGPT_MSG_VOICE_STOP p_scene null");
            return -1;
        }

        //req command
        uint16_t offset = 8;
        data_send_buffer[offset + 0] = WATCH_CHATGPT_COMMAND_ID;
        data_send_buffer[offset + 1] = 0x00;//version
        data_send_buffer[offset + 2] = CHATGPT_CMD_ACTION_REQ_COMPLETE;//key
        uint16_t length = 1/*Message ID*/ +
                          1/*end state*/ ;
        data_send_buffer[offset + 3] = (length >> 8) & 0xff;
        data_send_buffer[offset + 4] = (length >> 0) & 0xff;
        data_send_buffer[offset + 5] = m_id;//massage id
        data_send_buffer[offset + 6] = 0x00;//end voice

        uint16_t L1_payload_len = offset + 5 + length;
        p_scene->send_list = chatgpt_queue_add_data(p_scene->send_list, data_send_buffer,
                                                    L1_payload_len);

        if (event_bus_publish(EVENT_BUS_TOPIC_CHATGPT_CMD_SEND, (void *)p_scene, sizeof(p_scene)) == 0)
        {
            chatgpt_stop_record();
        }
    }

    if (strcmp(topic, EVENT_BUS_TOPIC_CHATGPT_VOICE_STOP) == 0)
    {

    }

    if (strcmp(topic, EVENT_BUS_TOPIC_CHATGPT_APP_CONFIRM_IND) == 0)
    {
        uint32_t param = (uint32_t)event_data->data;
        uint8_t m_id = (param >> 24) & 0xff;
        uint8_t m_result = (param >> 16) & 0xff;
        uint16_t m_next_scene = (param >> 0) & 0xffff;

        _chatgpt_scene_thread *p_scene = chatgpt_scene_find(m_id);
        if (p_scene == NULL)
        {
            chatgpt_log("[chatgpt] CHATGPT_MSG_APP_CONFIRM_IND scene null");
            return -1;
        }
        p_scene->action_type = m_next_scene;

        chatgpt_log("[chatgpt] CHATGPT_MSG_APP_CONFIRM_IND m_id %d, m_result %d, m_next_scene %d",
                    m_id, m_result, m_next_scene);
        //req command
        uint16_t offset = 8;
        data_send_buffer[offset + 0] = WATCH_CHATGPT_COMMAND_ID;
        data_send_buffer[offset + 1] = 0x00;//version
        data_send_buffer[offset + 2] = CHATGPT_CMD_ACTION_CONFIRM;//key
        uint16_t length = 1/*Message ID*/ +
                          6/*time stamp*/ +
                          1/*confirm type*/ +
                          2 /*next action type*/;
        data_send_buffer[offset + 3] = (length >> 8) & 0xff;
        data_send_buffer[offset + 4] = (length >> 0) & 0xff;
        data_send_buffer[offset + 5] = m_id;//massage id

        memcpy(data_send_buffer + offset + 6, p_scene->time_stamp, 6);


        data_send_buffer[offset + 12] = m_result;//confirm type

        data_send_buffer[offset + 13] = (m_next_scene >> 8) & 0xff;//next action type
        data_send_buffer[offset + 14] = (m_next_scene >> 0) & 0xff;

        uint16_t L1_payload_len = offset + 5 + length;
        p_scene->send_list = chatgpt_queue_add_data(p_scene->send_list, data_send_buffer,
                                                    L1_payload_len);

        event_bus_publish(EVENT_BUS_TOPIC_CHATGPT_CMD_SEND, (void *)p_scene, sizeof(p_scene));
    }

    return 0;
}

