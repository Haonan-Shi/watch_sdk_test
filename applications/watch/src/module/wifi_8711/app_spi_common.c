/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_spi_common.h"
#include "app_spi_api.h"
#include "app_dlps.h"
#include "wifi_8711_app.h"   /* dedicated wifi_8711 task + message queue */
#include "trace.h"

static uint8_t app_spi_xmit_status_bitmask = 0;

void app_spi_disable_dlps(uint8_t bit)
{
    app_spi_xmit_status_bitmask |= bit;
    app_dlps_disable(APP_DLPS_ENTER_CHECK_WIFI_8711);
}

void app_spi_enable_dlps(uint8_t bit)
{
    app_spi_xmit_status_bitmask &= ~bit;
    if (app_spi_xmit_status_bitmask == 0)
    {
        app_dlps_enable(APP_DLPS_ENTER_CHECK_WIFI_8711);
    }
}

bool app_spi_is_transmit_done(void)
{
    return (app_spi_xmit_status_bitmask == 0) ? true : false;
}

bool app_spi_msg_send(IO_SPI_MSG_TYPE subtype, void *param_buf)
{
    /* Post to the dedicated wifi_8711 task instead of the shared app task.
     * WIFI_8711_EVENT_SPI_* mirror IO_SPI_MSG_TYPE one-to-one, so the subtype
     * doubles as the event id. */
    T_WIFI_8711_MSG msg;

    msg.event = (uint16_t)subtype;
    msg.buf   = param_buf;
    // APP_PRINT_INFO1("app_spi_msg_send: subtype %d", subtype);
    return app_send_msg_to_wifi_8711_task(&msg);
}

void app_spi_msg_handle(uint8_t subtype, void *buf)
{
    IO_SPI_MSG_TYPE sub_type = (IO_SPI_MSG_TYPE)subtype;
    APP_PRINT_INFO1("app_spi_msg_handle: subtype %d", sub_type);
    switch (sub_type)
    {
    case IO_SPI_MASTER_DATA_IN:
        {
#if defined(CONFIG_WIFI_8711_ROLE_MASTER)
            app_spi_master_rx_parser(buf);
#endif
        }
        break;

    case IO_SPI_SLAVE_DATA_IN:
        {
#if defined(CONFIG_WIFI_8711_ROLE_SLAVE)
            app_spi_slave_rx_parser(buf);
#endif
        }
        break;

    case IO_SPI_SLAVE_TRIGGER:
        {
#if defined(CONFIG_WIFI_8711_ROLE_SLAVE)
            app_spi_slave_listen_async();
#endif
        }
        break;
    case IO_SPI_MASTER_TRIGGER:
        {
#if defined(CONFIG_WIFI_8711_ROLE_MASTER)
            app_spi_master_send_raw_data_trigger();
#endif
        }
    default:
        break;
    }
}
