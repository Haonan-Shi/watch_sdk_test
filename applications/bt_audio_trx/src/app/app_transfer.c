/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdlib.h>
#include <string.h>
#include "stdlib_corecrt.h"
#include "os_mem.h"
#include "os_timer.h"
#include "os_sync.h"
#include "console.h"
#include "gap.h"
#include "gap_br.h"
#include "trace.h"
#include "bt_types.h"
#include "hal_gpio.h"
#include "app_timer.h"
#include "transmit_service.h"

#if F_APP_IAP_SUPPORT
#include "bt_iap.h"
#endif
#include "app_main.h"
#include "app_ble_service.h"
#include "app_cmd.h"
#include "app_cfg.h"
#include "app_spp.h"
#include "app_iap_rtk.h"
#include "app_report.h"
#include "app_data_transfer.h"
#include "app_transfer_cfg.h"
#include "app_transfer.h"

#if F_APP_LEA_SUPPORT
#include "app_dongle_common.h"
#include "app_dongle_data_ctrl.h"
#endif

#if F_APP_IAP_RTK_SUPPORT
#include "app_iap_rtk.h"
#endif


static uint8_t *uart_rx_dt_pkt_ptr = NULL;
static uint16_t uart_rx_dt_pkt_len = 0;

/*ref T_CMD_PATH,not change the order*/
static const uint8_t cmd_path_map[] =
{
    DATA_TRANS_PATH_NONE,

    DATA_TRANS_PATH_UART,
    DATA_TRANS_PATH_LE,
    DATA_TRANS_PATH_SPP,
    DATA_TRANS_PATH_IAP,
    DATA_TRANS_PATH_GATT_OVER_BREDR,

    DATA_TRANS_PATH_NONE
};

static uint8_t app_transfer_get_path(uint8_t cmd_path)
{
    uint8_t data_trans_path = DATA_TRANS_PATH_NONE;

    if (cmd_path >= CMD_PATH_DT_MAX)
    {
        data_trans_path = DATA_TRANS_PATH_NONE;
    }
    else
    {
        data_trans_path = cmd_path_map[cmd_path];
    }

    return data_trans_path;
}

static void app_transfer_bt_data(uint8_t *cmd_ptr, uint8_t cmd_path, uint8_t app_idx,
                                 uint8_t *ack_pkt)
{
    uint16_t cmd_id = (uint16_t)(cmd_ptr[0] | (cmd_ptr[1] << 8));
    uint16_t total_len = (cmd_ptr[4] | (cmd_ptr[5] << 8));
    uint16_t pkt_len = (cmd_ptr[6] | (cmd_ptr[7] << 8));
    uint8_t  idx = cmd_ptr[2];
    uint8_t pkt_type = cmd_ptr[3];
    uint8_t  *pkt_ptr = &cmd_ptr[8];
    bool skip_ack = false;
#if F_APP_LEA_SUPPORT && F_APP_GAMING_DONGLE_SUPPORT
    T_APP_LE_LINK *p_lea_link = app_dongle_get_le_audio_link();

    if (p_lea_link && (cmd_id == CMD_LE_DATA_TRANSFER))
    {
        /*
            Replace idx with app_idx because LE dongle would send data to primary and secondary simultaneously.
        */
        idx = app_idx;
    }
#endif

    if (pkt_len == 0)
    {
        ack_pkt[2] = CMD_SET_STATUS_PARAMETER_ERROR;
        goto bt_data_ack;
    }

    if (pkt_type > PKT_TYPE_END)
    {
        ack_pkt[2] = CMD_SET_STATUS_PARAMETER_ERROR;
        goto bt_data_ack;
    }

    if (cmd_path == CMD_PATH_UART)
    {
        if ((pkt_type == PKT_TYPE_SINGLE) || (pkt_type == PKT_TYPE_START))
        {
            if (uart_rx_dt_pkt_ptr)
            {
                free(uart_rx_dt_pkt_ptr);
            }

            uart_rx_dt_pkt_ptr = malloc(total_len);
            memcpy_s(uart_rx_dt_pkt_ptr, total_len, pkt_ptr, pkt_len);
            uart_rx_dt_pkt_len = pkt_len;
        }
        else
        {
            if (uart_rx_dt_pkt_ptr != NULL)
            {
                uint8_t *temp_ptr;

                temp_ptr = uart_rx_dt_pkt_ptr + uart_rx_dt_pkt_len;
                memcpy(temp_ptr, pkt_ptr, pkt_len);
                uart_rx_dt_pkt_len += pkt_len;
            }
        }

        if ((pkt_type == PKT_TYPE_SINGLE) || (pkt_type == PKT_TYPE_END))
        {
            if (ack_pkt[2] == CMD_SET_STATUS_COMPLETE)
            {
                uint8_t cmd_path = CMD_PATH_NONE;

                if (cmd_id == CMD_LEGACY_DATA_TRANSFER)
                {
                    if (app_db.br_link[idx].connected_profile & SPP_PROFILE_MASK)
                    {
                        cmd_path = CMD_PATH_SPP;
                    }
                    else if (app_db.br_link[idx].connected_profile & IAP_PROFILE_MASK)
                    {
                        cmd_path = CMD_PATH_IAP;
                    }
                }
                else if (cmd_id == CMD_LE_DATA_TRANSFER)
                {
                    cmd_path = CMD_PATH_LE;
                }

                if (!app_report_raw_data(cmd_path, app_idx, uart_rx_dt_pkt_ptr, uart_rx_dt_pkt_len))
                {
                    ack_pkt[2] = CMD_SET_STATUS_BUSY;
                }

                free(uart_rx_dt_pkt_ptr);
                uart_rx_dt_pkt_ptr = NULL;
            }
        }
    }
    else if (cmd_path == CMD_PATH_LE)
    {
#if F_APP_LEA_SUPPORT && F_APP_GAMING_DONGLE_SUPPORT
        if (p_lea_link)
        {
            app_dongle_handle_le_data(pkt_ptr, pkt_len);
            skip_ack = true;
        }
#endif
    }

bt_data_ack:
    if (!skip_ack)
    {
        app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);
    }
}

static T_DT_RESULT app_transfer_console_send(uint8_t app_idx, uint8_t *data, uint32_t len)
{
    bool ret = false;

#if F_APP_CONSOLE_SUPPORT
    ret = console_write(data, len);
#endif

    return ret ? DT_OK : DT_SEND_FAIL;
}

static T_DT_MODE app_transfer_check_mode(uint8_t cmd_path, uint16_t event_id, bool broadcast)
{
    if (broadcast)
    {
        return DT_MODE_NO_NEED_ACK;
    }

    if (event_id == EVENT_ACK)
    {
        return DT_MODE_NO_NEED_ACK;
    }

#if F_APP_GAMING_DONGLE_SUPPORT
    if (app_db.remote_is_dongle)
    {
        return DT_MODE_NO_NEED_ACK;
    }
#endif

#if CONFIG_REALTEK_APP_AI_RECORD
    if ((event_id == EVENT_AI_RECORD_APP_VOICE_DATA) ||
        (event_id == EVENT_AI_RECORD_APP_GET_FILE_DATA) ||
        (event_id == EVENT_AI_RECORD_APP_GET_DATA_TCP) ||
        (event_id == EVENT_AI_RECORD_APP_LIVE_STREAM_RESP) ||
        (event_id == EVENT_AI_RECORD_WIFI_DATA_TCP_ACK))
    {
        return DT_MODE_NO_NEED_ACK;
    }
#endif

    if (cmd_path == CMD_PATH_UART)
    {
        if (app_transfer_cfg.report_uart_event_only_once != 0)
        {
            return DT_MODE_NO_NEED_ACK;
        }

        if ((event_id == EVENT_LEGACY_DATA_TRANSFER) || (event_id == EVENT_XM_SPP_DATA_TRANSFER) ||
            (event_id == EVENT_XM_RECORDING_DATA))
        {
            return DT_MODE_NO_NEED_ACK;
        }
    }

#if F_APP_DATA_CAPTURE_SUPPORT
    if ((event_id == EVENT_DATA_CAPTURE_START_STOP) ||
        (event_id == EVENT_DATA_CAPTURE_DATA)       ||
        (event_id == EVENT_DATA_CAPTURE_ENTER_EXIT) ||
        (event_id == EVENT_ACK))
    {
        return DT_MODE_NO_NEED_ACK;
    }
#endif


    return DT_MODE_NEED_ACK;
}

bool app_transfer_push_data_queue(uint8_t cmd_path, uint8_t idx, uint16_t event_id, bool broadcast,
                                  uint8_t *data, uint16_t len)
{
    T_DT_MODE mode = app_transfer_check_mode(app_transfer_get_path(cmd_path), event_id, broadcast);
    return app_data_transfer_push_data_queue(app_transfer_get_path(cmd_path), idx, data, len, mode);
}

void app_transfer_switch(uint8_t cmd_path, uint16_t event_id)
{
    app_data_transfer_switch(app_transfer_get_path(cmd_path), event_id, false);
}

void app_transfer_queue_reset(uint8_t cmd_path)
{
    app_data_transfer_queue_reset(app_transfer_get_path(cmd_path));
}

void app_transfer_pop_data_queue(uint8_t cmd_path)
{
    app_data_transfer_pop_data_queue(app_transfer_get_path(cmd_path));
}

void app_transfer_handle_msg(T_IO_MSG *io_driver_msg_recv)
{
    app_data_transfer_handle_msg(io_driver_msg_recv);
}

void app_transfer_cmd_handle(uint8_t *cmd_ptr, uint16_t cmd_len, uint8_t cmd_path, uint8_t app_idx,
                             uint8_t *ack_pkt)
{
    uint16_t cmd_id = (uint16_t)(cmd_ptr[0] | (cmd_ptr[1] << 8));

    switch (cmd_id)
    {
    case CMD_LEGACY_DATA_TRANSFER:
    case CMD_LE_DATA_TRANSFER:
        {
            app_transfer_bt_data(&cmd_ptr[0], cmd_path, app_idx, &ack_pkt[0]);
        }
        break;

    default:
        {
            ack_pkt[2] = CMD_SET_STATUS_UNKNOW_CMD;
            app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);
        }
        break;
    }
}

static void app_transfer_param_init(void)
{
    data_trans.map[DATA_TRANS_PATH_UART].send_instance = app_transfer_console_send;
    data_trans.map[DATA_TRANS_PATH_LE].send_instance = app_ble_service_transfer;
    data_trans.map[DATA_TRANS_PATH_SPP].send_instance = app_spp_transfer;
#if F_APP_IAP_RTK_SUPPORT
    data_trans.map[DATA_TRANS_PATH_IAP].send_instance = app_iap_rtk_transfer;
#endif
    data_trans.map[DATA_TRANS_PATH_GATT_OVER_BREDR].send_instance = app_gatt_over_bredr_transfer;

    data_trans.enable = (app_transfer_cfg.enable_rtk_vendor_cmd != 0) ? true : false;
    data_trans.resend.wait_ack_ms = app_transfer_cfg.resend_interval_ms;
    data_trans.resend.wait_ack_num = app_transfer_cfg.resend_num;
}

void app_transfer_init(void)
{
    app_transfer_param_init();
    app_data_transfer_init();
}
