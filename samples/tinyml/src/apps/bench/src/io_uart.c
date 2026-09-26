/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*
 * Zephyr interrupt-driven UART driver for the tinyml edge host protocol.
 *
 * Port of the vendor io_uart.c (register-level UART3 on P3_0/P3_1).  The physical
 * pins and baud rate are configured by devicetree (board node "uart2", enabled
 * by the sample overlay, current-speed = 115200).  Here we only attach an RX
 * interrupt callback that accumulates incoming bytes and, after a short line
 * idle, signals the worker thread with IO_MSG_UART_RX_DONE.  The worker then
 * pulls the bytes via uart_drain_rx() and feeds the framed-protocol parser.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <string.h>

#include "io_uart.h"
#include "trace.h"
#include "app_msg.h"
#include "tinyml_main.h"

/* Data UART = board "uart2" node (P3_0 RX / P3_1 TX, 115200 baud). */
static const struct device *const uart_dev = DEVICE_DT_GET(DT_NODELABEL(uart2));

/* Line-idle window after which a received chunk is handed to the parser. */
#define RX_IDLE_MS   5

/* RX accumulation buffer, shared between the ISR and uart_drain_rx().
 * Matches the vendor UART_Recv_Data_Buff sizing. */
static uint8_t  s_rx_buf[2048];
static volatile uint16_t s_rx_len;

static struct k_timer rx_idle_timer;

static void rx_idle_expiry(struct k_timer *timer)
{
    ARG_UNUSED(timer);

    if (s_rx_len == 0)
    {
        return;
    }

    /* ISR/timer context: k_msgq_put (inside tinyml_send_msg_to_task) is
     * ISR-safe.  The worker snapshots the buffer via uart_drain_rx(). */
    T_IO_MSG uart_msg =
    {
        .type    = IO_MSG_TYPE_UART,
        .subtype = IO_MSG_UART_RX_DONE,
        .len     = 0,
        .u.buf   = NULL,
    };
    tinyml_send_msg_to_task(&uart_msg);
}

static void uart_isr(const struct device *dev, void *user_data)
{
    ARG_UNUSED(user_data);

    if (!uart_irq_update(dev))
    {
        return;
    }

    bool got_bytes = false;

    while (uart_irq_rx_ready(dev))
    {
        uint8_t tmp[64]; /* mirrors vendor UART_Rev_Temp_Buf (FIFO depth) */
        int n = uart_fifo_read(dev, tmp, sizeof(tmp));

        if (n <= 0)
        {
            break;
        }

        uint16_t space = (uint16_t)(sizeof(s_rx_buf) - s_rx_len);
        uint16_t copy = (n > space) ? space : (uint16_t)n;

        if (copy)
        {
            memcpy(&s_rx_buf[s_rx_len], tmp, copy);
            s_rx_len += copy;
            got_bytes = true;
        }
        /* Overflow bytes are dropped; the framed parser resynchronises on the
         * next HEAD/idle boundary. */
    }

    if (got_bytes)
    {
        /* (Re)arm the line-idle timeout that flushes the chunk to the parser. */
        k_timer_start(&rx_idle_timer, K_MSEC(RX_IDLE_MS), K_NO_WAIT);
    }
}

void uart_init(void)
{
    if (!device_is_ready(uart_dev))
    {
        DBG_DIRECT("edge: uart2 device not ready");
        return;
    }

    k_timer_init(&rx_idle_timer, rx_idle_expiry, NULL);

    uart_irq_rx_disable(uart_dev);
    uart_irq_tx_disable(uart_dev);
    uart_irq_callback_user_data_set(uart_dev, uart_isr, NULL);
    uart_irq_rx_enable(uart_dev);

    const char *banner = "### tinyml edge (zephyr) ready ###\r\n";
    uart_senddata((uint8_t *)banner, (uint16_t)strlen(banner));
}

void uart_senddata(uint8_t *pSend_Buf, uint16_t vCount)
{
    if (pSend_Buf == NULL || vCount == 0)
    {
        return;
    }

    for (uint16_t i = 0; i < vCount; i++)
    {
        uart_poll_out(uart_dev, pSend_Buf[i]);
    }
}

uint16_t uart_drain_rx(uint8_t *dst, uint16_t dst_size)
{
    unsigned int key = irq_lock();

    uint16_t n = s_rx_len;
    if (n > dst_size)
    {
        n = dst_size;
    }
    if (n)
    {
        memcpy(dst, s_rx_buf, n);
    }
    s_rx_len = 0;

    irq_unlock(key);
    return n;
}
