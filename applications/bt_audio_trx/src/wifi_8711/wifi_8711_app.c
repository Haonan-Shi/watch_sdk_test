/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*
 * wifi_8711 module entry point.
 *
 * Owns the dedicated wifi_8711 task that handles all SPI send/receive events
 * (master data-in, transfer triggers) and the AT-over-SPI flow control - so the
 * SPI+AT engine no longer runs on the shared app task. Boot-time initialisation
 * (AT engine + SPI master) is gathered here and driven by wifi_8711_init(),
 * which app/app_main.c calls explicitly during app init.
 *
 * Unlike the watch port this app has no APP_MODULE_INIT registry, so the init
 * is an ordinary call rather than an APP_MODULE_INIT() hook. The role/command
 * sub-options (CONFIG_WIFI_8711_ROLE_MASTER / _CMD) each guard their own source
 * file, so every reference below is gated the same way to keep the link clean
 * when a sub-option is off.
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
#include "wdg.h"
#include "wifi_8711_app.h"
#include "app_spi_common.h"
#include "app_spi_api.h"
#include "app_spi_atcmd.h"

/* WIFI_8711_EVENT_SPI_* double as IO_SPI_MSG_TYPE subtypes: app_spi_msg_send()
 * posts the subtype straight through as the event id and app_spi_msg_handle()
 * casts it back, so the two enums must stay value-for-value identical. Catch any
 * reorder at compile time instead of silently misrouting the SPI events. */
BUILD_ASSERT((int)WIFI_8711_EVENT_SPI_MASTER_DATA_IN == (int)IO_SPI_MASTER_DATA_IN);
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
        /* Feed the core system watchdog (RESET_ALL, ~4s) from this task. During a
         * throughput blast the SPI master pump holds DLPS off for the whole
         * transfer, so the platform never enters the sleep path that normally
         * refreshes the watchdog; nothing else in the SPI path kicks it. A slow
         * (weak-signal) blast that stays awake past the ~4s window would then
         * trip a RESET_ALL. RX-parse + trigger events arrive here every frame
         * (~9ms even at 7Mbps), so kicking here keeps the dog fed while the pump
         * is making progress. Deliberately NOT kicked in the low-priority blast
         * task: if the m2s/s2m handshake ever deadlocks, no messages reach this
         * task, the dog is left unfed, and the watchdog reset auto-recovers the
         * stuck pump - which is the behaviour we want. */
        wdg_kick();

        if (os_msg_recv(wifi_8711_msg_queue_handle, &msg, 0xFFFFFFFF) == true)
        {
            switch ((T_WIFI_8711_EVENT)msg.event)
            {
#if defined(CONFIG_WIFI_8711_ROLE_MASTER)
            case WIFI_8711_EVENT_SPI_MASTER_DATA_IN:
            case WIFI_8711_EVENT_SPI_MASTER_TRIGGER:
                /* event id == IO_SPI_MSG_TYPE subtype (enums kept in sync) */
                app_spi_msg_handle((uint8_t)msg.event, msg.buf);
                break;
#endif

#if defined(CONFIG_WIFI_8711_CMD)
            case WIFI_8711_EVENT_ATCMD_FLOW_CTRL:
                spi_atcmd_flow_ctrl_handler();
                break;
#endif

            default:
                break;
            }
        }
    }
}

void wifi_8711_init(void)
{
    /* Create the queue before the task so the task's os_msg_recv() always has a
     * valid handle, and both exist before the SPI master is brought up
     * (its ISR path posts straight to this queue). */
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

#if defined(CONFIG_WIFI_8711_CMD)
    /* AT-over-SPI engine: registers the SPI RX parser callback (and the blast
     * TX task); must run before the SPI master is brought up. */
    app_spi_atcmd_init();
#endif

#if defined(CONFIG_WIFI_8711_ROLE_MASTER)
    app_spi_master_init();
#endif

    APP_PRINT_INFO0("[wifi8711] module init done");
}
