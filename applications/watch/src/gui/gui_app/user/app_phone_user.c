/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_phone_user.h"
#include "../ui/app_phone_ui.h"
#include "gui_img.h"
#include "gui_text.h"
#include "gui_view.h"
#include "gui_server.h"
#include "gui_api.h"
#include "gui_fb.h"
#include "gui_listener.h"
#ifndef _HONEYGUI_SIMULATOR_
#include "bridge_phone_call.h"
#else
/* Simulator build: bridge_phone_call.* is part of the app side and not
 * compiled. Mirror just enough of its public surface (types, macros, and a
 * no-op phone_call_gui_to_app) so the GUI sources keep compiling unchanged. */
#define PHONE_CALL_NUMBER_MAX_LEN              32
#define PHONE_CALL_NAME_MAX_LEN                32
#define PHONE_CALL_VOLUME_MAX                  15

#define GUI_TOPIC_PHONE_NUMBER                 "phone/number"
#define GUI_TOPIC_PHONE_CALLER_ID              "phone/caller_id"

#define EVENT_BUS_TOPIC_PHONE_CALL_REQ_STATE   "phone_call/req/state"
#define EVENT_BUS_TOPIC_PHONE_CALL_CMD_DIAL    "phone_call/cmd/dial"
#define EVENT_BUS_TOPIC_PHONE_CALL_CMD_ANSWER  "phone_call/cmd/answer"
#define EVENT_BUS_TOPIC_PHONE_CALL_CMD_END     "phone_call/cmd/end"
#define EVENT_BUS_TOPIC_PHONE_CALL_CMD_MUTE    "phone_call/cmd/mute"
#define EVENT_BUS_TOPIC_PHONE_CALL_CMD_UNMUTE  "phone_call/cmd/unmute"
#define EVENT_BUS_TOPIC_PHONE_CALL_CMD_VOL_UP  "phone_call/cmd/vol_up"
#define EVENT_BUS_TOPIC_PHONE_CALL_CMD_VOL_DOWN "phone_call/cmd/vol_down"

typedef enum
{
    PHONE_CALL_STATE_IDLE     = 0x00,
    PHONE_CALL_STATE_INCOMING = 0x01,
    PHONE_CALL_STATE_OUTGOING = 0x02,
    PHONE_CALL_STATE_ACTIVE   = 0x03,
    PHONE_CALL_STATE_ENDED    = 0x04,
} T_PHONE_CALL_STATE_E;

typedef struct
{
    T_PHONE_CALL_STATE_E call_state;
    char    number[PHONE_CALL_NUMBER_MAX_LEN];
    uint8_t number_len;
    char    caller_name[PHONE_CALL_NAME_MAX_LEN];
    uint8_t caller_name_len;
    uint8_t volume;
} T_PHONE_CALL_STATE;

typedef struct
{
    char    number[PHONE_CALL_NUMBER_MAX_LEN];
    uint8_t len;
} T_PHONE_CALL_DIAL_DATA;

static inline bool phone_call_gui_to_app(const char *topic, void *data, uint32_t size)
{
    (void)topic; (void)data; (void)size;
    return true;
}
#endif
#include <stdio.h>
#include <string.h>

#define PHONE_MAX_DIGITS 12
#define INCOMING_FRAME_COUNT 30

/* GUI-local input buffer for the dialer. Not part of bridged app state. */
static char phone_number[PHONE_MAX_DIGITS + 1] = "";
static int  phone_number_len = 0;

/* GUI-local view text buffers */
static char call_timer_text[16] = "00:00";
static char phone_volume_text[4] = "5";

/* GUI-local UX state.
 * elapsed_seconds is driven by call_timer_tick_impl(); it is not provided by
 * the bridge (the bridge only exposes call_state transitions).
 * phone_muted is tracked locally until the audio layer publishes a mute event.
 * phone_volume mirrors s_call_state.volume but allows optimistic UI updates
 * before the app-side acknowledgement comes back through the state topic. */
static bool phone_muted = false;
static int  phone_volume = 5;
static int  elapsed_seconds = 0;
static int  incoming_frame_index = 0;

/* Cached snapshot of the unified phone-call state, refreshed every time the
 * bridge publishes on GUI_TOPIC_PHONE_NUMBER / GUI_TOPIC_PHONE_CALLER_ID. */
static T_PHONE_CALL_STATE s_call_state;

/*===========================================================================*
 *                          Forward declarations
 *===========================================================================*/
static void phone_call_switch_to_incoming_view(void *param);
static void phone_call_switch_to_calling_view(void *param);
static void phone_call_switch_to_main_view(void *param);

/*===========================================================================*
 *                          Local helpers
 *===========================================================================*/
static bool is_current_view(const char *view_name)
{
    gui_view_t *current_view = gui_view_get_current();
    if (current_view == NULL || current_view->base.name == NULL)
    {
        return false;
    }
    return strcmp(current_view->base.name, view_name) == 0;
}

static void set_text_content(gui_text_t *text_obj, const char *text)
{
    if (!text_obj || !text)
    {
        return;
    }
    gui_text_content_set(text_obj, (char *)text, strlen(text));
}

static void clear_dialed_number(void)
{
    phone_number[0] = '\0';
    phone_number_len = 0;
}

/* Pick the best label to show for the active/incoming call:
 *   caller_name (PBAP) > number (HFP) > local dial buffer > "Unknown". */
static const char *pick_call_display_label(void)
{
    if (s_call_state.caller_name_len > 0)
    {
        return s_call_state.caller_name;
    }
    if (s_call_state.number_len > 0)
    {
        return s_call_state.number;
    }
    if (phone_number_len > 0)
    {
        return phone_number;
    }
    return "Unknown";
}

/*===========================================================================*
 *                          Display update helpers
 *===========================================================================*/
static void update_dialer_number_display(void)
{
    if (!number_display_label)
    {
        return;
    }
    const char *display_text = phone_number_len > 0 ? phone_number : " ";
    set_text_content((gui_text_t *)number_display_label, display_text);
}

static void update_calling_number_display(void)
{
    if (!calling_number_label)
    {
        return;
    }
    set_text_content((gui_text_t *)calling_number_label, pick_call_display_label());
}

static void update_call_timer_display(void)
{
    if (!call_timer_label)
    {
        return;
    }
    snprintf(call_timer_text, sizeof(call_timer_text), "%02d:%02d",
             elapsed_seconds / 60, elapsed_seconds % 60);
    set_text_content((gui_text_t *)call_timer_label, call_timer_text);
}

static void update_mute_button_display(void)
{
    if (!phone_call_mute_btn)
    {
        return;
    }
    gui_img_set_src((gui_img_t *)phone_call_mute_btn,
                    (const uint8_t *)(phone_muted ? "/app_phone/mute_btn_active.bin"
                                      : "/app_phone/mute_btn_normal.bin"),
                    IMG_SRC_FILESYS);
}

static void update_volume_display(void)
{
    if (!volume_value_label)
    {
        return;
    }
    snprintf(phone_volume_text, sizeof(phone_volume_text), "%d", phone_volume);
    set_text_content((gui_text_t *)volume_value_label, phone_volume_text);
}

static void update_incoming_ring_frame(void)
{
    char frame_path[80];
    if (!incoming_ring_animation_img)
    {
        return;
    }
    snprintf(frame_path, sizeof(frame_path),
             "/app_phone/incoming_ring_animation/frame_%02d.bin",
             incoming_frame_index);
    gui_img_set_src((gui_img_t *)incoming_ring_animation_img, (const uint8_t *)frame_path,
                    IMG_SRC_FILESYS);
}

static void sync_calling_view(void)
{
    update_calling_number_display();
    update_call_timer_display();
    update_mute_button_display();
    update_volume_display();
}

static void append_phone_digit(char key)
{
    if (phone_number_len >= PHONE_MAX_DIGITS)
    {
        return;
    }
    phone_number[phone_number_len++] = key;
    phone_number[phone_number_len] = '\0';
    update_dialer_number_display();
    gui_fb_change();
}

/*===========================================================================*
 *                          State-driven view switching
 *===========================================================================*/
/* React to call_state transitions: drive view switches and screen wakeup
 * entirely from the GUI side, without any bridge -> user reverse calls. */
static void phone_call_handle_state_transition(T_PHONE_CALL_STATE_E prev,
                                               T_PHONE_CALL_STATE_E next)
{
    if (prev == next)
    {
        return;
    }

    gui_msg_t msg;

    if (next == PHONE_CALL_STATE_INCOMING)
    {
        /* Wake the screen first so the incoming view is visible. */
        msg.event = GUI_EVENT_DISPLAY_ON;
        gui_send_msg_to_server(&msg);

        msg.event = GUI_EVENT_USER_DEFINE;
        msg.cb = phone_call_switch_to_incoming_view;
        gui_send_msg_to_server(&msg);
    }
    else if (next == PHONE_CALL_STATE_OUTGOING)
    {
        /* Outgoing call in progress (initiated either from watch dialer or
         * directly from the paired phone). Show the calling view with timer
         * at 00:00; the timer only starts ticking once the call goes ACTIVE. */
        elapsed_seconds = 0;

        msg.event = GUI_EVENT_DISPLAY_ON;
        gui_send_msg_to_server(&msg);

        msg.event = GUI_EVENT_USER_DEFINE;
        msg.cb = phone_call_switch_to_calling_view;
        gui_send_msg_to_server(&msg);
    }
    else if (next == PHONE_CALL_STATE_ACTIVE)
    {
        clear_dialed_number();
        elapsed_seconds = 0;

        msg.event = GUI_EVENT_DISPLAY_ON;
        gui_send_msg_to_server(&msg);

        msg.event = GUI_EVENT_USER_DEFINE;
        msg.cb = phone_call_switch_to_calling_view;
        gui_send_msg_to_server(&msg);
    }
    else if (next == PHONE_CALL_STATE_ENDED || next == PHONE_CALL_STATE_IDLE)
    {
        clear_dialed_number();
        elapsed_seconds = 0;
        snprintf(call_timer_text, sizeof(call_timer_text), "%02d:%02d", 0, 0);

        msg.event = GUI_EVENT_USER_DEFINE;
        msg.cb = phone_call_switch_to_main_view;
        gui_send_msg_to_server(&msg);
    }
}

/* Apply a unified state snapshot from the bridge and react to transitions. */
static void phone_call_apply_state(const void *data, uint16_t len)
{
    if (data == NULL || len != sizeof(T_PHONE_CALL_STATE))
    {
        return;
    }
    const T_PHONE_CALL_STATE *state = (const T_PHONE_CALL_STATE *)data;
    T_PHONE_CALL_STATE_E prev = s_call_state.call_state;

    s_call_state = *state;
    phone_volume = state->volume;

    phone_call_handle_state_transition(prev, state->call_state);
}

/*===========================================================================*
 *                          View-init callbacks
 *===========================================================================*/
void incoming_view_init_cb_impl(void)
{
    clear_dialed_number();

    /* Pull a fresh snapshot in case we missed earlier publishes. */
    phone_call_gui_to_app(EVENT_BUS_TOPIC_PHONE_CALL_REQ_STATE, NULL, 0);

    if (incoming_name_label)
    {
        const char *name = (s_call_state.caller_name_len > 0) ? s_call_state.caller_name
                           : "Unknown";
        set_text_content((gui_text_t *)incoming_name_label, name);
    }
    if (incoming_number_label)
    {
        const char *number = (s_call_state.number_len > 0) ? s_call_state.number : "";
        set_text_content((gui_text_t *)incoming_number_label, number);
    }
    incoming_frame_index = 0;
    update_incoming_ring_frame();
    gui_fb_change();
}

void calling_view_init_cb_impl(void)
{
    phone_call_gui_to_app(EVENT_BUS_TOPIC_PHONE_CALL_REQ_STATE, NULL, 0);
    sync_calling_view();
    gui_fb_change();
}

/*===========================================================================*
 *                          Timer callbacks
 *===========================================================================*/
void calling_number_label_timer_0_cb_impl(void)
{
    update_calling_number_display();
    gui_fb_change();
}

void call_timer_tick_impl(void)
{
    if (!is_current_view("app_phoneCallingView"))
    {
        return;
    }
    /* Only count duration once the call is connected; during OUTGOING the
     * calling view is shown but the timer should stay at 00:00. */
    if (s_call_state.call_state != PHONE_CALL_STATE_ACTIVE)
    {
        return;
    }
    elapsed_seconds++;
    update_call_timer_display();
    gui_fb_change();
}

void incoming_ring_timer_cb_impl(void)
{
    if (!is_current_view("app_phoneIncomingView"))
    {
        return;
    }
    incoming_frame_index = (incoming_frame_index + 1) % INCOMING_FRAME_COUNT;
    update_incoming_ring_frame();
    gui_fb_change();
}

/*===========================================================================*
 *                          Dial-key callbacks
 *===========================================================================*/
void dial_key_0_cb(void *obj, gui_event_t *e) { GUI_UNUSED(obj); GUI_UNUSED(e); append_phone_digit('0'); }
void dial_key_1_cb(void *obj, gui_event_t *e) { GUI_UNUSED(obj); GUI_UNUSED(e); append_phone_digit('1'); }
void dial_key_2_cb(void *obj, gui_event_t *e) { GUI_UNUSED(obj); GUI_UNUSED(e); append_phone_digit('2'); }
void dial_key_3_cb(void *obj, gui_event_t *e) { GUI_UNUSED(obj); GUI_UNUSED(e); append_phone_digit('3'); }
void dial_key_4_cb(void *obj, gui_event_t *e) { GUI_UNUSED(obj); GUI_UNUSED(e); append_phone_digit('4'); }
void dial_key_5_cb(void *obj, gui_event_t *e) { GUI_UNUSED(obj); GUI_UNUSED(e); append_phone_digit('5'); }
void dial_key_6_cb(void *obj, gui_event_t *e) { GUI_UNUSED(obj); GUI_UNUSED(e); append_phone_digit('6'); }
void dial_key_7_cb(void *obj, gui_event_t *e) { GUI_UNUSED(obj); GUI_UNUSED(e); append_phone_digit('7'); }
void dial_key_8_cb(void *obj, gui_event_t *e) { GUI_UNUSED(obj); GUI_UNUSED(e); append_phone_digit('8'); }
void dial_key_9_cb(void *obj, gui_event_t *e) { GUI_UNUSED(obj); GUI_UNUSED(e); append_phone_digit('9'); }
void dial_key_star_cb(void *obj, gui_event_t *e) { GUI_UNUSED(obj); GUI_UNUSED(e); append_phone_digit('*'); }
void dial_key_hash_cb(void *obj, gui_event_t *e) { GUI_UNUSED(obj); GUI_UNUSED(e); append_phone_digit('#'); }

void delete_key_pressed(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    if (phone_number_len <= 0)
    {
        return;
    }
    phone_number[--phone_number_len] = '\0';
    update_dialer_number_display();
    gui_fb_change();
}

/*===========================================================================*
 *                          Audio control callbacks
 *===========================================================================*/
void mute_toggle_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    phone_muted = !phone_muted;
    const char *topic = phone_muted ? EVENT_BUS_TOPIC_PHONE_CALL_CMD_MUTE
                        : EVENT_BUS_TOPIC_PHONE_CALL_CMD_UNMUTE;
    phone_call_gui_to_app(topic, NULL, 0);

    update_mute_button_display();
    gui_fb_change();
}

void volume_up_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    /* Optimistic local bump; overwritten by the next state publish. */
    if (phone_volume < PHONE_CALL_VOLUME_MAX)
    {
        phone_volume++;
    }
    update_volume_display();
    gui_fb_change();

    phone_call_gui_to_app(EVENT_BUS_TOPIC_PHONE_CALL_CMD_VOL_UP, NULL, 0);
}

void volume_down_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    if (phone_volume > 0)
    {
        phone_volume--;
    }
    update_volume_display();
    gui_fb_change();

    phone_call_gui_to_app(EVENT_BUS_TOPIC_PHONE_CALL_CMD_VOL_DOWN, NULL, 0);
}

/*===========================================================================*
 *                          State-topic update callbacks
 *
 * These three are wired up from auto-generated app_phone_callbacks.c whenever
 * the bridge publishes on GUI_TOPIC_PHONE_NUMBER / GUI_TOPIC_PHONE_CALLER_ID.
 * Each one applies the unified payload and refreshes its own UI element.
 *===========================================================================*/
void phone_update_incoming_caller_id(gui_obj_t *obj, const char *topic, void *data, uint16_t len)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(topic);

    phone_call_apply_state(data, len);

    if (incoming_name_label)
    {
        const char *name = (s_call_state.caller_name_len > 0) ? s_call_state.caller_name
                           : "Unknown";
        set_text_content((gui_text_t *)incoming_name_label, name);
    }
    update_calling_number_display();
    gui_fb_change();
}

void phone_update_incoming_number(gui_obj_t *obj, const char *topic, void *data, uint16_t len)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(topic);

    phone_call_apply_state(data, len);

    if (incoming_number_label)
    {
        const char *number = (s_call_state.number_len > 0) ? s_call_state.number : "";
        set_text_content((gui_text_t *)incoming_number_label, number);
    }
    update_calling_number_display();
    gui_fb_change();
}

void phone_update_calling_label(gui_obj_t *obj, const char *topic, void *data, uint16_t len)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(topic);

    phone_call_apply_state(data, len);

    if (is_current_view("app_phoneCallingView"))
    {
        sync_calling_view();
    }
    gui_fb_change();
}

/*===========================================================================*
 *                          Call-control callbacks
 *===========================================================================*/
void phone_outgoing_call_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    if (phone_number_len > 0)
    {
        T_PHONE_CALL_DIAL_DATA dial;
        memset(&dial, 0, sizeof(dial));

        uint8_t copy_len = (uint8_t)phone_number_len;
        if (copy_len >= PHONE_CALL_NUMBER_MAX_LEN)
        {
            copy_len = PHONE_CALL_NUMBER_MAX_LEN - 1;
        }
        memcpy(dial.number, phone_number, copy_len);
        dial.number[copy_len] = '\0';
        dial.len = copy_len;

        phone_call_gui_to_app(EVENT_BUS_TOPIC_PHONE_CALL_CMD_DIAL,
                              &dial, sizeof(dial));
    }
    gui_fb_change();
}

void phone_end_call_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    phone_call_gui_to_app(EVENT_BUS_TOPIC_PHONE_CALL_CMD_END, NULL, 0);
    gui_fb_change();
}

void phone_answer_call_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    phone_call_gui_to_app(EVENT_BUS_TOPIC_PHONE_CALL_CMD_ANSWER, NULL, 0);
    gui_fb_change();
}

/*===========================================================================*
 *                          Public helpers
 *===========================================================================*/
const char *get_dialed_number(void)
{
    return phone_number;
}

void phone_switch_to_dialer_view(void)
{
    clear_dialed_number();
    update_dialer_number_display();
    gui_fb_change();
}

/*===========================================================================*
 *                          Root-level state subscription
 *
 * Per-widget gui_msg_subscribe() in app_phone_ui.c only takes effect after the
 * corresponding view (incoming / calling) is entered, so an HFP_INCOMING that
 * arrives while the user is on the watchface would otherwise have no GUI-side
 * subscriber. We attach a permanent subscription to the GUI root object — it
 * is alive for the entire app lifetime and drives view switches via the same
 * phone_call_apply_state() path.
 *===========================================================================*/
static void phone_call_root_state_cb(gui_obj_t *obj, const char *topic,
                                     void *data, uint16_t len)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(topic);

    /* Only apply the snapshot — the root has no widgets to refresh; per-view
     * widgets refresh themselves via their own subscriptions once the view is
     * entered (and a REQ_STATE inside *_view_init_cb_impl pulls fresh data). */
    phone_call_apply_state(data, len);
}

void app_phone_user_init(void)
{
    /* Subscribing to GUI_TOPIC_PHONE_NUMBER alone is enough: the bridge
     * publishes the same T_PHONE_CALL_STATE payload on both phone topics
     * for every state change, so we only need one driver for transitions. */
    gui_log("[phone_user] init: subscribing root to %s", GUI_TOPIC_PHONE_NUMBER);
    gui_msg_subscribe(gui_obj_get_root(), GUI_TOPIC_PHONE_NUMBER,
                      phone_call_root_state_cb);
}

/*===========================================================================*
 *                          View-switch trampolines
 *===========================================================================*/
static void phone_call_switch_to_incoming_view(void *param)
{
    GUI_UNUSED(param);
    gui_view_switch_direct(gui_view_get_current(), "app_phoneIncomingView",
                           SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
    /* The auto-generated switch_in only seeds widgets with their HML default
     * strings ("John Doe", "+1 (555) ...") and registers per-widget topic
     * subscribers. Run the init impl now so the freshly created widgets are
     * populated from the cached T_PHONE_CALL_STATE that already drove this
     * transition; otherwise the user would see defaults until the next
     * publish from the bridge. */
    incoming_view_init_cb_impl();
}

static void phone_call_switch_to_calling_view(void *param)
{
    GUI_UNUSED(param);
    gui_view_switch_direct(gui_view_get_current(), "app_phoneCallingView",
                           SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
    calling_view_init_cb_impl();
}

static void phone_call_switch_to_main_view(void *param)
{
    GUI_UNUSED(param);
    gui_view_switch_direct(gui_view_get_current(), "SmartWatchTemplateMainView",
                           SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
}
