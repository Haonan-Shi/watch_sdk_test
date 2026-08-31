/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "wifi_transport.h"

const wifi_transport_ops_t *wifi_transport_get(void)
{
#if defined(CONFIG_WIFI_8711_CMD) || F_APP_WIFI_SPI_CMD
    extern const wifi_transport_ops_t g_spi_transport;
    return &g_spi_transport;
#elif F_APP_WIFI_UART_CMD
    extern const wifi_transport_ops_t g_uart_transport;
    return &g_uart_transport;
#else
    return NULL;
#endif
}

bool wifi_transport_register_callback(wifi_at_evt_cb_t cb)
{
    const wifi_transport_ops_t *t = wifi_transport_get();
    if (t && t->register_callback)
    {
        return t->register_callback(cb);
    }
    return false;
}
