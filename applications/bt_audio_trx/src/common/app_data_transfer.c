/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdint.h>
#include "stdlib_corecrt.h"
#include "console.h"
#include "gap.h"
#include "gap_br.h"
#include "trace.h"
#include "bt_types.h"
#include "hal_gpio.h"
#include "app_timer.h"
#include "bt_iap.h"
#include "app_main.h"
#include "app_ble_service.h"
#include "app_cmd.h"
#include "app_cfg.h"
#include "app_dlps.h"
#include "app_spp.h"
#include "app_msg.h"
#include "app_data_transfer.h"

#if F_APP_IAP_RTK_SUPPORT
#include "app_iap_rtk.h"
#endif

#if F_APP_CHATGPT_SUPPORT
#include "app_chatgpt.h"
#endif

#define DT_EVENT_ID_OFFSET                  (4)

#define DT_QUEUE_DEF_NUM                    (16)
#define DT_TRANS_UART_TX_WAKE_MS            (10)

#define DT_RETRY_NUM                        (20)
#define DT_RETRY_MS                         (100)

#define DT_WAIT_ACK_NUM                     (5)
#define DT_WAIT_ACK_MS                      (1000)

#define DT_WAIT_READY                       (5)

typedef enum
{
    APP_TIMER_UART_DLPS_WAKE,
    APP_TIMER_UART_TX_WAKE,
} T_APP_TRANSFER_TIMER;

typedef enum t_packet_type
{
    PACKET_TYPE_SINGLE                 = 0,
    PACKET_TYPE_FRAGMENT_START         = 1,
    PACKET_TYPE_FRAGMENT_CONTINUE      = 2,
    PACKET_TYPE_FRAGMENT_END           = 3,
} T_PACKET_TYPE;

static bool rtk_cmd_en = false;
static uint8_t app_data_transfer_timer_id = 0;
static uint8_t timer_idx_uart_dlps_wake = 0;
static uint8_t timer_idx_uart_tx_wake = 0;
// for APP_CMD_BR_VENDOR_DATA_TRANSFER and CMD_LE_DATA_TRANSFER
static uint8_t *uart_rx_dt_pkt_ptr = NULL;
static uint16_t uart_rx_dt_pkt_len;
static T_DT_HANDLE uart_data_transfer_handle = NULL;
static T_DT_HANDLE ble_data_transfer_handle = NULL;
static T_DT_HANDLE spp_data_transfer_handle = NULL;
static T_DT_HANDLE iap_data_transfer_handle = NULL;
static T_DT_HANDLE gatt_data_transfer_handle = NULL;

static T_DT_RESULT app_data_transfer_console(uint8_t app_idx, uint8_t *data, uint32_t len);
static T_DT_RESULT app_data_transfer_ble(uint8_t app_idx, uint8_t *data, uint32_t len);
static T_DT_RESULT app_data_transfer_spp(uint8_t app_idx, uint8_t *data, uint32_t len);
static T_DT_RESULT app_data_transfer_iap(uint8_t app_idx, uint8_t *data, uint32_t len);
static T_DT_RESULT app_data_transfer_gatt(uint8_t app_idx, uint8_t *data, uint32_t len);

static void app_data_transfer_uart_start(void);
static void app_data_transfer_uart_finish(void);

/*ref T_DATA_TRANS_PATH not to change order*/
T_DATA_TRANS_MAP data_trans_map[] =
{
    {NULL,                             NULL,                         NULL,      NULL,      NULL,                              0},
    {&uart_data_transfer_handle,       app_data_transfer_console,    NULL,      NULL,      app_data_transfer_uart_finish,     DT_QUEUE_DEF_NUM},
    {&ble_data_transfer_handle,        app_data_transfer_ble,        NULL,      NULL,      NULL,                              DT_QUEUE_DEF_NUM},
    {&spp_data_transfer_handle,        app_data_transfer_spp,        NULL,      NULL,      NULL,                              DT_QUEUE_DEF_NUM},
    {&iap_data_transfer_handle,        app_data_transfer_iap,        NULL,      NULL,      NULL,                              DT_QUEUE_DEF_NUM},
    {&gatt_data_transfer_handle,       app_data_transfer_gatt,       NULL,      NULL,      NULL,                              DT_QUEUE_DEF_NUM},
    {NULL,                             NULL,                         NULL,      NULL,      NULL,                              0},
};

T_DATA_TRANS data_trans =
{
    .map = data_trans_map,
    .enable = false,

    .resend =
    {
        .retry_num = DT_RETRY_NUM,
        .retry_ms = DT_RETRY_MS,

        .wait_ack_num = DT_WAIT_ACK_NUM,
        .wait_ack_ms = DT_WAIT_ACK_MS,

        .wait_ready_ms = DT_WAIT_READY,
    }
};

static bool app_data_transfer_path_check(uint8_t cmd_path)
{
    if ((cmd_path >= CMD_PATH_DT_MAX) || (cmd_path == CMD_PATH_NONE))
    {
        return false;
    }

    return true;
}

static void app_data_transfer_uart_start(void)
{
    hal_gpio_set_level(app_cfg_const.tx_wake_up_pinmux, GPIO_LEVEL_LOW);
    app_start_timer(&timer_idx_uart_tx_wake, "uart_tx_wake",
                    app_data_transfer_timer_id, APP_TIMER_UART_TX_WAKE, 0, false,
                    DT_TRANS_UART_TX_WAKE_MS);
}

static void app_data_transfer_uart_finish(void)
{
    hal_gpio_set_level(app_cfg_const.tx_wake_up_pinmux, GPIO_LEVEL_HIGH);
}

static uint8_t app_data_transfer_console(uint8_t app_idx, uint8_t *data, uint32_t len)
{
    if (data_trans_map[DATA_TRANS_PATH_UART].send_instance != NULL)
    {
        return data_trans_map[DATA_TRANS_PATH_UART].send_instance(app_idx, data, len);
    }

    return DT_SEND_FAIL;
}

static uint8_t app_data_transfer_ble(uint8_t app_idx, uint8_t *data, uint32_t len)
{
    uint8_t result = BLE_SEND_OK;

    if (data_trans_map[DATA_TRANS_PATH_LE].send_instance != NULL)
    {
        result = data_trans_map[DATA_TRANS_PATH_LE].send_instance(app_idx, data, len);

        if (result == BLE_SEND_OK)
        {
            return DT_OK;
        }
        else
        {
            if (result == BLE_NOT_CONN)
            {
                return DT_NOT_CONN;
            }
            else if (result == BLE_NOT_READY)
            {
                return DT_NOT_READY;
            }
        }
    }

    return DT_SEND_FAIL;
}

static uint8_t app_data_transfer_gatt(uint8_t app_idx, uint8_t *data, uint32_t len)
{
    uint8_t result = BLE_SEND_OK;

    if (data_trans_map[DATA_TRANS_PATH_GATT_OVER_BREDR].send_instance != NULL)
    {
        result = data_trans_map[DATA_TRANS_PATH_GATT_OVER_BREDR].send_instance(app_idx, data, len);

        if (result == BLE_SEND_OK)
        {
            return DT_OK;
        }
        else
        {
            if (result == BLE_NOT_CONN)
            {
                return DT_NOT_CONN;
            }
            else if (result == BLE_NOT_READY)
            {
                return DT_NOT_READY;
            }
        }
    }

    return DT_SEND_FAIL;
}

static uint8_t app_data_transfer_spp(uint8_t app_idx, uint8_t *data, uint32_t len)
{
    uint8_t result = SPP_SEND_OK;

    if (data_trans_map[DATA_TRANS_PATH_SPP].send_instance != NULL)
    {
        result = data_trans_map[DATA_TRANS_PATH_SPP].send_instance(app_idx, data, len);

        if (result == SPP_SEND_OK)
        {
            return DT_OK;
        }
        else
        {
            if (result == SPP_PROFILE_NOT_CONN)
            {
                return DT_NOT_CONN;
            }
            else if (result == SPP_NO_RFC_CREDIT)
            {
                return DT_NOT_READY;
            }
        }
    }

    return DT_SEND_FAIL;
}

static T_DT_RESULT app_data_transfer_iap(uint8_t app_idx, uint8_t *data, uint32_t len)
{
#if F_APP_IAP_RTK_SUPPORT
    uint8_t result = DT_OK;

    if (data_trans_map[DATA_TRANS_PATH_IAP].send_instance != NULL)
    {
        result = data_trans_map[DATA_TRANS_PATH_IAP].send_instance(app_idx, data, len);

        if (result == IAP_SEND_OK)
        {
            return DT_OK;
        }
        else
        {
            if (result == IAP_PROFILE_NOT_CONN)
            {
                return DT_NOT_CONN;
            }
            else if ((result == IAP_SESSION_NOT_CONN) || (result == IAP_RTK_LAUNCHED))
            {
                return DT_NOT_READY;
            }
        }
    }
#endif

    return DT_SEND_FAIL;
}

static bool app_data_transfer_push_data_check(uint8_t cmd_path)
{
    T_DT_PARA para = {0};
    bool ret = false;

    if (!app_data_transfer_path_check(cmd_path))
    {
        goto push_data_check;
    }

    if (cmd_path != DATA_TRANS_PATH_UART)
    {
        if (!data_trans.enable)
        {
            goto push_data_check;
        }
    }

    if (*(data_trans_map[cmd_path].p_handle) == NULL)
    {
        if ((cmd_path == DATA_TRANS_PATH_UART) && (app_cfg_const.enable_tx_wake_up != 0))
        {
            para.hold = true;
            para.finish = app_data_transfer_uart_finish;
            app_data_transfer_uart_start();
        }
        para.send = data_trans_map[cmd_path].send;
        para.queue_num = data_trans_map[cmd_path].queue_num;
        para.p_handle = &data_trans_map[cmd_path].p_handle;

#if F_APP_CHATGPT_SUPPORT
        para.offset = TRANS_EVENT_ID_OFFSET;
#else
        para.offset = DT_EVENT_ID_OFFSET;
#endif

        *(data_trans_map[cmd_path].p_handle) = data_transfer_create(&para);
    }
    else
    {
        if ((cmd_path == DATA_TRANS_PATH_UART) && (app_cfg_const.enable_tx_wake_up != 0))
        {
            if (data_transfer_is_empty(*(data_trans_map[cmd_path].p_handle)))
            {
                data_transfer_cfg_hold_flag(*(data_trans_map[cmd_path].p_handle), true);
                app_data_transfer_uart_start();
            }
        }
    }

    if (*(data_trans_map[cmd_path].p_handle) != NULL)
    {
        ret = true;
    }

push_data_check:
    APP_PRINT_INFO3("app_data_transfer_push_data_check: cmd_path %d handle %p ret %d", cmd_path,
                    *(data_trans_map[cmd_path].p_handle), ret);

    return ret;
}

bool app_data_transfer_push_data_queue(uint8_t cmd_path, uint8_t idx, uint8_t *data, uint16_t len,
                                       T_DT_MODE mode)
{
    if (app_data_transfer_push_data_check(cmd_path))
    {
        return data_transfer_push_data_queue(*(data_trans_map[cmd_path].p_handle), idx, data, len, mode);
    }

    return false;
}

void app_data_transfer_switch(uint8_t cmd_path, uint16_t event_id, bool is_big_end)
{
    if (app_data_transfer_path_check(cmd_path) && (*(data_trans_map[cmd_path].p_handle) != NULL))
    {
        data_transfer_switch_check(*(data_trans_map[cmd_path].p_handle), event_id, is_big_end);
    }
}

void app_data_transfer_queue_reset(uint8_t cmd_path)
{
    if (app_data_transfer_path_check(cmd_path) && (*(data_trans_map[cmd_path].p_handle) != NULL))
    {
        data_transfer_queue_reset(*(data_trans_map[cmd_path].p_handle));
    }
}

void app_data_transfer_pop_data_queue(uint8_t cmd_path)
{
    if (app_data_transfer_path_check(cmd_path) && (*(data_trans_map[cmd_path].p_handle) != NULL))
    {
        data_transfer_pop_data_queue(*(data_trans_map[cmd_path].p_handle));
    }
}

static void app_data_transfer_cfg_hold_flag(uint8_t cmd_path, bool hold)
{
    if (app_data_transfer_path_check(cmd_path) && (*(data_trans_map[cmd_path].p_handle) != NULL))
    {
        data_transfer_pop_data_queue(*(data_trans_map[cmd_path].p_handle));
        data_transfer_cfg_hold_flag(*(data_trans_map[cmd_path].p_handle), hold);
    }
}

void app_data_transfer_handle_msg(T_IO_MSG *io_driver_msg_recv)
{
    uint32_t active_time = (console_get_mode() == CONSOLE_MODE_STRING) ? 30000 : 2000;

    app_dlps_disable(APP_DLPS_ENTER_CHECK_UART_RX);
    app_start_timer(&timer_idx_uart_dlps_wake, "uart_dlps_wake", app_data_transfer_timer_id,
                    APP_TIMER_UART_DLPS_WAKE, 0, false, active_time);
}

static void app_data_transfer_timeout_cb(uint8_t timer_evt, uint16_t param)
{
    APP_PRINT_TRACE2("app_data_transfer_timeout_cb: timer_id %d, timer_chann %d", timer_evt, param);

    switch (timer_evt)
    {
    case APP_TIMER_UART_DLPS_WAKE:
        {
            app_stop_timer(&timer_idx_uart_dlps_wake);
            app_dlps_enable(APP_DLPS_ENTER_CHECK_UART_RX);
        }
        break;

    case APP_TIMER_UART_TX_WAKE:
        {
            app_stop_timer(&timer_idx_uart_tx_wake);
            app_data_transfer_cfg_hold_flag(DATA_TRANS_PATH_UART, false);
            app_data_transfer_pop_data_queue(DATA_TRANS_PATH_UART);
        }
        break;

    default:
        break;
    }
}

void app_data_transfer_init(void)
{
    data_transfer_init(&data_trans.resend);
    app_timer_reg_cb(app_data_transfer_timeout_cb, &app_data_transfer_timer_id);
}
