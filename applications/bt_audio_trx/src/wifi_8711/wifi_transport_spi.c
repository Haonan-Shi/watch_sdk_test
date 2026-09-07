/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if defined(CONFIG_WIFI_8711_CMD) || F_APP_WIFI_SPI_CMD

#include "wifi_transport.h"
#include "app_spi_atcmd.h"
#include "spi_file_upload.h"

static bool _spi_register_cb(wifi_at_evt_cb_t cb)
{
    return app_spi_atcmd_register_callback((app_spi_atcmd_cb_t)cb);
}

static bool _spi_queue_fill(uint8_t cmd, const char *param)
{
    return app_spi_atcmd_queue_fill((T_ATCMD_TYPE)cmd, (char *)param);
}

static void _spi_trigger(void)
{
    app_spi_atcmd_trigger_send_flow();
}

static bool _spi_sendraw(const char *cmd, const uint8_t *raw, uint16_t len)
{
    return app_spi_atcmd_sendraw(cmd, raw, len);
}

static int _spi_file_upload_init(void *cfg)
{
    return spi_file_upload_init((const T_SPI_UPLOAD_CFG *)cfg);
}

static bool _spi_file_upload_is_busy(void)
{
    return spi_file_upload_is_busy();
}

static void _spi_file_upload_restore_clk(void)
{
    spi_file_upload_restore_clk();
}

static void _spi_power_on(void)
{
    /* SPI IC power is handled by wifi_8711_app.c */
}

static void _spi_power_down(bool disable)
{
    (void)disable;
}

const wifi_transport_ops_t g_spi_transport =
{
    .name = "spi",
    .init = NULL,
    .register_callback = _spi_register_cb,
    .queue_fill = _spi_queue_fill,
    .trigger_send = _spi_trigger,
    .sendraw = _spi_sendraw,
    .file_upload_init = _spi_file_upload_init,
    .file_upload_is_busy = _spi_file_upload_is_busy,
    .file_upload_restore_clk = _spi_file_upload_restore_clk,
    .power_on = _spi_power_on,
    .power_down = _spi_power_down,
};

#endif /* CONFIG_WIFI_8711_CMD || F_APP_WIFI_SPI_CMD */
