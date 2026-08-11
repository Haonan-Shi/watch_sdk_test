/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*
 * wifi_8711 module entry point.
 *
 * Owns the dedicated wifi_8711 task that handles all SPI send/receive events
 * (master/slave data-in, transfer triggers) and the AT-over-SPI flow control -
 * so the SPI+AT engine no longer runs on the shared app task. Boot-time
 * initialisation (AT engine + SPI master) is hooked into APP_MODULE_INIT,
 * mirroring module/wifi/wifi_app.c, and replaces the explicit calls that used
 * to live in app/main.c.
 *
 * Note: the AT engine still owns a separate, deliberately low-priority "blast"
 * TX streaming task (SPI_TCP_TP_TX_TASK in app_spi_atcmd.c); it stays on its
 * own task so a long throughput flood never blocks this task from draining RX.
 */

#include <zephyr/kernel.h>
#include "trace.h"
#include "os_msg.h"
#include "os_task.h"
#include "os_sched.h"
#include "app_module_init.h"
#include "wifi_8711_app.h"
#include "app_spi_common.h"
#include "app_spi_api.h"
#include "app_spi_atcmd.h"

/* WIFI_8711_EVENT_SPI_* double as IO_SPI_MSG_TYPE subtypes: app_spi_msg_send()
 * posts the subtype straight through as the event id and app_spi_msg_handle()
 * casts it back, so the two enums must stay value-for-value identical. Catch any
 * reorder at compile time instead of silently misrouting the SPI events. */
BUILD_ASSERT((int)WIFI_8711_EVENT_SPI_MASTER_DATA_IN == (int)IO_SPI_MASTER_DATA_IN);
BUILD_ASSERT((int)WIFI_8711_EVENT_SPI_SLAVE_DATA_IN  == (int)IO_SPI_SLAVE_DATA_IN);
BUILD_ASSERT((int)WIFI_8711_EVENT_SPI_SLAVE_TRIGGER  == (int)IO_SPI_SLAVE_TRIGGER);
BUILD_ASSERT((int)WIFI_8711_EVENT_SPI_MASTER_TRIGGER == (int)IO_SPI_MASTER_TRIGGER);

#define WIFI_8711_MSG_QUEUE_DEPTH   0x10
#define WIFI_8711_TASK_STACK_SIZE   (1024 * 6)
#define WIFI_8711_TASK_PRIORITY     2   /* same band as the app / wifi tasks */

static void *wifi_8711_msg_queue_handle = NULL;
static void *wifi_8711_task_handle      = NULL;

static void wifi_8711_task(void *p_param);

bool app_send_msg_to_wifi_8711_task(T_WIFI_8711_MSG *p_msg)
{
    if (wifi_8711_msg_queue_handle == NULL ||
        os_msg_send(wifi_8711_msg_queue_handle, p_msg, 0) == false)
    {
        APP_PRINT_ERROR0("[wifi8711] send msg to wifi_8711 task fail !");
        return false;
    }
    return true;
}

static void wifi_8711_task(void *p_param)
{
    T_WIFI_8711_MSG msg;

    (void)p_param;

    while (1)
    {
        if (os_msg_recv(wifi_8711_msg_queue_handle, &msg, 0xFFFFFFFF) == true)
        {
            switch ((T_WIFI_8711_EVENT)msg.event)
            {
            case WIFI_8711_EVENT_SPI_MASTER_DATA_IN:
            case WIFI_8711_EVENT_SPI_SLAVE_DATA_IN:
            case WIFI_8711_EVENT_SPI_SLAVE_TRIGGER:
            case WIFI_8711_EVENT_SPI_MASTER_TRIGGER:
                /* event id == IO_SPI_MSG_TYPE subtype (enums kept in sync) */
                app_spi_msg_handle((uint8_t)msg.event, msg.buf);
                break;

            case WIFI_8711_EVENT_ATCMD_FLOW_CTRL:
                spi_atcmd_flow_ctrl_handler();
                break;

            default:
                break;
            }
        }
    }
}

void wifi_8711_init(void)
{
    /* Create the queue before the task so the task's os_msg_recv() always has a
     * valid handle. */
    if (wifi_8711_msg_queue_handle == NULL)
    {
        os_msg_queue_create(&wifi_8711_msg_queue_handle, "wifi_8711 msg queue",
                            WIFI_8711_MSG_QUEUE_DEPTH, sizeof(T_WIFI_8711_MSG));
    }
    if (wifi_8711_task_handle == NULL)
    {
        os_task_create(&wifi_8711_task_handle, "wifi_8711 task", wifi_8711_task,
                       NULL, WIFI_8711_TASK_STACK_SIZE, WIFI_8711_TASK_PRIORITY);
    }

    /* AT-over-SPI engine: registers the SPI RX parser callback (and the blast
     * TX task); must run before the SPI master is brought up. */
    app_spi_atcmd_init();

#if defined(CONFIG_WIFI_8711_ROLE_MASTER)
    app_spi_master_init();
#endif

    APP_PRINT_INFO0("[wifi8711] module init done");
}

static void wifi_8711_module_init(void)
{
    wifi_8711_init();
}
APP_MODULE_INIT(wifi_8711_module_init);
