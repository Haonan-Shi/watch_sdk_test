/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "string.h"
#include "board.h"
#include "os_timer.h"
#include "os_sched.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rtc.h"
#include "app_msg.h"
#include "rtl876x_wdg.h"
#include "os_sync.h"
//#include "gui_core.h"
#include "hub_task.h"
#include "platform_utils.h"
#include "vector_table.h"
//#include "iotsec.h"
#include "app_dlps.h"
#include "os_mem.h"
#include "app_main.h"
#include "profile_server_def.h"
#include "profile_server.h"
#include "section.h"
#include "upay_service.h"
#include "alipay_ble_transport.h"
#include "alipay_queue.h"

#if CONFIG_ALIPAY

extern T_SERVER_ID upay_gatt_srv_id;
static ali_queue_t *g_alipay_ble_transport_list = NULL;
static uint8_t alipay_connect_id = 0x00;
//static uint8_t alipay_link_status ;
extern RtkWristBandSysType_t RtkWristbandSys;


/**
 * alipay ble send api
 */
void alipay_ble_send(uint16_t handle, uint8_t *data, uint16_t len)
{
    uint8_t alipay_cretids = 0;

    le_get_gap_param(GAP_PARAM_LE_REMAIN_CREDITS, &alipay_cretids);
    APP_PRINT_INFO2("[AliPay] alipay_ble_send len %d,data %b", len, TRACE_BINARY(4, data));
    if (alipay_cretids > 0)
    {
        uint8_t alipay_cretids = 0;
        uint16_t cid;
        uint8_t cid_num;
        uint16_t conn_handle = le_get_conn_handle(alipay_connect_id);

        gap_chann_get_cid(conn_handle, 1, &cid, &cid_num);
        ali_queue_t *p_last_node  = ali_queue_indexof_last(g_alipay_ble_transport_list);
        if (p_last_node == NULL)
        {
            if (RtkWristbandSys.gap_conn_state == GAP_CONN_STATE_CONNECTED)
            {
                gatt_svc_send_data(conn_handle, cid, upay_gatt_srv_id, INDEX_UPAY_WRITE, data, len,
                                   GATT_PDU_TYPE_ANY);
            }
            else
            {
                APP_PRINT_INFO1("[AliPay] alipay_ble_send not connected, state %d", RtkWristbandSys.gap_conn_state);
            }
        }
        else
        {
            if (RtkWristbandSys.gap_conn_state == GAP_CONN_STATE_CONNECTED)
            {
                gatt_svc_send_data(conn_handle, cid, upay_gatt_srv_id, INDEX_UPAY_WRITE, p_last_node->p_data,
                                   p_last_node->data_length, GATT_PDU_TYPE_ANY);
            }
            else
            {
                APP_PRINT_INFO1("[AliPay] alipay_ble_send ble not connected, state %d",
                                RtkWristbandSys.gap_conn_state);
            }
            g_alipay_ble_transport_list = ali_queue_remove_last_node(g_alipay_ble_transport_list);
            g_alipay_ble_transport_list = ali_queue_add_node(g_alipay_ble_transport_list, data, len);
        }
    }
    else
    {
        g_alipay_ble_transport_list = ali_queue_add_node(g_alipay_ble_transport_list, data, len);
    }
    // to-do
}

void alipay_ble_send_completed_proc(uint8_t connect_id)
{
    uint8_t alipay_cretids = 0;
    uint16_t cid;
    uint8_t cid_num;
    uint16_t conn_handle = le_get_conn_handle(connect_id);

    gap_chann_get_cid(conn_handle, 1, &cid, &cid_num);
    le_get_gap_param(GAP_PARAM_LE_REMAIN_CREDITS, &alipay_cretids);
    APP_PRINT_INFO0("[AliPay] alipay_ble_send_completed_proc");

    //if (connect_id !=)
    for (; alipay_cretids > 0;)
    {
        ali_queue_t *p_last_node = ali_queue_indexof_last(g_alipay_ble_transport_list);
        if (p_last_node == NULL)
        {
            break;
        }

        // APP_PRINT_INFO1("[AliPay] alipay_ble_send_completed_proc, data %b", TRACE_BINARY(4, g_alipay_ble_transport_list->p_data));
        if (RtkWristbandSys.gap_conn_state != GAP_CONN_STATE_CONNECTED)
        {
            APP_PRINT_INFO1("[AliPay] alipay_ble_send_completed_proc not connected, state %d",
                            RtkWristbandSys.gap_conn_state);
        }
        else
        {
            gatt_svc_send_data(conn_handle, cid, upay_gatt_srv_id, INDEX_UPAY_WRITE, p_last_node->p_data,
                               p_last_node->data_length, GATT_PDU_TYPE_ANY);
        }

        g_alipay_ble_transport_list = ali_queue_remove_last_node(g_alipay_ble_transport_list);

        le_get_gap_param(GAP_PARAM_LE_REMAIN_CREDITS, &alipay_cretids);
    }

    // to-do
}


void  alipay_ble_set_connect_id(uint8_t connect_id)
{
    APP_PRINT_INFO1("[AliPay] alipay_ble_set_connect_id %d", connect_id);
    alipay_connect_id = connect_id;
}



#endif // CONFIG_ALIPAY
