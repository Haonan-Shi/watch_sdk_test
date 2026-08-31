/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if F_APP_WIFI_UART_CMD

#include "wifi_transport.h"
#include "wifi/app_uart_atcmd.h"
#include "wifi/wifi_file_upload.h"

static bool _uart_register_cb(wifi_at_evt_cb_t cb)
{
    app_uart_atcmd_register_callback((app_uart_atcmd_cb_t)cb);
    return true;
}

static bool _uart_queue_fill(uint8_t cmd, const char *param)
{
    return app_uart_atcmd_queue_fill((T_ATCMD_TYPE)cmd, param);
}

static void _uart_trigger(void)
{
    app_uart_atcmd_trigger_send_flow();
}

static bool _uart_sendraw(const char *cmd, const uint8_t *raw, uint16_t len)
{
    (void)cmd;
    (void)raw;
    (void)len;
    return false; /* UART has no SKTSENDRAW */
}

static int _uart_file_upload_init(void *cfg)
{
    return wifi_file_upload_init((const T_WIFI_UPLOAD_CFG *)cfg);
}

static bool _uart_file_upload_is_busy(void)
{
    return wifi_file_upload_is_busy();
}

static void _uart_file_upload_restore_clk(void)
{
    wifi_file_upload_restore_clk();
}

static void _uart_power_on(void)
{
    extern void app_ai_record_wifi_power_on(void);
    app_ai_record_wifi_power_on();
}

static void _uart_power_down(bool disable)
{
    (void)disable;
}

const wifi_transport_ops_t g_uart_transport =
{
    .name = "uart",
    .init = NULL,
    .register_callback = _uart_register_cb,
    .queue_fill = _uart_queue_fill,
    .trigger_send = _uart_trigger,
    .sendraw = _uart_sendraw,
    .file_upload_init = _uart_file_upload_init,
    .file_upload_is_busy = _uart_file_upload_is_busy,
    .file_upload_restore_clk = _uart_file_upload_restore_clk,
    .power_on = _uart_power_on,
    .power_down = _uart_power_down,
};

#endif /* F_APP_WIFI_UART_CMD */
