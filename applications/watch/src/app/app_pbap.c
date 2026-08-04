/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "os_mem.h"
#include "trace.h"
#include "btm.h"
#include "remote.h"
#include "app_cfg.h"
#include "app_main.h"
#include "app_report.h"
#include "app_hfp.h"
#include "app_linkback.h"
#include "event_bus.h"
#include "app_pbap.h"

// Global variable to store caller name from PBAP (for incoming call display)
static char pbap_caller_name[64] = {0};

#if CONFIG_PHONEBOOK_SYNC_SUPPORT
#define MAX_SEGMENTS      50
#define MAX_CONTACTS_NUM  20
#define MAX_NAME_LEN      50
#define MAX_TEL_LEN       20
#define MAX_HANZI_SHOW    6

typedef struct
{
    char name[MAX_NAME_LEN];
    char tel[MAX_TEL_LEN];
} pbap_tel_dir;

static uint8_t book_size = 0;
static uint16_t start_contacts = 0x01;
static char *segments[MAX_SEGMENTS];
static uint8_t token_name_temp[MAX_NAME_LEN];

uint16_t contacts_count = 0;
pbap_tel_dir save_dir_data[MAX_CONTACTS_NUM];
#endif
static void app_pbap_bt_cback(T_BT_EVENT event_type, void *event_buf, uint16_t buf_len)
{
    T_BT_EVENT_PARAM *param = event_buf;
    bool handle = true;

    switch (event_type)
    {
    case BT_EVENT_PBAP_CONN_CMPL:
        {
            T_APP_BR_LINK *p_link;

            p_link = app_find_br_link(param->pbap_conn_cmpl.bd_addr);
            if (p_link == NULL)
            {
                APP_PRINT_ERROR0("app_pbap_bt_cback: no link found");
                break;
            }
            bt_pbap_phone_book_set(p_link->bd_addr, BT_PBAP_REPOSITORY_LOCAL, BT_PBAP_PATH_PB);
#if CONFIG_PHONEBOOK_SYNC_SUPPORT
            bt_pbap_phone_book_size_get(p_link->bd_addr, BT_PBAP_REPOSITORY_LOCAL, BT_PBAP_PHONE_BOOK_PB);
#endif
            if (app_hfp_get_call_status() != APP_HFP_CALL_IDLE)
            {
                p_link->call_id_type_check = true;
                p_link->call_id_type_num = false;
                bt_hfp_current_call_list_req(param->pbap_conn_cmpl.bd_addr);
            }

        }
        break;

    case BT_EVENT_PBAP_CONN_FAIL:
        {

        }
        break;

    case BT_EVENT_PBAP_CALLER_ID_NAME:
        {
            T_APP_BR_LINK *br_link;
            T_APP_LE_LINK *le_link;

            br_link = app_find_br_link(param->pbap_caller_id_name.bd_addr);
            le_link = app_find_le_link_by_addr(param->pbap_caller_id_name.bd_addr);

            if (br_link != NULL)
            {
                /* Sanity check if BR/EDR TTS session is ongoing */
                if (br_link->cmd_set_enable == true &&
                    br_link->tts_handle != NULL)
                {
                    break;
                }

                /* Sanity check if BLE TTS session is ongoing */
                if (le_link != NULL &&
                    le_link->cmd_set_enable == true &&
                    le_link->tts_handle != NULL)
                {
                    break;
                }

                if (param->pbap_caller_id_name.name_len)
                {
                    APP_PRINT_INFO2("[PBAP] caller_id event: call_id_type_check=%d, call_id_type_num=%d",
                                    br_link->call_id_type_check, br_link->call_id_type_num);

                    if (br_link->call_id_type_check == true)
                    {
                        if (br_link->call_id_type_num == false)
                        {
                            /* Validate name_ptr before any memory access */
                            if (param->pbap_caller_id_name.name_ptr == NULL)
                            {
                                APP_PRINT_ERROR0("[PBAP] caller name_ptr is NULL, discard event");
                                br_link->call_id_type_check = false;
                                br_link->call_id_type_num = false;
                                break;
                            }

                            uint16_t copy_len = param->pbap_caller_id_name.name_len;
                            if (copy_len >= sizeof(pbap_caller_name))
                            {
                                APP_PRINT_WARN2("[PBAP] caller name_len=%d exceeds buffer=%d, truncated",
                                                copy_len, (uint16_t)sizeof(pbap_caller_name));
                                copy_len = sizeof(pbap_caller_name) - 1;
                            }

                            APP_PRINT_INFO1("[PBAP] caller name len = %d", copy_len);

                            memcpy(pbap_caller_name, param->pbap_caller_id_name.name_ptr, copy_len);
                            pbap_caller_name[copy_len] = '\0';

                            /* Validate copied content: reject non-printable characters */
                            bool name_valid = true;
                            for (uint16_t i = 0; i < copy_len; i++)
                            {
                                uint8_t ch = (uint8_t)pbap_caller_name[i];
                                /* Allow printable ASCII and common UTF-8 multibyte (>= 0x80) */
                                if (ch < 0x20)
                                {
                                    name_valid = false;
                                    APP_PRINT_ERROR1("[PBAP] caller name contains invalid char 0x%02X, discard", ch);
                                    pbap_caller_name[0] = '\0';
                                    break;
                                }
                            }

                            if (name_valid)
                            {
                                APP_PRINT_INFO1("[PBAP] caller name = %s", TRACE_STRING(pbap_caller_name));

                                /* Notify phone UI only for incoming calls */
                                if (app_hfp_get_call_status() == APP_HFP_CALL_INCOMING)
                                {
                                    APP_PRINT_INFO0("[PBAP] publishing caller_id to phone UI");
                                    event_bus_publish(EVENT_BUS_TOPIC_PBAP_CALLER_ID, pbap_caller_name,
                                                      strlen(pbap_caller_name) + 1);
                                }
                                else
                                {
                                    APP_PRINT_INFO1("[PBAP] call status=%d, skip UI publish",
                                                    app_hfp_get_call_status());
                                }
                            }
                            br_link->call_id_type_check = false;
                            br_link->call_id_type_num = false;
                        }
                        else
                        {
                            APP_PRINT_INFO0("[PBAP] caller name NOT stored: call_id_type_num is true");
                        }
                    }
                    else
                    {
                        APP_PRINT_INFO0("[PBAP] caller name NOT stored: call_id_type_check is false");
                    }
                }
                else
                {
                    /* name_len == 0: no caller name available, clear buffer */
                    APP_PRINT_INFO0("[PBAP] caller name not available, clearing pbap_caller_name");
                    pbap_caller_name[0] = '\0';
                    br_link->call_id_type_check = false;
                    br_link->call_id_type_num = false;
                }
            }
            else
            {
                APP_PRINT_WARN0("[PBAP] BT_EVENT_PBAP_CALLER_ID_NAME: br_link not found, discard");
            }
        }
        break;
#if CONFIG_PHONEBOOK_SYNC_SUPPORT
    case BT_EVENT_PBAP_GET_PHONE_BOOK_SIZE_CMPL:
        {
            T_APP_BR_LINK *p_link = app_find_br_link(param->pbap_get_phone_book_size_cmpl.bd_addr);
            book_size = param->pbap_get_phone_book_size_cmpl.pb_size;
            uint64_t filter = BT_PBAP_PROPERTY_FN | BT_PBAP_PROPERTY_TEL;
            bt_pbap_phone_book_pull(p_link->bd_addr, BT_PBAP_REPOSITORY_LOCAL, BT_PBAP_PHONE_BOOK_PB,
                                    start_contacts, 0x01, filter);

            APP_PRINT_INFO1("[PBAP] book size = %d", param->pbap_get_phone_book_size_cmpl.pb_size);
            APP_PRINT_INFO1("[PBAP] new missed calls = %d",
                            param->pbap_get_phone_book_size_cmpl.new_missed_calls);
        }
        break;
    case BT_EVENT_PBAP_GET_PHONE_BOOK_CMPL:
        {
            uint8_t segment_count = 0;
            T_APP_BR_LINK *p_link = app_find_br_link(param->pbap_get_phone_book_cmpl.bd_addr);
            char *token = strtok((char *)param->pbap_get_phone_book_cmpl.p_data, "\n");

            APP_PRINT_INFO1("[PBAP] contacts data_end = %d", param->pbap_get_phone_book_cmpl.data_end);
            APP_PRINT_INFO1("[PBAP] contacts data_len = %d", param->pbap_get_phone_book_cmpl.data_len);

            while (token != NULL)
            {
                APP_PRINT_INFO1("[PBAP] contacts p_data = %s", TRACE_STRING((uint8_t *)token));
                if (segment_count < MAX_SEGMENTS)
                {
                    segments[segment_count] = malloc(strlen(token) + 1);
                    if (segments[segment_count] == NULL)
                    {
                        APP_PRINT_ERROR0("PBAP: memory allocation failed!");
                        return;
                    }
                    strcpy(segments[segment_count], token);

                    if (segments[segment_count][0] == 'B')
                    {
                        contacts_count++;
                    }
                    if (segments[segment_count][0] == 'F')
                    {
                        char *token_name = strchr(segments[segment_count], ':');
                        if (token_name != NULL)
                        {
                            token_name ++;
                            if (contacts_count <= MAX_CONTACTS_NUM)
                            {
                                if (token_name[0] == '=')
                                {
                                    char *equals;
                                    while ((equals = strchr(token_name, '=')) != NULL)
                                    {
                                        strcpy(equals, equals + 1);
                                    }
                                    memcpy(token_name_temp, (uint8_t *)token_name, strlen(token_name));
                                    uint8_t i = 0;
                                    uint8_t j = 0;
                                    while (i < (MAX_HANZI_SHOW * 6 - 1 > strlen(token_name) - 1 ? strlen(token_name) - 1 :
                                                MAX_HANZI_SHOW * 6 - 1))
                                    {
                                        if (token_name_temp[i] < 0x40)
                                        {
                                            if ((i + 1 >= MAX_HANZI_SHOW * 6) || (token_name_temp[i + 1] == '\n'))
                                            {
                                                i = i + 2;
                                                break;
                                            }
                                            save_dir_data[contacts_count - 1].name[j] = (token_name_temp[i] - 0x30) * 0x10 +
                                                                                        (token_name_temp[i + 1] - 0x30);
                                            i = i + 2;
                                            j = j + 1;
                                        }
                                        if (token_name_temp[i] > 0x40)
                                        {
                                            if ((i + 5 >= MAX_HANZI_SHOW * 6) || (token_name_temp[i + 5] == '\n'))
                                            {
                                                i = i + 6;
                                                break;
                                            }
                                            save_dir_data[contacts_count - 1].name[j] = (token_name_temp[i] - 0x37) * 0x10 +
                                                                                        (token_name_temp[i + 1] - 0x30);
                                            save_dir_data[contacts_count - 1].name[j + 1] = (token_name_temp[i + 2] > 0x40 ?
                                                                                             (token_name_temp[i + 2] - 0x37) : (token_name_temp[i + 2] - 0x30)) * 0x10
                                                                                            + (token_name_temp[i + 3] > 0x40 ? (token_name_temp[i + 3] - 0x37) : (token_name_temp[i + 3] -
                                                                                                    0x30));

                                            save_dir_data[contacts_count - 1].name[j + 2] = (token_name_temp[i + 4] > 0x40 ?
                                                                                             (token_name_temp[i + 4] - 0x37) : (token_name_temp[i + 4] - 0x30)) * 0x10
                                                                                            + (token_name_temp[i + 5] > 0x40 ? (token_name_temp[i + 5] - 0x37) : (token_name_temp[i + 5] -
                                                                                                    0x30));
                                            i = i + 6;
                                            j = j + 3;
                                        }
                                    }
                                    if (strlen(token_name) > MAX_HANZI_SHOW * 6)
                                    {
                                        *(uint8_t *)(save_dir_data[contacts_count - 1].name + j) = '.';
                                        *(uint8_t *)(save_dir_data[contacts_count - 1].name + j + 1) = '.';
                                        *(uint8_t *)(save_dir_data[contacts_count - 1].name + j + 2) = '.';
                                        *(uint8_t *)(save_dir_data[contacts_count - 1].name + j + 3) = '\0';
                                    }
                                    else
                                    {
                                        *(uint8_t *)(save_dir_data[contacts_count - 1].name + j) = '\0';
                                    }
                                }
                                else
                                {
                                    memcpy(token_name_temp, (uint8_t *)token_name, strlen(token_name) - 1);
                                    uint8_t i = 0;
                                    while (i < (MAX_HANZI_SHOW * 3 - 1))
                                    {
                                        if (token_name_temp[i] > 0xE3)
                                        {
                                            i = i + 3;
                                        }
                                        else
                                        {
                                            i = i + 1;
                                        }
                                    }
                                    memcpy(save_dir_data[contacts_count - 1].name, (uint8_t *)token_name, i);
                                    *(uint8_t *)(save_dir_data[contacts_count - 1].name + i) = '.';
                                    *(uint8_t *)(save_dir_data[contacts_count - 1].name + i + 1) = '.';
                                    *(uint8_t *)(save_dir_data[contacts_count - 1].name + i + 2) = '.';
                                    *(uint8_t *)(save_dir_data[contacts_count - 1].name + i + 3) = '\0';
                                }
                            }
                            APP_PRINT_INFO2("[PBAP] contacts token_name %d = %s", contacts_count,
                                            TRACE_STRING((uint8_t *)token_name));
                        }
                    }
                    if (segments[segment_count][0] == 'T')
                    {
                        char *token_tel = strchr(segments[segment_count], ':');
                        if (token_tel != NULL)
                        {
                            token_tel++;
                            if (contacts_count <= MAX_CONTACTS_NUM)
                            {
                                if (strlen(token_tel) < MAX_TEL_LEN)
                                {
                                    memcpy(save_dir_data[contacts_count - 1].tel, (uint8_t *)token_tel, strlen(token_tel) - 1);
                                    *(uint8_t *)(save_dir_data[contacts_count - 1].tel + (strlen(token_tel) - 1)) = '\0';
                                }
                                else
                                {
                                    APP_PRINT_ERROR0("contacts tel is too log");
                                    memcpy(save_dir_data[contacts_count - 1].tel, (uint8_t *)token_tel, MAX_TEL_LEN);
                                    *(uint8_t *)(save_dir_data[contacts_count - 1].tel + MAX_TEL_LEN - 1) = '\0';
                                }
                            }
                            APP_PRINT_INFO2("[PBAP] contacts token_tel %d = %s", contacts_count,
                                            TRACE_STRING((uint8_t *)token_tel));
                        }
                    }
                    segment_count++;
                }
                else
                {
                    APP_PRINT_ERROR0("[PBAP] contacts segment overflow");
                    break;
                }
                token = strtok(NULL, "\n");
            }
            for (uint8_t i = 0; i < segment_count; i++)
            {
                free(segments[i]);
            }
            if (param->pbap_get_phone_book_cmpl.data_end == 0)
            {
                bt_pbap_pull_continue(p_link->bd_addr);
                break;
            }
            if (start_contacts < ((MAX_CONTACTS_NUM > book_size) ? book_size : MAX_CONTACTS_NUM))
            {
                start_contacts += 0x01;

                uint64_t filter = BT_PBAP_PROPERTY_FN | BT_PBAP_PROPERTY_TEL;
                bt_pbap_phone_book_pull(p_link->bd_addr, BT_PBAP_REPOSITORY_LOCAL, BT_PBAP_PHONE_BOOK_PB,
                                        start_contacts, 0x01, filter);
            }
            else if (start_contacts >= ((MAX_CONTACTS_NUM > book_size) ? book_size : MAX_CONTACTS_NUM))
            {
                start_contacts = 0x01;
            }
        }
        break;
#endif
    default:
        handle = false;
        break;
    }

    if (handle == true)
    {
        APP_PRINT_INFO1("app_pbap_bt_cback: event_type 0x%04x", event_type);
    }
}

void app_pbap_init(void)
{
    if (app_cfg_const.supported_profile_mask & PBAP_PROFILE_MASK)
    {
        bt_pbap_init();
        bt_mgr_cback_register(app_pbap_bt_cback);
    }
}

/**
 * @brief Get caller name reported by PBAP (for incoming call display)
 * @return caller name if available, empty string otherwise
 */
const char *app_pbap_get_caller_name(void)
{
    return pbap_caller_name;
}

/**
 * @brief Clear caller name (should be called when new incoming call starts)
 */
void app_pbap_clear_caller_name(void)
{
    pbap_caller_name[0] = '\0';
}
