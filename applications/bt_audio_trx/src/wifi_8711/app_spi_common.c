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
#include "os_sync.h"

static uint8_t app_spi_xmit_status_bitmask = 0;

/* app_spi_disable_dlps() is called from the SPI completion callback (ISR
 * context) as well as task context, so the read-modify-write of the refcount
 * mask must be atomic; os_lock() masks interrupts around it (app_dlps.c guards
 * its own bitmap the same way). */
void app_spi_disable_dlps(uint8_t bit)
{
    uint32_t s = os_lock();
    app_spi_xmit_status_bitmask |= bit;
    os_unlock(s);

    app_dlps_disable(APP_DLPS_ENTER_CHECK_SPI);
}

void app_spi_enable_dlps(uint8_t bit)
{
    uint32_t s = os_lock();
    app_spi_xmit_status_bitmask &= ~bit;
    bool all_released = (app_spi_xmit_status_bitmask == 0);
    os_unlock(s);

    /* Only re-allow DLPS once the last holder (per-frame XFER + any active blast)
     * has released - this is what collapses the per-frame enable/disable churn
     * into a single disable at blast start and a single enable at blast end. */
    if (all_released)
    {
        app_dlps_enable(APP_DLPS_ENTER_CHECK_SPI);
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
    /* NOTE: no per-message log here - this runs on the SPI hot path (once or
     * twice per 16K frame during a bulk upload). A trace print per frame added
     * measurable per-frame latency; keep this path log-free. */
    switch (sub_type)
    {
    case IO_SPI_MASTER_DATA_IN:
        {
            app_spi_master_rx_parser(buf);
        }
        break;

    case IO_SPI_MASTER_TRIGGER:
        {
            app_spi_master_send_raw_data_trigger();
        }
    default:
        break;
    }
}
