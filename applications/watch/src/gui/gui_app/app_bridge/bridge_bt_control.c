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
#include "bridge_bt_control.h"
#include "gui_listener.h"
#include "app_bt_policy_api.h"
#include "app_gap.h"
#include "app_task.h"
#include "app_bond.h"
#include "app_link_util.h"
#include "app_main.h"
#include "trace.h"

/*============================================================================*
 *                            Variables
 *============================================================================*/

static T_BT_CONTROL_STATE s_bt_control;
static T_EVENT_BUS_SUBSCRIBER_HANDLE s_bt_control_async_handle;
static T_EVENT_BUS_SUBSCRIBER_HANDLE s_bt_evt_handle;

/* Pending headphone swap target. Set when GUI requests SWAP_TO and another
 * earphone is currently active; cleared after the actual connect is fired.
 * All access happens on apptask (async cmd cb + bt event cb), so no lock. */
static uint8_t s_pending_swap_addr[6] = {0};
static bool    s_pending_swap_set     = false;

/*============================================================================*
 *                           Private Functions
 *============================================================================*/

/**
 * @brief Aggregate app_db.bond_device[] + app_find_br_link() into s_bt_control.
 *        Slot 0 is reserved for phone, slots 1..7 for headphones, matching the
 *        existing convention used by app_control_center_user.c.
 */
static void bt_control_collect_state(void)
{
    memset(&s_bt_control, 0, sizeof(s_bt_control));

    for (uint8_t i = 0; i < BT_CONTROL_MAX_BONDED; i++)
    {
        T_APP_BOND_DEVICE   *src = &app_db.bond_device[i];
        T_BT_CONTROL_DEVICE *dst = &s_bt_control.bonded[i];

        if (!src->exist_addr_flag)
        {
            dst->kind = BT_DEV_KIND_NONE;
            continue;
        }

        memcpy(dst->bd_addr, src->bd_addr, 6);
        memcpy(dst->device_name, src->device_name, sizeof(dst->device_name));
        dst->device_name_len = src->device_name_len;
        dst->kind            = (uint8_t)src->device_type;
        dst->exist           = 1;
        dst->connected       = (app_find_br_link(src->bd_addr) != NULL) ? 1 : 0;

        if (i == 0)
        {
            s_bt_control.phone_connected = dst->connected;
        }
        else
        {
            if (dst->connected) { s_bt_control.headphone_connected = 1; }
            s_bt_control.headphone_count++;
        }
    }
}

/**
 * @brief If a pending swap target is set and no other earphone is still
 *        connected, fire the actual connect command. Caller must hold a fresh
 *        s_bt_control snapshot. Apptask-only.
 */
static void try_flush_pending_swap(void)
{
    if (!s_pending_swap_set) { return; }

    /* If another earphone (other than the swap target) is still connected,
     * keep waiting for its disconnect. */
    for (uint8_t i = 1; i < BT_CONTROL_MAX_BONDED; i++)
    {
        const T_BT_CONTROL_DEVICE *b = &s_bt_control.bonded[i];
        if (!b->exist) { continue; }
        if (memcmp(b->bd_addr, s_pending_swap_addr, 6) == 0) { continue; }
        if (b->connected) { return; }
    }

    APP_PRINT_INFO0("[bt_control] flush pending swap -> connect_bredr");
    app_bt_policy_connect_bredr(s_pending_swap_addr);
    s_pending_swap_set = false;
    memset(s_pending_swap_addr, 0, 6);
}

/**
 * @brief Forward an App-side BT event to the matching legacy GUI topic.
 *        For connection events, the payload is replaced with a fresh
 *        T_BT_CONTROL_STATE snapshot so GUI widgets can render without
 *        touching app_db directly. Inquiry events are passed through.
 */
static int32_t app_bt_control_event_callback(T_EVENT_BUS_EVENT_DATA *event_data)
{
    const char *topic = event_data->topic;

    if (strcmp(topic, EVENT_BUS_TOPIC_BT_EVT_PHONE_CONN) == 0 ||
        strcmp(topic, EVENT_BUS_TOPIC_BT_EVT_PHONE_DISCONN) == 0 ||
        strcmp(topic, EVENT_BUS_TOPIC_BT_EVT_HEADPHONE_CONN) == 0 ||
        strcmp(topic, EVENT_BUS_TOPIC_BT_EVT_HEADPHONE_DISCONN) == 0)
    {
        bt_control_collect_state();

        const char *gui_topic =
            (strcmp(topic, EVENT_BUS_TOPIC_BT_EVT_PHONE_CONN) == 0)        ? GUI_TOPIC_BT_PHONE_CONN :
            (strcmp(topic, EVENT_BUS_TOPIC_BT_EVT_PHONE_DISCONN) == 0)     ? GUI_TOPIC_BT_PHONE_DISCONN :
            (strcmp(topic, EVENT_BUS_TOPIC_BT_EVT_HEADPHONE_CONN) == 0)    ? GUI_TOPIC_BT_HEADPHONE_CONN :
            GUI_TOPIC_BT_HEADPHONE_DISCONN;

        gui_msg_publish(gui_topic, &s_bt_control, sizeof(s_bt_control));

        /* If this disconn frees the slot for a pending swap, fire the connect
         * now. Snapshot was just refreshed; helper is a no-op when nothing is
         * pending. */
        if (strcmp(topic, EVENT_BUS_TOPIC_BT_EVT_HEADPHONE_DISCONN) == 0)
        {
            try_flush_pending_swap();
        }
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_BT_EVT_INQUIRY_RESULT) == 0)
    {
        gui_msg_publish(GUI_TOPIC_BT_INQUIRY_RESULT, event_data->data, event_data->data_len);
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_BT_EVT_INQUIRY_CMPL) == 0)
    {
        gui_msg_publish(GUI_TOPIC_BT_INQUIRY_CMPL, event_data->data, event_data->data_len);
    }
    else
    {
        APP_PRINT_INFO1("[bt_control] received unknown event topic: %s", TRACE_STRING(topic));
    }

    return EVENT_BUS_OK;
}

/**
 * @brief Handle GUI -> App BT control commands (req/state, cmd/...).
 *        Runs on the apptask thread (event_bus_async_send_to_apptask) so
 *        BT APIs can be invoked directly without a second IO_MSG hop.
 */
static int32_t app_bt_control_async_event_callback(T_EVENT_BUS_EVENT_DATA *event_data)
{
    const char *topic = event_data->topic;

    APP_PRINT_TRACE1("[bt_control] async cmd topic: %s", TRACE_STRING(topic));

    if (strcmp(topic, EVENT_BUS_TOPIC_BT_CONTROL_CMD_TOGGLE) == 0)
    {
        if (event_data->data == NULL || event_data->data_len < sizeof(T_BT_CONTROL_TOGGLE_DATA))
        {
            APP_PRINT_ERROR0("[bt_control] toggle: bad payload");
            return EVENT_BUS_OK;
        }
        T_BT_CONTROL_TOGGLE_DATA *p = (T_BT_CONTROL_TOGGLE_DATA *)event_data->data;
        if (p->enable == 0)
        {
            /* BT going off: any pending swap is moot; cancel before the
             * forthcoming bulk disconnects fire flush. */
            s_pending_swap_set = false;
            memset(s_pending_swap_addr, 0, 6);
        }
        app_bt_policy_set_enabled(p->enable != 0);
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_BT_CONTROL_CMD_INQUIRY_START) == 0)
    {
        app_bt_inquiry_start();
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_BT_CONTROL_CMD_INQUIRY_STOP) == 0)
    {
        app_bt_inquiry_stop();
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_BT_CONTROL_CMD_CONNECT_PHONE) == 0 ||
             strcmp(topic, EVENT_BUS_TOPIC_BT_CONTROL_CMD_CONNECT_BREDR) == 0 ||
             strcmp(topic, EVENT_BUS_TOPIC_BT_CONTROL_CMD_DISCONNECT) == 0)
    {
        if (event_data->data == NULL || event_data->data_len < sizeof(T_BT_CONTROL_ADDR_DATA))
        {
            APP_PRINT_ERROR1("[bt_control] %s: bad payload", TRACE_STRING(topic));
            return EVENT_BUS_OK;
        }
        T_BT_CONTROL_ADDR_DATA *p = (T_BT_CONTROL_ADDR_DATA *)event_data->data;

        if (strcmp(topic, EVENT_BUS_TOPIC_BT_CONTROL_CMD_CONNECT_PHONE) == 0)
        {
            app_bt_policy_connect_phone(p->bd_addr);
        }
        else if (strcmp(topic, EVENT_BUS_TOPIC_BT_CONTROL_CMD_CONNECT_BREDR) == 0)
        {
            app_bt_policy_connect_bredr(p->bd_addr);
        }
        else
        {
            app_bt_policy_disconnect_bredr(p->bd_addr);
        }
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_BT_CONTROL_CMD_SWAP_TO) == 0)
    {
        if (event_data->data == NULL || event_data->data_len < sizeof(T_BT_CONTROL_ADDR_DATA))
        {
            APP_PRINT_ERROR0("[bt_control] swap_to: bad payload");
            return EVENT_BUS_OK;
        }
        T_BT_CONTROL_ADDR_DATA *p = (T_BT_CONTROL_ADDR_DATA *)event_data->data;

        /* Detect whether another earphone is still connected. Refresh snapshot
         * first so the decision uses live state. */
        bt_control_collect_state();
        bool need_pending = false;
        for (uint8_t i = 1; i < BT_CONTROL_MAX_BONDED; i++)
        {
            const T_BT_CONTROL_DEVICE *b = &s_bt_control.bonded[i];
            if (!b->exist || !b->connected) { continue; }
            if (memcmp(b->bd_addr, p->bd_addr, 6) != 0) { need_pending = true; break; }
        }

        if (need_pending)
        {
            memcpy(s_pending_swap_addr, p->bd_addr, 6);
            s_pending_swap_set = true;
        }
        else
        {
            s_pending_swap_set = false;
            memset(s_pending_swap_addr, 0, 6);
        }

        /* connect_bredr disconnects the active one (if any) and otherwise
         * connects the target directly. */
        app_bt_policy_connect_bredr(p->bd_addr);
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_BT_CONTROL_CMD_REMOVE_BOND) == 0)
    {
        if (event_data->data == NULL || event_data->data_len < sizeof(T_BT_CONTROL_REMOVE_DATA))
        {
            APP_PRINT_ERROR0("[bt_control] remove_bond: bad payload");
            return EVENT_BUS_OK;
        }
        T_BT_CONTROL_REMOVE_DATA *p = (T_BT_CONTROL_REMOVE_DATA *)event_data->data;
        uint8_t removed_index = p->index;

        /* If the device being removed is the pending swap target, clear
         * pending so the forthcoming disconnect doesn't auto-reconnect a
         * bond record that no longer exists. */
        if (s_pending_swap_set &&
            removed_index < BT_CONTROL_MAX_BONDED &&
            s_bt_control.bonded[removed_index].exist &&
            memcmp(s_bt_control.bonded[removed_index].bd_addr,
                   s_pending_swap_addr, 6) == 0)
        {
            s_pending_swap_set = false;
            memset(s_pending_swap_addr, 0, 6);
        }

        app_bt_bond_remove_by_index(removed_index);

        /* Bond table changed - re-collect and republish so the list view
         * updates immediately. Without this, an unconnected device removal
         * produces no bt_evt event and the GUI list would stay stale until
         * the next connection event or REQ_STATE. */
        bt_control_collect_state();
        const char *gui_topic = (removed_index == 0) ? GUI_TOPIC_BT_PHONE_DISCONN
                                : GUI_TOPIC_BT_HEADPHONE_DISCONN;
        gui_msg_publish(gui_topic, &s_bt_control, sizeof(s_bt_control));
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_BT_CONTROL_REQ_STATE) == 0)
    {
        /* GUI requested a state refresh: re-collect and re-publish snapshot. */
        bt_control_collect_state();
        gui_msg_publish(GUI_TOPIC_BT_PHONE_CONN, &s_bt_control, sizeof(s_bt_control));
    }
    else
    {
        APP_PRINT_INFO1("[bt_control] async unknown topic: %s", TRACE_STRING(topic));
    }

    return EVENT_BUS_OK;
}

/*============================================================================*
 *                           Public Functions
 *============================================================================*/

bool bt_control_gui_to_app(const char *topic, void *data, uint32_t size)
{
    return event_bus_publish(topic, data, size) == EVENT_BUS_OK;
}

const T_BT_CONTROL_STATE *bridge_bt_control_get_state(void)
{
    return &s_bt_control;
}

void bridge_bt_control_init(void)
{
    /* Register the bridge's own command/req namespace. */
    event_bus_topic_register(EVENT_BUS_TOPIC_BT_CONTROL_ALL_TOPIC);

    /* App-side BT event namespace - register here so the bridge's own
     * subscribe() succeeds even if the owning modules (app_bt_policy /
     * app_gap) forgot to register it. event_bus_topic_register is idempotent.
     * Plan B: move this to the owning modules' init functions. */
    event_bus_topic_register(EVENT_BUS_TOPIC_BT_EVT_ALL_TOPIC);

    /* GUI -> App: single async subscription covering both cmd and req. */
    event_bus_subscribe_async(&s_bt_control_async_handle,
                              EVENT_BUS_TOPIC_BT_CONTROL_ALL_TOPIC,
                              event_bus_async_send_to_apptask,
                              NULL,
                              app_bt_control_async_event_callback);

    /* App -> GUI: subscribe to the bt_evt namespace. */
    event_bus_subscribe(&s_bt_evt_handle,
                        EVENT_BUS_TOPIC_BT_EVT_ALL_TOPIC,
                        app_bt_control_event_callback);

    /* Seed the cache from the current bond table so any GUI page that comes
     * up before the first connect/disconnect event still gets a meaningful
     * snapshot via bridge_bt_control_get_state(). */
    bt_control_collect_state();

    APP_PRINT_INFO0("[bt_control] initialized");
}
