/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
*                        Header Files
*============================================================================*/

#if CONFIG_WALKIE_TALKIE
#include "event_bus.h"
#include "bridge_intercom.h"
#include "gui_listener.h"
#include "app_main.h"
#include "app_task.h"
#include "walkie_talkie_app.h"
#include "walkie_talkie_adv.h"
#include "trace.h"
#include <string.h>

/*============================================================================*
 *                            Macros
 *============================================================================*/

/*============================================================================*
 *                           Types
 *============================================================================*/

/*============================================================================*
 *                           Constants
 *============================================================================*/

/*============================================================================*
 *                            Variables
 *============================================================================*/

static T_EVENT_BUS_SUBSCRIBER_HANDLE walkie_talkie_app_event_handle;
static T_EVENT_BUS_SUBSCRIBER_HANDLE intercom_async_event_handle;

/*============================================================================*
 *                           Private Functions
 *============================================================================*/

static int32_t intercom_app_to_gui(T_EVENT_BUS_EVENT_DATA *event_data)
{
    if (strcmp(event_data->topic, EVENT_BUS_TOPIC_WALKIE_TALKIE_CONNECTED) == 0)
    {
        // Handle connected event
        walkie_talkie_receive_start();
        gui_msg_publish("walkie_talkie_conn", NULL, 0);
    }
    else if (strcmp(event_data->topic, EVENT_BUS_TOPIC_WALKIE_TALKIE_DISCONNECTED) == 0)
    {
        // Handle disconnected event
        walkie_talkie_receive_stop();
        gui_msg_publish("walkie_talkie_disconn", NULL, 0);
    }
    else if (strcmp(event_data->topic, EVENT_BUS_TOPIC_WALKIE_TALKIE_SCAN_REPORT) == 0)
    {
        // Handle scan report event
        // publish to gui on EVENT_BUS_TOPIC_WALKIE_TALKIE_DEV_INFO_UPDATE topic
        walkie_talkie_scan_report(&event_data->data);
    }
    else if (strcmp(event_data->topic, EVENT_BUS_TOPIC_WALKIE_TALKIE_USER_NAME) == 0)
    {
        // Handle user name event
        walkie_talkie_save_dev_name(&event_data->data);
        gui_msg_publish("walkie_talkie_user_name", &event_data->data, event_data->data_len);
    }
    else if (strcmp(event_data->topic, EVENT_BUS_TOPIC_WALKIE_TALKIE_RECEIVE_START) == 0)
    {
        gui_msg_publish("walkie_talkie_receive_start", NULL, 0);
    }
    else if (strcmp(event_data->topic, EVENT_BUS_TOPIC_WALKIE_TALKIE_RECEIVE_STOP) == 0)
    {
        gui_msg_publish("walkie_talkie_receive_stop", NULL, 0);
    }
    else if (strcmp(event_data->topic, EVENT_BUS_TOPIC_WALKIE_TALKIE_DEV_INFO_UPDATE) == 0)
    {
        gui_msg_publish("walkie_talkie_scan_report", &event_data->data, event_data->data_len);
    }

    return 0;
}

static int32_t walkie_talkie_app_handle_msg(T_EVENT_BUS_EVENT_DATA *event_data)
{
    APP_PRINT_INFO3("walkie_talkie_app_handle_msg: topic %s, data 0x%p, len %d",
                    TRACE_STRING(event_data->topic), event_data->data, event_data->data_len);

    if (strcmp(event_data->topic, EVENT_BUS_TOPIC_INTERCOM_GUI_ON) == 0)
    {
        walkie_talkie_on();
    }
    else if (strcmp(event_data->topic, EVENT_BUS_TOPIC_INTERCOM_GUI_OFF) == 0)
    {
        walkie_talkie_off();
    }
    else if (strcmp(event_data->topic, EVENT_BUS_TOPIC_INTERCOM_GUI_CONNECT_DEV1) == 0)
    {
        walkie_talkie_connect_dev(0);
    }
    else if (strcmp(event_data->topic, EVENT_BUS_TOPIC_INTERCOM_GUI_CONNECT_DEV2) == 0)
    {
        walkie_talkie_connect_dev(1);
    }
    else if (strcmp(event_data->topic, EVENT_BUS_TOPIC_INTERCOM_GUI_CONNECT_DEV3) == 0)
    {
        walkie_talkie_connect_dev(2);
    }
    else if (strcmp(event_data->topic, EVENT_BUS_TOPIC_INTERCOM_TRANSMIT_START) == 0)
    {
        walkie_talkie_transmit_start();
        walkie_talkie_transmit_start_send_to_ble();
    }
    else if (strcmp(event_data->topic, EVENT_BUS_TOPIC_INTERCOM_TRANSMIT_STOP) == 0)
    {
        walkie_talkie_transmit_stop();
        walkie_talkie_transmit_stop_send_to_ble();
    }
    else if (strcmp(event_data->topic, EVENT_BUS_TOPIC_INTERCOM_GUI_DISCONNECT) == 0)
    {
        walkie_talkie_disconnect();
    }

    return 0;
}


/*============================================================================*
 *                           Public Functions
 *============================================================================*/
bool intercom_gui_to_app(const char *topic, void *data, uint32_t size)
{
    return event_bus_publish(topic, data, size) == EVENT_BUS_OK;
}

void bridge_intercom_init(void)
{
    /*create intercom topic*/
    event_bus_topic_register(EVENT_BUG_TOPIC_INTERCOM_ALL_TOPIC);
    /*subscribe to intercom topic asynchronously*/
    event_bus_subscribe_async(&intercom_async_event_handle,
                              EVENT_BUG_TOPIC_INTERCOM_ALL_TOPIC,
                              event_bus_async_send_to_apptask,
                              NULL,
                              walkie_talkie_app_handle_msg);

    /*subscribe to walkie talkie app topic*/
    event_bus_subscribe(&walkie_talkie_app_event_handle,
                        EVENT_BUS_TOPIC_WALKIE_TALKIE_ALL_TOPIC,
                        intercom_app_to_gui);

    APP_PRINT_INFO0("[intercom] initialized");
}
#endif