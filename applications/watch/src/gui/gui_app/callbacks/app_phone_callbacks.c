/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_phone_callbacks.h"
#include "../ui/app_phone_ui.h"
#include "../user/app_phone_user.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// Time string global variables (defined in UI file)
extern char dialer_time_label_time_str[10];
extern char calling_time_label_time_str[10];
extern char incoming_time_label_time_str[10];

// Timer animation counters
uint16_t calling_number_label_timer_cnt = 0;
uint16_t call_timer_label_timer_cnt = 0;
uint16_t incoming_ring_animation_img_timer_cnt = 0;

// Event callback function implementations

void app_phoneDialerView_key_0_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    // Check key name
    if (strcmp(e->indev_name, "Home") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "SmartWatchTemplateMainView",
                               SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
    }
    else if (strcmp(e->indev_name, "Menu") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "app_menu_view", SWITCH_OUT_ANIMATION_FADE,
                               SWITCH_IN_ANIMATION_FADE);
    }
}

void win_10_clicked_0_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_menu_view", SWITCH_OUT_TO_LEFT_USE_TRANSLATION,
                           SWITCH_IN_FROM_RIGHT_USE_TRANSLATION);
}

void dial_key_1_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    dial_key_1_cb(obj, e);
}

void dial_key_2_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    dial_key_2_cb(obj, e);
}

void dial_key_3_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    dial_key_3_cb(obj, e);
}

void dial_key_4_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    dial_key_4_cb(obj, e);
}

void dial_key_5_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    dial_key_5_cb(obj, e);
}

void dial_key_6_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    dial_key_6_cb(obj, e);
}

void dial_key_7_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    dial_key_7_cb(obj, e);
}

void dial_key_8_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    dial_key_8_cb(obj, e);
}

void dial_key_9_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    dial_key_9_cb(obj, e);
}

void dial_key_star_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    dial_key_star_cb(obj, e);
}

void dial_key_0_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    dial_key_0_cb(obj, e);
}

void dial_key_hash_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    dial_key_hash_cb(obj, e);
}

void delete_btn_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    delete_key_pressed(obj, e);
}

void call_btn_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_phoneCallingView", SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
    phone_outgoing_call_cb(obj, e);
}

void simulate_incoming_btn_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_phoneIncomingView", SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
}

void phone_call_mute_btn_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    mute_toggle_cb(obj, e);
}

void hangup_btn_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_phoneDialerView", SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
    phone_end_call_cb(obj, e);
}

void volume_down_btn_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    volume_down_cb(obj, e);
}

void volume_up_btn_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    volume_up_cb(obj, e);
}

void decline_btn_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_phoneDialerView", SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
    phone_end_call_cb(obj, e);
}

void incoming_btn_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_phoneCallingView", SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
    phone_answer_call_cb(obj, e);
}

void calling_number_label_msg_cb_0(gui_obj_t *obj, const char *topic, void *data, uint16_t len)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(topic);
    GUI_UNUSED(data);
    GUI_UNUSED(len);
    phone_update_calling_label(obj, topic, data, len);
}

void calling_number_label_msg_cb_1(gui_obj_t *obj, const char *topic, void *data, uint16_t len)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(topic);
    GUI_UNUSED(data);
    GUI_UNUSED(len);
    phone_update_calling_label(obj, topic, data, len);
}

void incoming_name_label_msg_cb_0(gui_obj_t *obj, const char *topic, void *data, uint16_t len)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(topic);
    GUI_UNUSED(data);
    GUI_UNUSED(len);
    phone_update_incoming_caller_id(obj, topic, data, len);
}

void incoming_number_label_msg_cb_0(gui_obj_t *obj, const char *topic, void *data, uint16_t len)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(topic);
    GUI_UNUSED(data);
    GUI_UNUSED(len);
    phone_update_incoming_number(obj, topic, data, len);
}

void dialer_time_label_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(dialer_time_label_time_str, sizeof(dialer_time_label_time_str), "%02d:%02d", t->tm_hour,
             t->tm_min);

    gui_text_content_set((gui_text_t *)dialer_time_label, dialer_time_label_time_str,
                         strlen(dialer_time_label_time_str));
}

void calling_time_label_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(calling_time_label_time_str, sizeof(calling_time_label_time_str), "%02d:%02d", t->tm_hour,
             t->tm_min);

    gui_text_content_set((gui_text_t *)calling_time_label, calling_time_label_time_str,
                         strlen(calling_time_label_time_str));
}

void incoming_time_label_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(incoming_time_label_time_str, sizeof(incoming_time_label_time_str), "%02d:%02d",
             t->tm_hour, t->tm_min);

    gui_text_content_set((gui_text_t *)incoming_time_label, incoming_time_label_time_str,
                         strlen(incoming_time_label_time_str));
}

// Preset timer callback functions

/**
 * Animation 1
 * Component: calling_number_label
 */
void calling_number_label_timer_0_cb(void *obj)
{
    GUI_UNUSED(obj);
    // Call the implementation function in protected area (if exists)
    // Define calling_number_label_timer_0_cb_impl() in custom_functions protected area for custom logic
#ifdef __cplusplus
    extern "C" {
#endif
    extern void calling_number_label_timer_0_cb_impl(void) __attribute__((weak));
#ifdef __cplusplus
}
#endif

if (calling_number_label_timer_0_cb_impl)
{
    calling_number_label_timer_0_cb_impl();
}
else
{
    // TODO: Implement timer callback logic
    // Or define calling_number_label_timer_0_cb_impl() in custom_functions protected area
}
}

/**
 * call_timer
 * Component: call_timer_label
 */
void call_timer_tick(void *obj)
{
    GUI_UNUSED(obj);
    // Call the implementation function in protected area (if exists)
    // Define call_timer_tick_impl() in custom_functions protected area for custom logic
#ifdef __cplusplus
    extern "C" {
#endif
    extern void call_timer_tick_impl(void) __attribute__((weak));
#ifdef __cplusplus
}
#endif

if (call_timer_tick_impl)
{
    call_timer_tick_impl();
}
else
{
    // TODO: Implement timer callback logic
    // Or define call_timer_tick_impl() in custom_functions protected area
}
}

/**
 * incoming_ring
 * Component: incoming_ring_animation_img
 */
void incoming_ring_timer_cb(void *obj)
{
    GUI_UNUSED(obj);
    // Call the implementation function in protected area (if exists)
    // Define incoming_ring_timer_cb_impl() in custom_functions protected area for custom logic
#ifdef __cplusplus
    extern "C" {
#endif
    extern void incoming_ring_timer_cb_impl(void) __attribute__((weak));
#ifdef __cplusplus
}
#endif

if (incoming_ring_timer_cb_impl)
{
    incoming_ring_timer_cb_impl();
}
else
{
    // TODO: Implement timer callback logic
    // Or define incoming_ring_timer_cb_impl() in custom_functions protected area
}
}

/* @protected start custom_functions */
// Custom functions
/* @protected end custom_functions */
