/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <zephyr/drivers/uart.h>
#include "console.h"
#include "aci_uart.h"
#include "app_dlps.h"
#include "trace.h"

const struct device *const uart_dev = DEVICE_DT_GET(DT_NODELABEL(uart0));

P_CONSOLE_CALLBACK aci_console_callback = NULL;
static uint8_t rx_buf[4096] = {0};

static void init_test(void)
{
    __ASSERT_NO_MSG(device_is_ready(uart_dev));
    uart_rx_disable(uart_dev);
    uart_tx_abort(uart_dev);
}

static void aci_uart_async_init(void)
{
    static bool initialized;

    __ASSERT_NO_MSG(device_is_ready(uart_dev));
    uart_rx_disable(uart_dev);
    uart_tx_abort(uart_dev);

    if (!initialized)
    {
        init_test();
        initialized = true;
#ifdef CONFIG_USERSPACE
        set_permissions();
#endif
    }

}

static void aci_uart_async_console_callback(const struct device *dev,
                                            struct uart_event *evt, void *user_data)
{
    APP_PRINT_TRACE1("aci_uart_async_console_callback: type %d", evt->type);

    switch (evt->type)
    {
    case UART_TX_DONE:
        if (aci_console_callback)
        {
            aci_console_callback(CONSOLE_EVT_DATA_XMIT, NULL, evt->data.tx.len);
            app_dlps_enable(APP_DLPS_ENTER_CHECK_UART_TX);
        }
        break;
    case UART_RX_RDY:
        if (aci_console_callback)
        {
            APP_PRINT_TRACE2("aci_uart_async_console_callback: RX len %d, data %b", evt->data.rx.len,
                             TRACE_BINARY(evt->data.rx.len, &evt->data.rx.buf[evt->data.rx.offset]));

            aci_console_callback(CONSOLE_EVT_DATA_IND, &evt->data.rx.buf[evt->data.rx.offset],
                                 evt->data.rx.len);

            uart_rx_disable(uart_dev);
            uart_rx_enable(uart_dev, rx_buf, sizeof(rx_buf), 0 * 1000U);
        }
        break;
    default:
        break;
    }
}

bool aci_uart_tx(uint8_t *buf, uint32_t len)
{
    APP_PRINT_TRACE2("aci_uart_tx: len %d, data %b", len, TRACE_BINARY(len, buf));
    app_dlps_disable(APP_DLPS_ENTER_CHECK_UART_TX);
    uart_tx(uart_dev, buf, len, SYS_FOREVER_US);
    return true;
}

bool aci_uart_init(P_CONSOLE_CALLBACK p_callback)
{
    aci_console_callback = p_callback;

    aci_uart_async_init();

    uart_callback_set(uart_dev, aci_uart_async_console_callback, NULL);

    uart_rx_enable(uart_dev, rx_buf, sizeof(rx_buf), 0 * 1000U);

    return true;
}
