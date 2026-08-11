/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if CONFIG_REALTEK_FINDMY_FEATURE_SUPPORT
#include "app_msg.h"
#include "fmna_platform_includes.h"
#include "peer_manager.h"
#include "fmna_connection.h"

//static ble_conn_state_user_flag_id_t m_flag_conn_excluded = BLE_CONN_STATE_USER_FLAG_INVALID;
/**< User flag indicating whether a connection is excluded from being handled by the Peer Manager. */

typedef struct
{
    uint16_t conn_handle;
    bool exclude;
} conn_exclude_list;

static conn_exclude_list exclude_list[FMNA_BLE_CONN_BUFFER_SIZE] = {0};

void pm_exclude_list_init(void)
{
    for (int i = 0; i < 8; i++)
    {
        exclude_list[i].conn_handle = 0xFF;
        exclude_list[i].exclude = false;
    }
}

fmna_ret_code_t pm_conn_exclude_add(uint16_t conn_handle)
{
    uint8_t temp_index = 0xFF;
    for (int i = 0; i < FMNA_BLE_CONN_BUFFER_SIZE; i++)
    {
        if (exclude_list[i].conn_handle == conn_handle)
        {
            exclude_list[i].exclude = true;
            return 0;
        }
        else if (exclude_list[i].conn_handle == 0xFF)
        {
            temp_index = i;
        }
    }
    exclude_list[temp_index].conn_handle = conn_handle;
    exclude_list[temp_index].exclude = false;
    if (temp_index == 0xFF)
    {
        return 1;
    }
    return 0;
}

fmna_ret_code_t pm_conn_exclude_delete(uint16_t conn_handle)
{
    for (int i = 0; i < FMNA_BLE_CONN_BUFFER_SIZE; i++)
    {
        if (exclude_list[i].conn_handle == conn_handle)
        {
            exclude_list[i].exclude = false;
            exclude_list[i].conn_handle = 0xFF;
            return 0;
        }
    }
    return 1;
}

bool pm_is_conn_handle_excluded(T_IO_MSG *p_gap_msg)
{
    if (p_gap_msg->subtype == GAP_MSG_LE_CONN_STATE_CHANGE)
    {
        T_LE_GAP_MSG gap_msg;
        memcpy(&gap_msg, &p_gap_msg->u.param, sizeof(p_gap_msg->u.param));
        uint8_t conn_id = gap_msg.msg_data.gap_conn_state_change.conn_id;
        pm_conn_exclude_add(conn_id);
        for (int i = 0; i < FMNA_BLE_CONN_BUFFER_SIZE; i++)
        {
            if (conn_id == exclude_list[i].conn_handle)
            {
                return exclude_list[i].exclude;
            }
        }
    }
    return true;
}
#endif

