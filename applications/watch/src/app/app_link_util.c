/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <string.h>
#include "os_mem.h"
#include "remote.h"
#include "app_main.h"
#include "app_cfg.h"
#include "gap_conn_le.h"


uint32_t app_connected_profiles(void)
{
    uint32_t i, connected_profiles = 0;

    for (i = 0; i < MAX_BR_LINK_NUM; i++)
    {
        connected_profiles |= app_db.br_link[i].connected_profile;
    }
    return connected_profiles;
}


uint8_t app_connected_profile_link_num(uint32_t profile_mask)
{
    uint8_t i, link_number = 0;

    for (i = 0; i < MAX_BR_LINK_NUM; i++)
    {
        if (app_db.br_link[i].connected_profile & profile_mask)
        {
            link_number++;
        }

    }
    return link_number;
}

uint8_t app_find_sco_conn_num(void)
{
    uint8_t i;
    uint8_t num = 0;
    for (i = 0; i < MAX_BR_LINK_NUM; i++)
    {
        if (app_db.br_link[i].used == true &&
            app_db.br_link[i].sco_handle != 0)
        {
            num++;
        }
    }
    return num;
}

T_APP_BR_LINK *app_find_br_link(uint8_t *bd_addr)
{
    T_APP_BR_LINK *p_link = NULL;
    uint8_t        i;

    if (bd_addr != NULL)
    {
        for (i = 0; i < MAX_BR_LINK_NUM; i++)
        {
            if (app_db.br_link[i].used == true &&
                !memcmp(app_db.br_link[i].bd_addr, bd_addr, 6))
            {
                p_link = &app_db.br_link[i];
                break;
            }
        }
    }

    return p_link;
}

T_APP_BR_LINK *app_find_br_link_by_tts_handle(T_TTS_HANDLE handle)
{
    T_APP_BR_LINK *p_link = NULL;
    uint8_t        i;

    if (handle != NULL)
    {
        for (i = 0; i < MAX_BR_LINK_NUM; i++)
        {
            if ((app_db.br_link[i].used == true) &&
                (app_db.br_link[i].tts_handle == handle))
            {
                p_link = &app_db.br_link[i];
                break;
            }
        }
    }

    return p_link;
}

T_APP_BR_LINK *app_alloc_br_link(uint8_t *bd_addr)
{
    T_APP_BR_LINK *p_link = NULL;
    uint8_t        i;

    if (bd_addr != NULL)
    {
        for (i = 0; i < MAX_BR_LINK_NUM; i++)
        {
            if (app_db.br_link[i].used == false)
            {
                p_link = &app_db.br_link[i];

                p_link->used = true;
                p_link->id   = i;
                memcpy(p_link->bd_addr, bd_addr, 6);
                break;
            }
        }
    }

    return p_link;
}

bool app_free_br_link(T_APP_BR_LINK *p_link)
{
    if (p_link != NULL)
    {
        if (p_link->used == true)
        {
            if (p_link->tts_frame_buf != NULL)
            {
                os_mem_free(p_link->tts_frame_buf);
            }

            if (p_link->p_gfps_cmd != NULL)
            {
                os_mem_free(p_link->p_gfps_cmd);
            }

            memset(p_link, 0, sizeof(T_APP_BR_LINK));
            return true;
        }
    }

    return false;
}


T_APP_LE_LINK *app_find_le_link_by_conn_id(uint8_t conn_id)
{
    T_APP_LE_LINK *p_link = NULL;
    uint8_t        i;

    for (i = 0; i < MAX_BLE_LINK_NUM; i++)
    {
        if (app_db.le_link[i].used == true &&
            app_db.le_link[i].conn_id == conn_id)
        {
            p_link = &app_db.le_link[i];
            break;
        }
    }

    return p_link;
}

T_APP_LE_LINK *app_find_le_link_by_addr(uint8_t *bd_addr)
{
    T_APP_LE_LINK *p_link = NULL;
    uint8_t        i;

    if (bd_addr != NULL)
    {
        for (i = 0; i < MAX_BLE_LINK_NUM; i++)
        {
            if (app_db.le_link[i].used == true &&
                !memcmp(app_db.le_link[i].bd_addr, bd_addr, 6))
            {
                p_link = &app_db.le_link[i];
                break;
            }
        }
    }

    return p_link;
}

T_APP_LE_LINK *app_find_le_link_by_tts_handle(T_TTS_HANDLE handle)
{
    T_APP_LE_LINK *p_link = NULL;
    uint8_t        i;

    if (handle != NULL)
    {
        for (i = 0; i < MAX_BLE_LINK_NUM; i++)
        {
            if ((app_db.le_link[i].used == true) &&
                (app_db.le_link[i].tts_handle == handle))
            {
                p_link = &app_db.le_link[i];
                break;
            }
        }
    }

    return p_link;
}


T_APP_LE_LINK *app_alloc_le_link_by_conn_id(uint8_t conn_id)
{
    T_APP_LE_LINK *p_link = NULL;
    uint8_t        i;

    for (i = 0; i < MAX_BLE_LINK_NUM; i++)
    {
        if (app_db.le_link[i].used == false)
        {
            p_link = &app_db.le_link[i];

            p_link->used    = true;
            p_link->id      = i;
            p_link->conn_id = conn_id;
            break;
        }
    }

    return p_link;
}

bool app_free_le_link(T_APP_LE_LINK *p_link)
{
    if (p_link != NULL)
    {
        if (p_link->used == true)
        {
            if (p_link->tts_frame_buf != NULL)
            {
                os_mem_free(p_link->tts_frame_buf);
            }
            if (p_link->p_embedded_cmd != NULL)
            {
                os_mem_free(p_link->p_embedded_cmd);
            }

            if (p_link->eq_data_buf != NULL)
            {
                os_mem_free(p_link->eq_data_buf);
            }

            while (p_link->disc_cb_list.count > 0)
            {
                T_LE_DISC_CB_ENTRY *p_entry;
                p_entry = os_queue_out(&p_link->disc_cb_list);
                if (p_entry)
                {
                    os_mem_free(p_entry);
                }
            }
            memset(p_link, 0, sizeof(T_APP_LE_LINK));
            p_link->conn_id = 0xFF;
            return true;
        }
    }

    return false;
}

bool app_disconnect_le_link(T_APP_LE_LINK *p_link, T_LE_LOCAL_DISC_CAUSE disc_cause)
{
    if (p_link != NULL)
    {
        APP_PRINT_TRACE2("app_disconnect_le_link: conn_id %d, disc_cause %d",
                         p_link->conn_id, disc_cause);
        if (le_disconnect(p_link->conn_id) == GAP_CAUSE_SUCCESS)
        {
            p_link->local_disc_cause = disc_cause;
            return true;
        }
    }
    return false;
}

bool app_reg_le_link_disc_cb(uint8_t conn_id, P_FUN_LE_LINK_DISC_CB p_fun_cb)
{
    T_APP_LE_LINK *p_link = app_find_le_link_by_conn_id(conn_id);
    if (p_link != NULL)
    {
        T_LE_DISC_CB_ENTRY *p_entry;
        for (uint8_t i = 0; i < p_link->disc_cb_list.count; i++)
        {
            p_entry = os_queue_peek(&p_link->disc_cb_list, i);
            if (p_entry != NULL && p_entry->disc_callback == p_fun_cb)
            {
                return true;
            }
        }
        p_entry = os_mem_zalloc(OS_MEM_TYPE_DATA, sizeof(T_LE_DISC_CB_ENTRY));
        if (p_entry != NULL)
        {
            p_entry->disc_callback = p_fun_cb;
            os_queue_in(&p_link->disc_cb_list, p_entry);
            return true;
        }
    }
    return false;
}

uint8_t app_get_ble_link_num(void)
{
    uint8_t num = 0;
    uint8_t        i;

    for (i = 0; i < MAX_BLE_LINK_NUM; i++)
    {
        if (app_db.le_link[i].used == true)
        {
            num++;
        }
    }

    return num;
}

uint8_t app_find_b2s_link_num(void)
{
    uint8_t num = 0;
    uint8_t i = 0;

    for (i = 0; i < MAX_BR_LINK_NUM; i++)
    {
        if (app_db.br_link[i].used)
        {
            num++;
        }
    }
    return num;
}
