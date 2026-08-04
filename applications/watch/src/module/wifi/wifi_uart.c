/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <string.h>
#include <stdio.h>
#include "trace.h"
#include "wifi_atcmd.h"
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/uart.h>
#include "os_mem.h"
#include "os_sync.h"
#include "wifi_app.h"
#include "wifi_uart.h"

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
    uint8_t console_buf[RX_BUF_LENGTH];
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


const struct device *const uart_dev = DEVICE_DT_GET(DT_ALIAS(uart3));

T_WIFI_UART_SET uart_set = {.uart_mode = UART_ATCMD, .baud = 115200};
T_CONSOLE wifi_console =
{
    .read_idx = 0,
    .write_idx = 0,
    .rx_cnt = 0,
    .tx_lock = false,
};


static void uart_async_callback(const struct device *dev, struct uart_event *evt, void *user_data)
{
    // APP_PRINT_TRACE1("uart_async_console_callback: type %d", evt->type);

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
            APP_PRINT_TRACE3("uart_rx: buf = 0x%x, len %d, data %b", &evt->data.rx.buf[0], evt->data.rx.len,
                             TRACE_BINARY(evt->data.rx.len, &evt->data.rx.buf[evt->data.rx.offset]));

            wifi_uart_console_recv(&evt->data.rx.buf[evt->data.rx.offset], evt->data.rx.len);
            if (uart_set.uart_mode == UART_ATCMD)
            {
                T_WIFI_MSG rsp_msg;
                rsp_msg.event = EVENT_UART_RX;
                if (app_send_msg_to_wifitask(&rsp_msg) == false)
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
            uart_rx_buf_rsp(uart_dev,  &uart_set.rx_buf[uart_set.buf_index * RX_BUF_LENGTH], RX_BUF_LENGTH);
            APP_PRINT_TRACE1("uart_async_console_callback: set buf %d", uart_set.buf_index);
        }
        break;
    default:
        break;
    }
}

void wifi_uart_init(void)
{
    if (!device_is_ready(uart_dev))
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
    uart_callback_set(uart_dev, uart_async_callback, NULL);
    uart_set.buf_index = 0;
    uart_rx_disable(uart_dev);
    uart_rx_enable(uart_dev, uart_set.rx_buf, RX_BUF_LENGTH, 0);
    uart_atcmd_init();
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
        uart_tx(uart_dev, wifi_console.tx_buf, size, SYS_FOREVER_US);
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
        APP_PRINT_INFO1("recv_buf =%b", TRACE_BINARY(read_len, recv_buf));
    }
    return read_len;
}


int mp_open(void)
{
    APP_PRINT_INFO0("mp_open");
    uart_set.uart_mode = UART_MPCMD;
    uart_rx_disable(uart_dev);
    uart_set.buf_index = 0;
    uart_rx_enable(uart_dev, uart_set.rx_buf, RX_BUF_LENGTH, 0);
    return 0;
}

int mp_close(void)
{
    APP_PRINT_INFO0("mp_close");
    uart_set.uart_mode = UART_ATCMD;
    uart_rx_disable(uart_dev);
    uart_set.buf_index = 0;
    uart_rx_enable(uart_dev, uart_set.rx_buf, RX_BUF_LENGTH, 0);
    return 0;
}


void mp_send(uint8_t *cmd_buf, uint16_t cmd_len)
{
    uart_tx(uart_dev, cmd_buf, cmd_len, SYS_FOREVER_US);
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
    uart_config_get(uart_dev, &cfg);
    cfg.baudrate = baud;
    ret = uart_configure(uart_dev, &cfg);
    if (ret == 0)
    {
        uart_set.baud = baud;
    }
    uart_rx_disable(uart_dev);
    uart_set.buf_index = 0;
    uart_rx_enable(uart_dev, uart_set.rx_buf, RX_BUF_LENGTH, 0);
    return ret;
}
