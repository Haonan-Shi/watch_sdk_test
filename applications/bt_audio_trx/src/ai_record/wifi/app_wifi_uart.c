/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if F_APP_WIFI_UART_CMD

#include <string.h>
#include <stdio.h>
#include "trace.h"
#include "app_uart_atcmd.h"
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/pinctrl.h>
#include "os_mem.h"
#include "os_sync.h"
#include "app_wifi_uart.h"
#include "app_io_msg.h"
#include "os_msg.h"

#define RX_BUF_LENGTH       100
#define TX_BUF_LENGTH       128

typedef enum
{
    UART_ATCMD,
    UART_MPCMD
} T_UART_MODE;

typedef struct
{
    uint16_t read_idx;
    uint16_t write_idx;
    uint16_t rx_cnt;
    uint8_t  console_buf[RX_BUF_LENGTH];
    bool     tx_lock;
    uint8_t tx_buf[TX_BUF_LENGTH];
} T_CONSOLE;

typedef struct
{
    T_UART_MODE uart_mode;
    uint32_t baud;
    void *mp_sem_handle;
    uint8_t buf_index;
    uint8_t rx_buf[RX_BUF_LENGTH * 2];
} T_WIFI_UART_SET;

static P_WIFI_UART_RX_PARSER rgs_parser_cb = NULL;


#define PINCTRL_STATE_MP    (PINCTRL_STATE_PRIV_START+1)
PINCTRL_DT_STATE_PINS_DEFINE(DT_PATH(zephyr_user), uart3_mp_pin);
PINCTRL_DT_DEV_CONFIG_DECLARE(DT_ALIAS(uart3));
const struct pinctrl_dev_config *pincfg = PINCTRL_DT_DEV_CONFIG_GET(DT_ALIAS(uart3));
static const struct pinctrl_state uart3_mp_state = PINCTRL_DT_STATE_INIT(uart3_mp_pin,
                                                                         PINCTRL_STATE_MP);
const struct device *const uart3_dev = DEVICE_DT_GET(DT_ALIAS(uart3));

T_WIFI_UART_SET uart_set = {.uart_mode = UART_ATCMD, .baud = 115200};
T_CONSOLE wifi_console =
{
    .read_idx = 0,
    .write_idx = 0,
    .rx_cnt = 0,
    .tx_lock = false,
};

void app_wifi_uart_rx_parser_register(P_WIFI_UART_RX_PARSER cb_func)
{
    if (rgs_parser_cb == NULL)
    {
        rgs_parser_cb = cb_func;
    }
}

bool app_wifi_uart_msg_send(IO_WIFI_UART_MSG_TYPE subtype, void *param_buf)
{
    T_IO_MSG msg;

    msg.type = IO_MSG_TYPE_UART_AT_CMD;
    msg.subtype = subtype;
    msg.u.buf = param_buf;

    return app_io_msg_send(&msg);
}

void app_wifi_uart_msg_handle(T_IO_MSG *io_msg)
{
    IO_WIFI_UART_MSG_TYPE sub_type = (IO_WIFI_UART_MSG_TYPE)io_msg->subtype;
    // APP_PRINT_INFO1("app_wifi_uart_msg_handle: sub_type %d", sub_type);
    switch (sub_type)
    {
    case IO_WIFI_UART_AT_CMD_IND:
        {
            if (rgs_parser_cb)
            {
                rgs_parser_cb();
            }
        }
        break;

    case IO_WIFI_UART_EXIT_DLPS:
        {
            uart_rx_disable(uart3_dev);
            uart_set.buf_index = 0;
            uart_rx_enable(uart3_dev, uart_set.rx_buf, RX_BUF_LENGTH, 0);
        }
    default:
        break;
    }
}

void app_wifi_uart_rx_enable(void)
{
    uart_set.buf_index = 0;
    uart_rx_enable(uart3_dev, uart_set.rx_buf, RX_BUF_LENGTH, 0);
}


static void uart_async_callback(const struct device *dev, struct uart_event *evt, void *user_data)
{
    // APP_PRINT_TRACE1("wifi_uart_async_callback: type %d", evt->type);

    switch (evt->type)
    {
    case UART_TX_DONE:
        {
            if (uart_set.uart_mode == UART_ATCMD)
            {
                wifi_console.tx_lock = false;
            }
        }
        break;
    case UART_RX_RDY:
        {
            APP_PRINT_TRACE3("wifi_uart_rx: buf = 0x%x, len %d, data %b", &evt->data.rx.buf[0],
                             evt->data.rx.len,
                             TRACE_BINARY(evt->data.rx.len, &evt->data.rx.buf[evt->data.rx.offset]));

            wifi_uart_console_recv(&evt->data.rx.buf[evt->data.rx.offset], evt->data.rx.len);
            if (uart_set.uart_mode == UART_ATCMD)
            {
                if (app_wifi_uart_msg_send(IO_WIFI_UART_AT_CMD_IND, NULL) == false)
                {
                    APP_PRINT_ERROR0("[wifi] atcmd rsp msg send fail !");
                }
            }
            else if (uart_set.uart_mode == UART_MPCMD)
            {
                if (os_sem_give(uart_set.mp_sem_handle) == false)
                {
                    APP_PRINT_INFO0("mp_cmd os_sem_give fail");
                }
            }
        }
        break;
    case UART_RX_BUF_REQUEST:
        {
            if (uart_set.buf_index == 0)
            {
                uart_set.buf_index = 1;
            }
            else if (uart_set.buf_index == 1)
            {
                uart_set.buf_index = 0;
            }
            uart_rx_buf_rsp(uart3_dev,  &uart_set.rx_buf[uart_set.buf_index * RX_BUF_LENGTH], RX_BUF_LENGTH);
            APP_PRINT_TRACE1("uart_async_callback: set buf %d", uart_set.buf_index);
        }
        break;
    default:
        break;
    }
}

void app_wifi_uart_init(void)
{
    if (!device_is_ready(uart3_dev))
    {
        APP_PRINT_ERROR0("[wifi] UART device not found!");
        return;
    }
    if (os_sem_create(&uart_set.mp_sem_handle, "mp_sem", 0, 1) == true)
    {
        APP_PRINT_INFO0("os_sem_create mp_sem success ");
    }
    else
    {
        APP_PRINT_INFO0("os_sem_create mp_sem success");
    }
    uart_callback_set(uart3_dev, uart_async_callback, NULL);
    uart_set.buf_index = 0;
    uart_rx_enable(uart3_dev, uart_set.rx_buf, RX_BUF_LENGTH, 0);
}

void wifi_uart_console_recv(uint8_t *recv_buf, uint16_t recv_len)
{
    if (wifi_console.rx_cnt + recv_len > RX_BUF_LENGTH)
    {
        APP_PRINT_ERROR2("[wifi] uart console buf overflow console_rx_cnt = %d, recv_len = %d", \
                         wifi_console.rx_cnt, recv_len);
        return;
    }
    if (wifi_console.write_idx + recv_len <= RX_BUF_LENGTH)
    {
        memcpy(&wifi_console.console_buf[wifi_console.write_idx], recv_buf, recv_len);
        wifi_console.rx_cnt += recv_len;
        wifi_console.write_idx += recv_len;
        if (wifi_console.write_idx == RX_BUF_LENGTH)
        {
            wifi_console.write_idx = 0;
        }
    }
    else
    {
        uint32_t temp;
        temp = RX_BUF_LENGTH - wifi_console.write_idx;
        memcpy(&wifi_console.console_buf[wifi_console.write_idx], recv_buf, temp);
        memcpy(&wifi_console.console_buf[0], &recv_buf[temp], recv_len - temp);
        wifi_console.rx_cnt += recv_len;
        wifi_console.write_idx  = recv_len - temp;
    }
}

bool atcmd_send(const uint8_t *tx_data, int size)
{
    if (wifi_console.tx_lock == false)
    {
        if (size > TX_BUF_LENGTH)
        {
            APP_PRINT_ERROR2("atcmd_send fail, cmd len = %d, buf len = %d", size, TX_BUF_LENGTH);
            return false;
        }
        wifi_console.tx_lock = true;
        memcpy(wifi_console.tx_buf, tx_data, size);
        uart_tx(uart3_dev, wifi_console.tx_buf, size, SYS_FOREVER_US);
        return true;
    }
    return false;
}

uint16_t atcmd_recv(uint8_t *recv_buf, uint16_t recv_len)
{
    uint16_t read_len = wifi_console.rx_cnt;
    if (wifi_console.rx_cnt)
    {
        if (read_len > recv_len)
        {
            APP_PRINT_ERROR2("[wifi] error atcmd_recv read_len = %d, recv_len = %d", \
                             read_len, recv_len);
            read_len = recv_len;
        }

        if (wifi_console.read_idx + read_len <= RX_BUF_LENGTH)
        {
            memcpy(recv_buf, &wifi_console.console_buf[wifi_console.read_idx], read_len);
            wifi_console.read_idx += read_len;
            if (wifi_console.read_idx == RX_BUF_LENGTH)
            {
                wifi_console.read_idx = 0;
            }
        }
        else
        {
            uint32_t temp;
            temp = RX_BUF_LENGTH - wifi_console.read_idx;
            memcpy(recv_buf, &wifi_console.console_buf[wifi_console.read_idx], temp);
            memcpy(recv_buf + temp, &wifi_console.console_buf[0], read_len - temp);
            wifi_console.read_idx = read_len - temp;
        }
        wifi_console.rx_cnt -= read_len;
        // APP_PRINT_INFO1("recv_buf =%b", TRACE_BINARY(read_len, recv_buf));
    }
    return read_len;
}


int mp_open(void)
{
    int ret = pinctrl_apply_state_direct(pincfg, &uart3_mp_state);
    APP_PRINT_INFO1("mp_open  %d", ret);
    if (ret == 0)
    {
        uart_set.uart_mode = UART_MPCMD;
        uart_rx_disable(uart3_dev);
        uart_set.buf_index = 0;
        uart_rx_enable(uart3_dev, uart_set.rx_buf, RX_BUF_LENGTH, 0);
    }
    return ret;
}

int mp_close(void)
{
    int ret = pinctrl_apply_state(pincfg, PINCTRL_STATE_DEFAULT);
    APP_PRINT_INFO1("mp_close  %d", ret);
    if (ret == 0)
    {
        uart_set.uart_mode = UART_ATCMD;
        uart_rx_disable(uart3_dev);
        uart_set.buf_index = 0;
        uart_rx_enable(uart3_dev, uart_set.rx_buf, RX_BUF_LENGTH, 0);
    }
    return ret;
}


void mp_send(uint8_t *cmd_buf, uint16_t cmd_len)
{
    uart_tx(uart3_dev, cmd_buf, cmd_len, SYS_FOREVER_US);
}

uint16_t mp_recv(uint8_t *recv_buf, uint16_t recv_len)
{
    uint16_t read_len = 0;
    memset(recv_buf, 0, recv_len);
    if (os_sem_take(uart_set.mp_sem_handle, 1200) == true)
    {
        read_len = wifi_console.rx_cnt;
        if (wifi_console.rx_cnt)
        {
            if (read_len > recv_len)
            {
                APP_PRINT_ERROR2("[wifi] error mp_recv read_len = %d, recv_len = %d", \
                                 read_len, recv_len);
                read_len = recv_len;
            }

            if (wifi_console.read_idx + read_len <= RX_BUF_LENGTH)
            {
                memcpy(recv_buf, &wifi_console.console_buf[wifi_console.read_idx], read_len);
                wifi_console.read_idx += read_len;
                if (wifi_console.read_idx == RX_BUF_LENGTH)
                {
                    wifi_console.read_idx = 0;
                }
            }
            else
            {
                uint32_t temp;
                temp = RX_BUF_LENGTH - wifi_console.read_idx;
                memcpy(recv_buf, &wifi_console.console_buf[wifi_console.read_idx], temp);
                memcpy(recv_buf + temp, &wifi_console.console_buf[0], read_len - temp);
                wifi_console.read_idx = read_len - temp;
            }
            wifi_console.rx_cnt -= read_len;
            // APP_PRINT_INFO1("recv_buf =%b", TRACE_BINARY(read_len, recv_buf));
        }
    }
    else
    {
        // APP_PRINT_INFO0("mp_recv os_sem_take failed\n");
    }
    return read_len;
}

int mp_set_baudrate(uint32_t baud)
{
    int ret = 0;
    struct uart_config cfg;
    uart_config_get(uart3_dev, &cfg);
    cfg.baudrate = baud;
    ret = uart_configure(uart3_dev, &cfg);
    if (ret == 0)
    {
        uart_set.baud = baud;
    }
    uart_rx_disable(uart3_dev);
    uart_set.buf_index = 0;
    uart_rx_enable(uart3_dev, uart_set.rx_buf, RX_BUF_LENGTH, 0);
    return ret;
}

#endif
