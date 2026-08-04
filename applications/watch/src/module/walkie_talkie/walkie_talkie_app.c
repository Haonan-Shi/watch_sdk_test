/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_ble_adv.h"
#include "gap_le_types.h"
#include "gap.h"
#include "string.h"
#include "gap_le.h"
#include "ble_ext_adv.h"
#include "os_queue.h"
#include "ble_mgr.h"
#include "app_ble_gap.h"
#include "walkie_talkie_adv.h"
#include "walkie_talkie_scan.h"
#include "audio_track.h"
#include "audio.h"
#include "trace.h"
#include "walkie_talkie_voice.h"
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include "app_task.h"
#include "app_mmi.h"
#include "walkie_talkie_app.h"
#include "rtl876x_pinmux.h"
#include "gui_listener.h"
#include "walkie_talkie_gatt_svc.h"
#include "walkie_talkie_gatt_client.h"
#include "app_module_init.h"
#include "app_ble_service_info.h"
#include "app_ble_gap.h"
#include "event_bus.h"

/********************************************************************************************************
*Readme
*This is a walkie-talkie demo. Users need to config make the following configurations to enable this demo.
*1. CONFIG_WALKIE_TALKIE=y  CONFIG_OPUS=y  in prj.conf
*2. The mcu cfg should take care of LE link number and LE master link number. LE master link number shoud
*   be 1 for walkie-talkie master role, and LE link number should be increased by 1.
*3. Users call void walkie_talkie_transmit_start(void), void walkie_talkie_transmit_stop(void) to start or
*   stop voice transmitting.
*   example:
*   mmi actions:
*   MMI_WALKIE_TALKIE_TRANSMIT_START = 0xE6,
*   MMI_WALKIE_TALKIE_TRANSMIT_STOP = 0xE7,
*
*   case MMI_WALKIE_TALKIE_TRANSMIT_START:
        {
            walkie_talkie_transmit_start();
        }
        break;

    case MMI_WALKIE_TALKIE_TRANSMIT_STOP:
        {
            walkie_talkie_transmit_stop();
        }
        break;
********************************************************************************************************/

#define RF_EN_LNA_PIN            P1_0
#define RF_EN_PA_PIN             P1_1

T_WALKIE_TALKIE_CFG  walkie_talkie_cfg =
{
    .mode = WALKIE_TALKIE_CONN,
    .role = WALKIE_TALKIE_CONN_MASTER,
};

static T_WALKIE_TALKIE_DEV walkie_talkie_dev;

static int32_t walkie_talkie_app_handle_msg(T_EVENT_BUS_EVENT_DATA *event_data);

void walkie_talkie_init(void)
{
    // For RF PA control
    // Pad_Config(RF_EN_LNA_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
    //            PAD_OUT_HIGH);
    // Pinmux_Config(RF_EN_LNA_PIN, EN_EXLNA);
    // Pad_Config(RF_EN_PA_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
    //            PAD_OUT_HIGH);
    // Pinmux_Config(RF_EN_PA_PIN, EN_EXPA);

    walkie_talkie_adv_set_param();
    walkie_talkie_codec_init();


}

static void walkie_talkie_gatt_init(void)
{
    DBG_DIRECT("walkie_talkie_gatt_init");
    wts_reg_srv(NULL);
    wts_client_init(NULL);
    app_ble_gap_msg_handle_register(&transmit_adv);

    event_bus_topic_register(EVENT_BUS_TOPIC_WALKIE_TALKIE_ALL_TOPIC);
}
APP_MODULE_INIT(walkie_talkie_gatt_init);

APP_BLE_SERVICE_INFO(walkie_talkie, 1);

void walkie_talkie_on(void)
{
    if (walkie_talkie_cfg.mode == WALKIE_TALKIE_CONN)
    {
        walkie_talkie_dev.dev_cnt = 0;
        memset(walkie_talkie_dev.dev_info, 0, sizeof(walkie_talkie_dev.dev_info));
        walkie_talkie_scan_start();
        walkie_talkie_adv_start(0);
    }
}

void walkie_talkie_off(void)
{
    if (walkie_talkie_cfg.mode == WALKIE_TALKIE_CONN)
    {
        walkie_talkie_scan_stop();
        walkie_talkie_adv_stop(0);
    }
}


void walkie_talkie_receive_start(void)
{
    if (walkie_talkie_transmitter_working())
    {
        APP_PRINT_INFO0("walkie talkie transmitter is wroking, receive start fail!");
        return;
    }

    walkie_talkie_decoder_create();
    walkie_talkie_player_start();
    if (walkie_talkie_cfg.mode == WALKIE_TALKIE_ADV)
    {
        walkie_talkie_scan_start();
    }
}

void walkie_talkie_receive_stop(void)
{
    if (walkie_talkie_cfg.mode == WALKIE_TALKIE_ADV)
    {
        walkie_talkie_scan_stop();
    }
    walkie_talkie_player_stop();
    walkie_talkie_decoder_destroy();
}

void walkie_talkie_transmit_start(void)
{
    if (walkie_talkie_receiver_working())
    {
        APP_PRINT_INFO0("walkie talkie receiver is wroking, transmit start fail!");
        return;
    }

    walkie_talkie_receive_stop();

    if (walkie_talkie_cfg.mode == WALKIE_TALKIE_ADV)
    {
        walkie_talkie_adv_start(16);
    }
    walkie_talkie_encoder_create();
    walkie_talkie_recorder_start();
}

void walkie_talkie_transmit_stop(void)
{
    if (walkie_talkie_cfg.mode == WALKIE_TALKIE_ADV)
    {
        walkie_talkie_adv_stop(0);
    }
    walkie_talkie_recorder_stop();
    walkie_talkie_encoder_destroy();

    walkie_talkie_receive_start();
}

void walkie_talkie_transmit_start_send_to_ble(void)
{
    uint16_t cid;
    uint8_t cid_num;
    uint16_t conn_handle = le_get_conn_handle(transmit_adv.conn_id);
    gap_chann_get_cid(conn_handle, 1, &cid, &cid_num);
    uint8_t transmit_op = WALKIE_TALKIE_CP_REMOTE_TRANSMIT_START;

    if (transmit_adv.role == GAP_LINK_ROLE_SLAVE)
    {
        wts_send_control_point_notify(conn_handle, cid, &transmit_op, 1);
    }
    else if (transmit_adv.role == GAP_LINK_ROLE_MASTER)
    {
        wts_client_write_control_point_data(conn_handle, &transmit_op, 1);
    }
}

void walkie_talkie_transmit_stop_send_to_ble(void)
{
    uint16_t cid;
    uint8_t cid_num;
    uint16_t conn_handle = le_get_conn_handle(transmit_adv.conn_id);
    gap_chann_get_cid(conn_handle, 1, &cid, &cid_num);
    uint8_t transmit_op = WALKIE_TALKIE_CP_REMOTE_TRANSMIT_STOP;

    if (transmit_adv.role == GAP_LINK_ROLE_SLAVE)
    {
        wts_send_control_point_notify(conn_handle, cid, &transmit_op, 1);
    }
    else if (transmit_adv.role == GAP_LINK_ROLE_MASTER)
    {
        wts_client_write_control_point_data(conn_handle, &transmit_op, 1);
    }
}

void walkie_talkie_scan_report(void *data)
{
    if (walkie_talkie_dev.dev_cnt < WALKIE_TALKIE_DEV_MAX_CNT)
    {
        T_WALKIE_TALKIE_DEV_INFO *dev_info = (T_WALKIE_TALKIE_DEV_INFO *)data;

        // Check if device address already exists
        bool addr_exists = false;
        for (uint16_t i = 0; i < walkie_talkie_dev.dev_cnt; i++)
        {
            if (memcmp(walkie_talkie_dev.dev_info[i].addr, dev_info->addr, 6) == 0)
            {
                addr_exists = true;
                break;
            }
        }

        // Only add device if address does not exist
        if (!addr_exists)
        {
            memcpy(walkie_talkie_dev.dev_info[walkie_talkie_dev.dev_cnt].addr, dev_info->addr, 6);
            memcpy(walkie_talkie_dev.dev_info[walkie_talkie_dev.dev_cnt].dev_name, dev_info->dev_name, 12);
            walkie_talkie_dev.dev_cnt++;
            event_bus_publish(EVENT_BUS_TOPIC_WALKIE_TALKIE_DEV_INFO_UPDATE, &walkie_talkie_dev,
                              sizeof(T_WALKIE_TALKIE_DEV));
        }
    }
}

void walkie_talkie_save_dev_name(void *data)
{
    T_WALKIE_TALKIE_DEV_INFO *dev_info = (T_WALKIE_TALKIE_DEV_INFO *)data;
    memcpy(walkie_talkie_dev.connected_dev.addr, dev_info->addr, 6);
    memcpy(walkie_talkie_dev.connected_dev.dev_name, dev_info->dev_name, 12);
}

void walkie_talkie_connect_dev(uint8_t dev_to_connect)
{
    if (dev_to_connect >= WALKIE_TALKIE_DEV_MAX_CNT)
    {
        APP_PRINT_ERROR0("walkie_talkie_connect_dev dev cnt error");
        return;
    }

    if (walkie_talkie_dev.dev_cnt > dev_to_connect)
    {
        APP_PRINT_INFO1("walkie_talkie_connect_dev: connect device %s",
                        TRACE_STRING(walkie_talkie_dev.dev_info[dev_to_connect].dev_name));
        walkie_talkie_scan_stop();
        walkie_talkie_adv_stop(0);
        walkie_talkie_scan_connect(walkie_talkie_dev.dev_info[dev_to_connect].addr);
    }
}

/**
    * @brief    Handle written request on walkie talkie control point characteristic
    * @param    conn_handle     handle to identify the connection
    * @param    cid             Local CID assigned by Bluetooth stack.
    * @param    length      Length of value to be written
    * @param    p_value     Value to be written
    * @return   T_APP_RESULT
    * @retval   Handle result of this request
    */
T_APP_RESULT walkie_talkie_service_handle_cp_req(uint8_t conn_handle, uint16_t cid, uint16_t length,
                                                 uint8_t *p_value)
{
    T_APP_RESULT cause = APP_RESULT_SUCCESS;
    uint8_t opcode = p_value[0];

    APP_PRINT_INFO2("walkie_talkie_service_handle_cp_req: opcode=0x%x, length=%d", opcode, length);

    switch (opcode)
    {
    case WALKIE_TALKIE_CP_NAME:
        {
            uint8_t conn_id;
            uint8_t bd_type;
            T_WALKIE_TALKIE_DEV_INFO dev_info;
            memset(&dev_info, 0, sizeof(T_WALKIE_TALKIE_DEV_INFO));
            le_get_conn_id_by_handle(conn_handle, &conn_id);
            le_get_conn_addr(conn_id, dev_info.addr, &bd_type);
            memcpy(dev_info.dev_name, p_value + 1, USER_NAME_LEN);
            APP_PRINT_INFO1("walkie_talkie_control_point: user name %s", TRACE_STRING(dev_info.dev_name));
            event_bus_publish(EVENT_BUS_TOPIC_WALKIE_TALKIE_USER_NAME, &dev_info,
                              sizeof(T_WALKIE_TALKIE_DEV_INFO));

            if (transmit_adv.role == GAP_LINK_ROLE_SLAVE)
            {
                uint8_t user_name_exchange[USER_NAME_LEN + 1] = {0};
                user_name_exchange[0] = WALKIE_TALKIE_CP_NAME;
                memcpy(user_name_exchange + 1, transmit_adv_data + USER_NAME_OFFSET, USER_NAME_LEN);
                wts_send_control_point_notify(conn_handle, cid, user_name_exchange, USER_NAME_LEN + 1);
            }
        }
        break;

    case WALKIE_TALKIE_CP_REMOTE_TRANSMIT_START:
        {
            event_bus_publish(EVENT_BUS_TOPIC_WALKIE_TALKIE_RECEIVE_START, NULL, 0);
        }
        break;

    case WALKIE_TALKIE_CP_REMOTE_TRANSMIT_STOP:
        {
            event_bus_publish(EVENT_BUS_TOPIC_WALKIE_TALKIE_RECEIVE_STOP, NULL, 0);
        }
        break;
    default:
        cause = APP_RESULT_INVALID_PDU;
        break;
    }
    return cause;
}
