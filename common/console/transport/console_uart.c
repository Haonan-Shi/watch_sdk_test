/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <zephyr/devicetree.h>
#include "trace.h"

/*
 * Dual-path console UART transport.
 *
 * When a board/overlay selects a console UART through the devicetree chosen node
 *     chosen { realtek,console-uart = &uartX; };
 * this transport binds to that UART device. A UART with both RX and TX DMA
 * channels uses Zephyr's async UART API; a UART without DMA uses the
 * interrupt-driven API. The hardware instance, pins, baud rate and DMA channels
 * all come from devicetree, so a board can route the console onto any UART by
 * editing its overlay only.
 *
 */

#if DT_HAS_CHOSEN(realtek_console_uart)
#define CONSOLE_UART_ZEPHYR_NODE          DT_CHOSEN(realtek_console_uart)
#define CONSOLE_UART_DT_RX_DMA_ENABLED    DT_DMAS_HAS_NAME(CONSOLE_UART_ZEPHYR_NODE, rx)
#define CONSOLE_UART_DT_TX_DMA_ENABLED    DT_DMAS_HAS_NAME(CONSOLE_UART_ZEPHYR_NODE, tx)
#else
#error "The dts node (realtek,console-uart) should be defined!"
#endif

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/irq.h>
#include <zephyr/kernel.h>
#include "console.h"
#include "console_uart.h"

#if CONSOLE_UART_DT_RX_DMA_ENABLED != CONSOLE_UART_DT_TX_DMA_ENABLED
#error "realtek,console-uart must configure both RX and TX DMA or neither"
#endif

#if CONSOLE_UART_DT_RX_DMA_ENABLED
#if !defined(CONFIG_UART_ASYNC_API)
#error "DMA console UART requires CONFIG_UART_ASYNC_API=y"
#endif
#define CONSOLE_UART_USE_ASYNC_DMA 1
#else
#if !defined(CONFIG_UART_INTERRUPT_DRIVEN)
#error "Non-DMA console UART requires CONFIG_UART_INTERRUPT_DRIVEN=y"
#endif
#define CONSOLE_UART_USE_IRQ 1
#endif

/*
 * timeout 0: the rtl87x3g async driver flushes RX on every UART idle, giving
 * per-keystroke delivery suitable for an interactive console.
 */
#define CONSOLE_UART_RX_TIMEOUT  0

static const struct device *const console_uart_dev = DEVICE_DT_GET(CONSOLE_UART_ZEPHYR_NODE);

T_CONSOLE_UART_CONFIG g_console_uart;
P_CONSOLE_CALLBACK p_console_callback = NULL;

/* One allocation is split into two buffers for seamless async DMA RX. */
static uint8_t *console_uart_rx_mem;
static uint8_t *console_uart_rx_buf[2];
static uint16_t console_uart_rx_buf_size;

#if defined(CONSOLE_UART_USE_IRQ)
static uint8_t *console_uart_tx_buf;
static uint32_t console_uart_tx_len;
static uint32_t console_uart_tx_offset;
static bool console_uart_tx_active;
#else
static uint8_t console_uart_rx_next;
#endif
static bool console_uart_initialized;

static void console_uart_notify(T_CONSOLE_EVT evt, uint8_t *buf, uint32_t len)
{
    if (p_console_callback)
    {
        p_console_callback(evt, buf, len);
    }
}

static bool console_uart_alloc_rx_buffers(void)
{
    uint8_t buffer_count =
#if defined(CONSOLE_UART_USE_ASYNC_DMA)
        2U;
#else
        1U;
#endif

    if (console_uart_rx_mem)
    {
        return true;
    }

    console_uart_rx_buf_size = g_console_uart.uart_dma_rx_buffer_size;
    if (console_uart_rx_buf_size == 0U)
    {
        return false;
    }

    console_uart_rx_mem = malloc((size_t)buffer_count * console_uart_rx_buf_size);
    if (!console_uart_rx_mem)
    {
        return false;
    }

    console_uart_rx_buf[0] = console_uart_rx_mem;
#if defined(CONSOLE_UART_USE_ASYNC_DMA)
    console_uart_rx_buf[1] = console_uart_rx_mem + console_uart_rx_buf_size;
#endif
    return true;
}

static void console_uart_free_rx_buffers(void)
{
    free(console_uart_rx_mem);
    console_uart_rx_mem = NULL;
    console_uart_rx_buf[0] = NULL;
#if defined(CONSOLE_UART_USE_ASYNC_DMA)
    console_uart_rx_buf[1] = NULL;
#endif
    console_uart_rx_buf_size = 0U;
}

#if defined(CONSOLE_UART_USE_ASYNC_DMA)
static bool console_uart_restart_async_rx(const struct device *dev)
{
    int ret;

    console_uart_rx_next = 1U;
    ret = uart_rx_enable(dev, console_uart_rx_buf[0], console_uart_rx_buf_size,
                         CONSOLE_UART_RX_TIMEOUT);
    if (ret != 0)
    {
        console_uart_notify(CONSOLE_EVT_ERROR, NULL, 0);
        return false;
    }
    return true;
}

static void console_uart_async_cb(const struct device *dev,
                                  struct uart_event *evt, void *user_data)
{
    (void)user_data;

    switch (evt->type)
    {
    case UART_TX_DONE:
        console_uart_notify(CONSOLE_EVT_DATA_XMIT, NULL, evt->data.tx.len);
        break;

    case UART_TX_ABORTED:
        if (evt->data.tx.len)
        {
            console_uart_notify(CONSOLE_EVT_DATA_XMIT, NULL, evt->data.tx.len);
        }
        console_uart_notify(CONSOLE_EVT_ERROR, NULL, 0);
        break;

    case UART_RX_RDY:
        /* new bytes live at buf[offset..offset+len); the console layer copies
         * them out synchronously, so handing a transient pointer is safe. */
        if (evt->data.rx.len)
        {
            console_uart_notify(CONSOLE_EVT_DATA_IND,
                                &evt->data.rx.buf[evt->data.rx.offset],
                                evt->data.rx.len);
        }
        break;

    case UART_RX_BUF_REQUEST:
        if (uart_rx_buf_rsp(dev, console_uart_rx_buf[console_uart_rx_next],
                            console_uart_rx_buf_size) == 0)
        {
            console_uart_rx_next ^= 1U;
        }
        else
        {
            console_uart_notify(CONSOLE_EVT_ERROR, NULL, 0);
        }
        break;

    case UART_RX_DISABLED:
        if (console_uart_initialized)
        {
            (void)console_uart_restart_async_rx(dev);
        }
        break;

    case UART_RX_STOPPED:
        console_uart_notify(CONSOLE_EVT_ERROR, NULL, 0);
        break;

    case UART_RX_BUF_RELEASED:
    default:
        break;
    }
}
#else
static void console_uart_irq_cb(const struct device *dev, void *user_data)
{
    int received;

    (void)user_data;

    if (!uart_irq_update(dev))
    {
        return;
    }

    if (uart_err_check(dev) > 0)
    {
        console_uart_notify(CONSOLE_EVT_ERROR, NULL, 0);
    }

    while (uart_irq_rx_ready(dev))
    {
        received = uart_fifo_read(dev, console_uart_rx_buf[0], console_uart_rx_buf_size);
        if (received <= 0)
        {
            break;
        }
        console_uart_notify(CONSOLE_EVT_DATA_IND, console_uart_rx_buf[0], received);
    }

    if (uart_irq_tx_ready(dev))
    {
        if (!console_uart_tx_active)
        {
            uart_irq_tx_disable(dev);
            return;
        }

        if (console_uart_tx_offset < console_uart_tx_len)
        {
            received = uart_fifo_fill(dev, &console_uart_tx_buf[console_uart_tx_offset],
                                      console_uart_tx_len - console_uart_tx_offset);
            if (received > 0)
            {
                console_uart_tx_offset += received;
            }
        }

        if ((console_uart_tx_offset == console_uart_tx_len) && uart_irq_tx_complete(dev))
        {
            uint32_t sent_len = console_uart_tx_len;

            console_uart_tx_active = false;
            uart_irq_tx_disable(dev);
            console_uart_notify(CONSOLE_EVT_DATA_XMIT, NULL, sent_len);
        }
    }
}
#endif

/*
 * The application invokes console_uart_exit_low_power() from a
 * POWER_STAGE_RESTORE callback.  Defer RX rearming until the power manager has
 * completed the DLPS restore sequence and the system workqueue can run in
 * normal thread context.
 */
static void console_uart_exit_low_power_work_handler(struct k_work *work)
{
    (void)work;

    if (!console_uart_initialized)
    {
        return;
    }

#if defined(CONSOLE_UART_USE_ASYNC_DMA)
    /* UART_RX_DISABLED restarts DMA RX through console_uart_async_cb(). */
    int ret = uart_rx_disable(console_uart_dev);

    /*
     * The RTL driver emits UART_RX_DISABLED even when RX was already stopped,
     * then returns -EFAULT.  The callback restarts DMA in both cases.
     */
    if ((ret != 0) && (ret != -EFAULT))
    {
        console_uart_notify(CONSOLE_EVT_ERROR, NULL, 0);
    }
#endif

    if (g_console_uart.callback)
    {
        g_console_uart.callback(CONSOLE_UART_EVENT_WAKE_UP);
    }
}

K_WORK_DEFINE(console_uart_exit_low_power_work, console_uart_exit_low_power_work_handler);

bool console_uart_write(uint8_t *buf, uint32_t len)
{
#if defined(CONSOLE_UART_USE_ASYNC_DMA)
    if (!console_uart_initialized)
    {
        return false;
    }
    return uart_tx(console_uart_dev, buf, len, SYS_FOREVER_US) == 0;
#else
    unsigned int key;

    if (!console_uart_initialized || !buf || !len)
    {
        return false;
    }

    key = irq_lock();
    if (console_uart_tx_active)
    {
        irq_unlock(key);
        return false;
    }

    console_uart_tx_buf = buf;
    console_uart_tx_len = len;
    console_uart_tx_offset = 0;
    console_uart_tx_active = true;
    irq_unlock(key);

    uart_irq_tx_enable(console_uart_dev);
    return true;
#endif
}

bool console_uart_init(P_CONSOLE_CALLBACK p_callback)
{
    if (!device_is_ready(console_uart_dev))
    {
        return false;
    }

    if (console_uart_initialized)
    {
        return p_console_callback == p_callback;
    }

    if (!console_uart_alloc_rx_buffers())
    {
        return false;
    }

#if defined(CONSOLE_UART_USE_ASYNC_DMA)
    if (uart_callback_set(console_uart_dev, console_uart_async_cb, NULL) != 0)
    {
        console_uart_free_rx_buffers();
        return false;
    }

    console_uart_rx_next = 1U;
    if (uart_rx_enable(console_uart_dev, console_uart_rx_buf[0], console_uart_rx_buf_size,
                       CONSOLE_UART_RX_TIMEOUT) != 0)
    {
        console_uart_free_rx_buffers();
        console_uart_rx_next = 0U;
        return false;
    }
#else
    if (uart_irq_callback_user_data_set(console_uart_dev, console_uart_irq_cb, NULL) != 0)
    {
        console_uart_free_rx_buffers();
        return false;
    }
    uart_irq_rx_enable(console_uart_dev);
#endif

    p_console_callback = p_callback;
    console_uart_initialized = true;

    if (p_console_callback)
    {
        return p_console_callback(CONSOLE_EVT_OPENED, NULL, 0);
    }
    return true;
}

void console_uart_config_init(T_CONSOLE_UART_CONFIG *cfg)
{
    memcpy(&g_console_uart, cfg, sizeof(T_CONSOLE_UART_CONFIG));
}

void console_uart_exit_low_power(POWERMode mode)
{
    (void)mode;

    if (!console_uart_initialized)
    {
        return;
    }

    (void)k_work_submit(&console_uart_exit_low_power_work);
}

/* Hardware bring-up and wake pin configuration are owned by devicetree. */
void console_uart_driver_init(void) { }
bool console_uart_wakeup(void) { return true; }
bool console_uart_tx_wakeup_enable(uint8_t pin) { (void)pin; return true; }
bool console_uart_rx_wakeup_enable(uint8_t pin) { (void)pin; return true; }
void console_uart_enter_low_power(POWERMode mode) { (void)mode; }
