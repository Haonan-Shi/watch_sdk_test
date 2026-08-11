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
#include "chatgpt_app.h"
#include "app_msg.h"
#include "string.h"
#include "event_bus.h"
#include "app_module_init.h"

/*============================================================================*
*                              Local Variables
*============================================================================*/
_chatgpt_rev_data_t g_chatgpt_rev_text = {0};

uint8_t app_id = 0xff;//invalid

static _chatgpt_status g_chatgpt_status = CHATGPT_STATUS_IDLE;

static uint16_t image_offset = 0;
uint8_t *iamge_data;
static T_EVENT_BUS_SUBSCRIBER_HANDLE chatgpt_async_handle;
/*============================================================================*
*                              Local Functions
*============================================================================*/
static uint8_t chatgpt_check_char(uint8_t c);
static uint8_t chatgpt_get_single_word(uint8_t *p_buf, uint32_t buf_len, uint32_t *p_start_index);
static void chatgpt_format_rev_text(uint8_t *input_data, uint32_t input_length,
                                    _chatgpt_rev_data_t *p_rev_text);
static void chatgpt_app_data_rev_cb(const char *topic, void *data, uint16_t length);

static void chatgpt_module_init(void)
{
    extern int32_t chatgpt_port_task_io_message(T_EVENT_BUS_EVENT_DATA * event_data);

    event_bus_topic_register(EVENT_BUS_TOPIC_CHATGPT_ALL_TOPIC);
    event_bus_subscribe_async(&chatgpt_async_handle,
                              EVENT_BUS_TOPIC_CHATGPT_ALL_TOPIC,
                              event_bus_async_send_to_apptask,
                              NULL,
                              chatgpt_port_task_io_message);
}
APP_MODULE_INIT(chatgpt_module_init);

/**
*  app voice start
*/
void chatgpt_app_voice_start(void)
{
    //stop the last voice flow
//    void chatgpt_voice_stop(void);
//    chatgpt_voice_stop();

//  bool ret = chatgpt_scene_delete(app_id);
//  chatgpt_log("[chatgpt] chatgpt_app_voice_start ret %d", ret);

    //S1: Create app scene
    app_id = chatgpt_scene_create(CHATGPT_SCENE_SPEECH_RECOGNITION,
                                  CHATGPT_RSP_TYPE_TEXT | CHATGPT_RSP_TYPE_VOICE, chatgpt_app_data_rev_cb);
    if (app_id == 0xff)
    {
        chatgpt_log("[chatgpt] invalid appid");
        return;
    }
    chatgpt_log("[chatgpt] chatgpt_app_voice_start app_id %d, %d", app_id, *(uint8_t *)&app_id);

    //S2: send voice start command
    event_bus_publish(EVENT_BUS_TOPIC_CHATGPT_VOICE_START, (void *)&app_id, sizeof(app_id));


    g_chatgpt_status = CHATGPT_STATUS_RECORDING;

}

void chatgpt_app_voice_stop(void)
{
    chatgpt_log("[chatgpt] chatgpt_app_voice_stop");
    //stop the last voice flow
    event_bus_publish(EVENT_BUS_TOPIC_CHATGPT_VOICE_STOP, (void *)&app_id, sizeof(app_id));

    g_chatgpt_status = CHATGPT_STATUS_RECORD_END;
}

void chatgpt_app_confirm_ind(uint8_t id, e_chatgpt_confirm_type type, e_chatgpt_action next_scene)
{
    //send confirm command
    uint32_t param = (id << 24) | ((type << 16) & 0xff00) | (next_scene);
    event_bus_publish(EVENT_BUS_TOPIC_CHATGPT_APP_CONFIRM_IND, (void *)&param, sizeof(param));
}


void chatgpt_app_data_rev_cb(const char *topic, void *data, uint16_t length)
{
    chatgpt_log("[chatgpt] topic %s, g_chatgpt_status %d", topic, g_chatgpt_status);
    if (strcmp(topic, EVENT_BUS_TOPIC_CHATGPT_REV_TEXT_PREPARE) == 0)
    {

        if (CHATGPT_STATUS_RECORD_END == g_chatgpt_status)
        {
            // text by voice

            //clear text
            if (g_chatgpt_rev_text.p_data != NULL)
            {
                chatgpt_free(g_chatgpt_rev_text.p_data);
                g_chatgpt_rev_text.p_data = NULL;
            }
            memset(&g_chatgpt_rev_text, 0, sizeof(g_chatgpt_rev_text));

            g_chatgpt_status = CHATGPT_STATUS_WAITING_TXT_RSP;
        }

        else if (CHATGPT_STATUS_WHISPER_TXT == g_chatgpt_status)
        {
            g_chatgpt_status = CHATGPT_STATUS_REV_GPT_TEXT;

            uint8_t str[3] = {0x0A, 0x0A};
            chatgpt_format_rev_text(str, 2, &g_chatgpt_rev_text);

        }
    }

    else if (strcmp(topic, EVENT_BUS_TOPIC_CHATGPT_REV_TEXT_DATA) == 0)
    {
        if (CHATGPT_STATUS_WAITING_TXT_RSP == g_chatgpt_status)
        {
            // text by voice
            {
                chatgpt_format_rev_text(data, length, &g_chatgpt_rev_text);
            }
        }

        else if (CHATGPT_STATUS_REV_GPT_TEXT == g_chatgpt_status)
        {
            // text by voice
            {
                chatgpt_format_rev_text(data, length, &g_chatgpt_rev_text);
            }
        }
    }

    else if (strcmp(topic, EVENT_BUS_TOPIC_CHATGPT_REV_TEXT_COMPLETE) == 0)
    {
        //chatgpt_format_rev_text(data, length, &g_chatgpt_rev_text);
        if (CHATGPT_STATUS_WAITING_TXT_RSP == g_chatgpt_status)
        {
            //whisper text receive end
            chatgpt_app_confirm_ind(app_id, CHATGPT_CONFIRM_SUCCESS, CHATGPT_SCENE_AI_CHAT);
            g_chatgpt_status = CHATGPT_STATUS_WHISPER_TXT;

            // chatgpt_app_confirm_ind(app_id, CHATGPT_CONFIRM_SUCCESS, CHATGPT_SCENE_GENERATE_PICTURE);
            // g_chatgpt_status = CHATGPT_STATUS_WHISPER_TXT;

        }
        else if (CHATGPT_STATUS_WHISPER_TXT == g_chatgpt_status)
        {
            chatgpt_log("[chatgpt] aichat text received");
        }

        else if (CHATGPT_STATUS_REV_GPT_TEXT == g_chatgpt_status)
        {
            chatgpt_log("[chatgpt] aichat gpt answer text received");
        }

    }

    else if (strcmp(topic, EVENT_BUS_TOPIC_CHATGPT_REV_AUDIO_PREPARE) == 0)
    {
        chatgpt_log("[chatgpt] aichat audio received");
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_CHATGPT_REV_AUDIO_DATA) == 0)
    {
        chatgpt_log("[chatgpt] aichat audio data");
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_CHATGPT_REV_AUDIO_COMPLETE) == 0)
    {
        chatgpt_log("[chatgpt] aichat audio completed");

    }


    else if (strcmp(topic, EVENT_BUS_TOPIC_CHATGPT_REV_IMAGE_PREPARE) == 0)
    {
        chatgpt_log("[chatgpt] aichat image prepare");
        g_chatgpt_status = CHATGPT_STATUS_REV_IMAGE_PARAM;

        image_offset = 0;

        {
            uint16_t w = 296;
            uint16_t h = 321;

            if (iamge_data == NULL)
            {
                iamge_data = chatgpt_malloc(w * h * 2 + 8);
            }
            uint8_t *p_data = (uint8_t *)iamge_data;
            p_data[0] = 0x00;
            p_data[1] = 0x00;
            p_data[2] = w & 0xff;
            p_data[3] = (w >> 8) & 0xff;
            p_data[4] = (h >> 0) & 0xff;
            p_data[5] = (h >> 8) & 0xff;
            p_data[6] = 0x00;
            p_data[7] = 0x00;


//          gui_rgb_data_head_t *p_data = (gui_rgb_data_head_t *)img_data;
//          p_data->scan = 0;
//          p_data->align = 0;
//          p_data->resize = 0;
//          p_data->compress = 0;
//          p_data->rsvd = 0;
//          p_data->type = RGB565;
//          p_data->w = image_wsize;
//          p_data->h = image_hsize;
//          p_data->version = 0;
//          p_data->rsvd2 = 0;

            memset(p_data, 0xff, w * h * 2);
        }
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_CHATGPT_REV_IMAGE_DATA) == 0)
    {
        //chatgpt_log("[chatgpt] aichat image data, length %d", length);
        g_chatgpt_status = CHATGPT_STATUS_REV_IMAGE_DATA;

        //APP_PRINT_INFO1("[chatgpt] aichat: %b", TRACE_BINARY(length % 244, (uint8_t *)data + 244 * index));
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_CHATGPT_REV_IMAGE_COMPLETE) == 0)
    {
        chatgpt_log("[chatgpt] aichat image completed");

        g_chatgpt_status = CHATGPT_STATUS_REV_IMAGE_COMPLETE;
    }

}


_chatgpt_status chatgpt_app_get_status(void)
{
    return g_chatgpt_status;
}

/**===============================================================================
*
*                           Text handle for chatgpt rsp
*
*================================================================================*/
/**
* format the receive data into line every SCREEN_WIDTH characters and split with words.
*/
void chatgpt_format_rev_text(uint8_t *input_data, uint32_t input_length,
                             _chatgpt_rev_data_t *p_rev_text)
{
    if (p_rev_text == NULL)
    {
        return;
    }

    if (p_rev_text->p_data == NULL)
    {
        APP_PRINT_INFO0("[ChatGPT] chatgpt_format_rev_text 01");
        uint32_t length = 5 * 1024;
        p_rev_text->p_data = chatgpt_malloc(length);
        memset(p_rev_text->p_data, 0, length);
        p_rev_text->buffer_length = length;
    }
    else
    {
        // resize
        if (p_rev_text->data_length + (input_length << 2) > p_rev_text->buffer_length)
        {
            APP_PRINT_INFO0("[ChatGPT] chatgpt_format_rev_text 02");
            p_rev_text->p_data = chatgpt_realloc(p_rev_text->p_data,
                                                 p_rev_text->data_length + (input_length << 2));
            p_rev_text->buffer_length = p_rev_text->data_length + (input_length << 2);
        }
    }

    p_rev_text->line_width = SCREEN_WIDTH;

    uint8_t *pSrcData = input_data ;
    uint32_t line_index = p_rev_text->write_line_index;
    for (uint32_t index = 0; index < input_length;)
    {
        uint8_t *pdata = p_rev_text->p_data + p_rev_text->write_line_index * p_rev_text->line_width;
        if (p_rev_text->write_row_offset >= (p_rev_text->line_width - 1))
        {
            if ((p_rev_text->write_row_offset != 0) &&
                (pdata[p_rev_text->write_row_offset - 1] == 0x20/*space*/))
            {
                pdata[p_rev_text->write_row_offset - 1] = '\0';
            }
            else
            {
                pdata[p_rev_text->write_row_offset] = '\0';
            }

            p_rev_text->write_line_index ++;
            p_rev_text->write_row_offset = 0;
            pdata += p_rev_text->line_width;
        }

        if (chatgpt_check_char(pSrcData[index]) == 0)//not character
        {
            //if first character is blackspace not copy
            if ((p_rev_text->write_row_offset == 0) && (pSrcData[index] == ' '))
            {
                ;
            }
            else if (pSrcData[index] == 0x0A)//LF
            {
                if ((p_rev_text->write_row_offset != 0) &&
                    (pdata[p_rev_text->write_row_offset - 1] == 0x20/*space*/))
                {
                    pdata[p_rev_text->write_row_offset - 1] = '\0';
                }
                else
                {
                    pdata[p_rev_text->write_row_offset] = '\0';
                }
                p_rev_text->write_row_offset = 0;
                p_rev_text->write_line_index ++;
                pdata[p_rev_text->write_row_offset + p_rev_text->line_width] = '\0';
            }
            else
            {
                pdata[p_rev_text->write_row_offset] = pSrcData[index];
                pdata[p_rev_text->write_row_offset + 1] = '\0';
                p_rev_text->write_row_offset ++;
            }
            index ++;
        }
        else
        {
            uint32_t p_start_index = 0;
            uint32_t cnt = chatgpt_get_single_word(pSrcData + index, input_length - index, &p_start_index);

            if ((p_rev_text->write_row_offset + cnt) >= (p_rev_text->line_width - 1))
            {
                if (cnt > p_rev_text->line_width - 1)
                {
                    cnt = p_rev_text->line_width - 1 - p_rev_text->write_row_offset;    //not
                }
                else
                {
                    if ((p_rev_text->write_row_offset != 0) &&
                        (pdata[p_rev_text->write_row_offset - 1] == 0x20/*space*/))
                    {
                        pdata[p_rev_text->write_row_offset - 1] = '\0';
                    }
                    else
                    {
                        pdata[p_rev_text->write_row_offset] = '\0';
                    }

                    p_rev_text->write_line_index ++;
                    p_rev_text->write_row_offset = 0;
                    pdata += p_rev_text->line_width;
                }
            }

            if (cnt >= p_rev_text->line_width - 1)
            {
                cnt = p_rev_text->line_width - 1;
            }

            memcpy(pdata + p_rev_text->write_row_offset, pSrcData + index + p_start_index, cnt);
            p_rev_text->write_row_offset += cnt;

            index += cnt;

        }

        p_rev_text->line_num = p_rev_text->write_line_index + 1;
    }

    p_rev_text->data_length += (p_rev_text->write_line_index - line_index + 1) * p_rev_text->line_width;
    APP_PRINT_INFO2("[ChatGPT] chatgpt_format_rev_text p_rev_text->line_num %d, p_rev_text->data_length %d",
                    p_rev_text->line_num, p_rev_text->data_length);
}
uint8_t chatgpt_check_char(uint8_t c)
{
    if (((c >= 'a') && (c <= 'z'))
        || ((c >= 'A') && (c <= 'Z'))
        || (c == '\''))
    {
        return 1;
    }
    return 0;
}
uint8_t chatgpt_get_single_word(uint8_t *p_buf, uint32_t buf_len, uint32_t *p_start_index)
{
    uint8_t start_flag = 0;
    uint8_t cnt = 0;
    for (uint32_t index = 0; index < buf_len; index ++)
    {
        char c = p_buf[index];

        if (((c >= 'a') && (c < 'z'))
            || ((c >= 'A') && (c < 'Z')))
        {
            if (start_flag == 0)
            {
                *p_start_index = index;
                start_flag = 1;
                cnt ++;
            }
            else
            {
                cnt ++;
            }

            if (index == (buf_len - 1))
            {
                return cnt;
            }
        }
        else
        {
            if (start_flag == 1)
            {
                return cnt;
            }
        }
    }

    return 0;
}




