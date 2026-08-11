/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
*                        Header Files
*============================================================================*/

#include <string.h>
#include "event_bus.h"
#include "bridge_phone_call.h"
#include "gui_listener.h"
#include "app_mmi.h"
#include "app_hfp.h"
#include "app_pbap.h"
#include "app_audio_if.h"
#include "audio_hfp.h"
#include "app_task.h"
#include "trace.h"

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

static T_PHONE_CALL_STATE s_phone_call;
static T_EVENT_BUS_SUBSCRIBER_HANDLE s_phone_call_async_handle;
static T_EVENT_BUS_SUBSCRIBER_HANDLE s_hfp_handle;
static T_EVENT_BUS_SUBSCRIBER_HANDLE s_hfp_audio_handle;
static T_EVENT_BUS_SUBSCRIBER_HANDLE s_pbap_handle;

/*============================================================================*
 *                           Private Functions
 *============================================================================*/

/**
 * @brief map app hfp call status to phone_call state enum.
 */
static T_PHONE_CALL_STATE_E phone_call_map_call_status(T_APP_HFP_CALL_STATUS status)
{
    switch (status)
    {
    case APP_HFP_CALL_INCOMING:
        return PHONE_CALL_STATE_INCOMING;
    case APP_HFP_CALL_OUTGOING:
        return PHONE_CALL_STATE_OUTGOING;
    case APP_HFP_CALL_ACTIVE:
    case APP_HFP_CALL_ACTIVE_WITH_CALL_WAITING:
    case APP_HFP_CALL_ACTIVE_WITH_CALL_HELD:
    case APP_HFP_MULTILINK_CALL_ACTIVE_WITH_CALL_WAIT:
    case APP_HFP_MULTILINK_CALL_ACTIVE_WITH_CALL_HOLD:
        return PHONE_CALL_STATE_ACTIVE;
    default:
        return PHONE_CALL_STATE_IDLE;
    }
}

/**
 * @brief copy a NUL-terminated string into a fixed-size buffer with truncation,
 *        and report the stored length.
 */
static void phone_call_copy_str(char *dst, uint8_t dst_cap, uint8_t *out_len,
                                const char *src)
{
    if (dst == NULL || dst_cap == 0)
    {
        return;
    }

    if (src == NULL)
    {
        dst[0] = '\0';
        if (out_len != NULL) { *out_len = 0; }
        return;
    }

    uint8_t len = (uint8_t)strnlen(src, dst_cap - 1);
    memcpy(dst, src, len);
    dst[len] = '\0';
    if (out_len != NULL) { *out_len = len; }
}

/**
 * @brief collect current phone call state from app and update the global state.
 */
static void phone_call_collect_state(void)
{
    s_phone_call.call_state = phone_call_map_call_status(app_hfp_get_call_status());
    s_phone_call.volume = app_audio_get_volume();

    phone_call_copy_str(s_phone_call.number, PHONE_CALL_NUMBER_MAX_LEN,
                        &s_phone_call.number_len, app_hfp_get_current_call_number());
    phone_call_copy_str(s_phone_call.caller_name, PHONE_CALL_NAME_MAX_LEN,
                        &s_phone_call.caller_name_len, app_pbap_get_caller_name());

    APP_PRINT_INFO4("[phone_call] collect state: call_state %d, volume %d, number_len %d, caller_name_len %d",
                    s_phone_call.call_state, s_phone_call.volume,
                    s_phone_call.number_len, s_phone_call.caller_name_len);
}

/**
 * @brief publish current phone call state to GUI.
 *        Publishes a unified T_PHONE_CALL_STATE payload on every legacy GUI topic
 *        that is still subscribed by HoneyGUI-auto-generated UI code, so each
 *        subscribed widget receives the full snapshot regardless of which topic
 *        triggered its callback.
 */
static void phone_call_publish_state_app_to_gui(void)
{
    gui_msg_publish(GUI_TOPIC_PHONE_NUMBER,    &s_phone_call, sizeof(s_phone_call));
    gui_msg_publish(GUI_TOPIC_PHONE_CALLER_ID, &s_phone_call, sizeof(s_phone_call));
}

/**
 * @brief handle async events from GUI (cmd/req): dial / answer / end / mute / vol / req_state.
 *
 * @param[in] event_data The event data from event bus.
 * @return int32_t The result of handling the event.
 */
static int32_t app_phone_call_async_event_callback(T_EVENT_BUS_EVENT_DATA *event_data)
{
    const char *topic = event_data->topic;

    APP_PRINT_TRACE1("[phone_call] received async event topic: %s", TRACE_STRING(topic));

    if (strcmp(topic, EVENT_BUS_TOPIC_PHONE_CALL_REQ_STATE) == 0)
    {
        phone_call_collect_state();
        phone_call_publish_state_app_to_gui();
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_PHONE_CALL_CMD_DIAL) == 0)
    {
        if (event_data->data != NULL &&
            event_data->data_len == sizeof(T_PHONE_CALL_DIAL_DATA))
        {
            const T_PHONE_CALL_DIAL_DATA *dial = event_data->data;
            uint8_t len = dial->len;
            if (len >= PHONE_CALL_NUMBER_MAX_LEN)
            {
                len = PHONE_CALL_NUMBER_MAX_LEN - 1;
            }
            app_hfp_set_dial_number(dial->number, len);
        }
        app_mmi_handle_action(MMI_HF_OUTGOING_CALL);
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_PHONE_CALL_CMD_ANSWER) == 0)
    {
        app_mmi_handle_action(MMI_HF_ANSWER_CALL);
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_PHONE_CALL_CMD_END) == 0)
    {
        app_mmi_handle_action(MMI_HF_END_ACTIVE_CALL);
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_PHONE_CALL_CMD_MUTE) == 0)
    {
        app_mmi_handle_action(MMI_DEV_MIC_MUTE);
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_PHONE_CALL_CMD_UNMUTE) == 0)
    {
        app_mmi_handle_action(MMI_DEV_MIC_UNMUTE);
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_PHONE_CALL_CMD_VOL_UP) == 0)
    {
        app_mmi_handle_action(MMI_DEV_SPK_VOL_UP);
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_PHONE_CALL_CMD_VOL_DOWN) == 0)
    {
        app_mmi_handle_action(MMI_DEV_SPK_VOL_DOWN);
    }

    return 0;
}

/**
 * @brief handle phone-related events from app (hfp / hfp_audio / pbap),
 *        update local state and publish a unified snapshot to GUI.
 *
 * @param[in] event_data The event data from event bus.
 * @return int32_t The result of handling the event.
 */
static int32_t app_phone_call_event_callback(T_EVENT_BUS_EVENT_DATA *event_data)
{
    const char *topic = event_data->topic;

    if (strcmp(topic, EVENT_BUS_TOPIC_HFP_INCOMING) == 0)
    {
        s_phone_call.call_state = PHONE_CALL_STATE_INCOMING;
        phone_call_copy_str(s_phone_call.number, PHONE_CALL_NUMBER_MAX_LEN,
                            &s_phone_call.number_len, (const char *)event_data->data);
        /* caller_name comes via PBAP_CALLER_ID later; clear for now */
        s_phone_call.caller_name[0] = '\0';
        s_phone_call.caller_name_len = 0;
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_HFP_ANSWERED) == 0)
    {
        s_phone_call.call_state = PHONE_CALL_STATE_ACTIVE;
        if (event_data->data != NULL && event_data->data_len > 0)
        {
            phone_call_copy_str(s_phone_call.number, PHONE_CALL_NUMBER_MAX_LEN,
                                &s_phone_call.number_len, (const char *)event_data->data);
        }
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_HFP_ENDED) == 0)
    {
        s_phone_call.call_state = PHONE_CALL_STATE_ENDED;
        s_phone_call.number[0] = '\0';
        s_phone_call.number_len = 0;
        s_phone_call.caller_name[0] = '\0';
        s_phone_call.caller_name_len = 0;
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_HFP_AUDIO_VOLUME) == 0)
    {
        if (event_data->data != NULL && event_data->data_len == sizeof(uint8_t))
        {
            s_phone_call.volume = *(uint8_t *)event_data->data;
        }
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_PBAP_CALLER_ID) == 0)
    {
        phone_call_copy_str(s_phone_call.caller_name, PHONE_CALL_NAME_MAX_LEN,
                            &s_phone_call.caller_name_len, (const char *)event_data->data);
    }
    else
    {
        APP_PRINT_INFO1("[phone_call] received unknown event topic: %s", TRACE_STRING(topic));
        return EVENT_BUS_OK;
    }

    phone_call_publish_state_app_to_gui();
    return EVENT_BUS_OK;
}

/*============================================================================*
 *                           Public Functions
 *============================================================================*/

bool phone_call_gui_to_app(const char *topic, void *data, uint32_t size)
{
    return event_bus_publish(topic, data, size) == EVENT_BUS_OK;
}

void bridge_phone_call_init(void)
{
    /* Register the bridge's own command/req namespace. */
    event_bus_topic_register(EVENT_BUS_TOPIC_PHONE_CALL_ALL_TOPIC);

    /* Register HFP / HFP_AUDIO / PBAP namespaces here so the bridge's own
     * subscribe() calls below succeed even when the owning modules forgot to
     * register them. event_bus_topic_register is idempotent, so this is safe
     * to keep alongside any future per-owner registration. */
    event_bus_topic_register(EVENT_BUS_TOPIC_HFP_ALL_TOPIC);
    event_bus_topic_register(EVENT_BUS_TOPIC_HFP_AUDIO_ALL_TOPIC);
    event_bus_topic_register(EVENT_BUS_TOPIC_PBAP_ALL_TOPIC);

    /* GUI -> App: single async subscription covering both cmd and req sub-namespaces */
    event_bus_subscribe_async(&s_phone_call_async_handle,
                              EVENT_BUS_TOPIC_PHONE_CALL_ALL_TOPIC,
                              event_bus_async_send_to_apptask,
                              NULL,
                              app_phone_call_async_event_callback);

    /* App -> GUI: subscribe to HFP / HFP_AUDIO / PBAP events */
    event_bus_subscribe(&s_hfp_handle,
                        EVENT_BUS_TOPIC_HFP_ALL_TOPIC,
                        app_phone_call_event_callback);

    event_bus_subscribe(&s_hfp_audio_handle,
                        EVENT_BUS_TOPIC_HFP_AUDIO_ALL_TOPIC,
                        app_phone_call_event_callback);

    event_bus_subscribe(&s_pbap_handle,
                        EVENT_BUS_TOPIC_PBAP_ALL_TOPIC,
                        app_phone_call_event_callback);

    APP_PRINT_INFO0("[phone_call] initialized");
}
