/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_stopwatch_callbacks.h"
#include "../ui/app_stopwatch_ui.h"
#include "../user/app_stopwatch_user.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// Time string global variables (defined in UI file)
extern char tm_lbl_3_time_str[10];

// Timer animation counters
uint16_t app_stopwatch_view_timer_cnt = 0;
uint16_t lst_stopwatch_timer_cnt = 0;
uint16_t lst_stopwatch_item_0_timer_cnt = 0;
uint16_t lst_stopwatch_item_1_timer_cnt = 0;

// Event callback function implementations

void app_stopwatch_view_key_0_cb(void *obj, gui_event_t *e)
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

void bg_l_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    click_button_l(obj, e);
}

void bg_r_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    click_button_r(obj, e);
}

void tm_lbl_3_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(tm_lbl_3_time_str, sizeof(tm_lbl_3_time_str), "%02d:%02d", t->tm_hour, t->tm_min);

    gui_text_content_set((gui_text_t *)tm_lbl_3, tm_lbl_3_time_str, strlen(tm_lbl_3_time_str));
}

// Preset timer callback functions

/**
 * Animation 1
 * Component: app_stopwatch_view
 */
void app_stopwatch_view_timer_0_cb(void *obj)
{
    GUI_UNUSED(obj);
    // Call the implementation function in protected area (if exists)
    // Define app_stopwatch_view_timer_0_cb_impl() in custom_functions protected area for custom logic
#ifdef __cplusplus
    extern "C" {
#endif
    extern void app_stopwatch_view_timer_0_cb_impl(void) __attribute__((weak));
#ifdef __cplusplus
}
#endif

if (app_stopwatch_view_timer_0_cb_impl)
{
    app_stopwatch_view_timer_0_cb_impl();
}
else
{
    // TODO: Implement timer callback logic
    // Or define app_stopwatch_view_timer_0_cb_impl() in custom_functions protected area
}
}

/**
 * list timer
 * Component: lst_stopwatch
 */
void lst_stopwatch_timer_0_cb(void *obj)
{
    GUI_UNUSED(obj);
    // Call the implementation function in protected area (if exists)
    // Define lst_stopwatch_timer_0_cb_impl() in custom_functions protected area for custom logic
#ifdef __cplusplus
    extern "C" {
#endif
    extern void lst_stopwatch_timer_0_cb_impl(void) __attribute__((weak));
#ifdef __cplusplus
}
#endif

if (lst_stopwatch_timer_0_cb_impl)
{
    lst_stopwatch_timer_0_cb_impl();
}
else
{
    // TODO: Implement timer callback logic
    // Or define lst_stopwatch_timer_0_cb_impl() in custom_functions protected area
}
}

/**
 * Animation 1
 * Component: lst_stopwatch_item_0
 */
void stopwatch_page_0_timer_cb(void *obj)
{
    GUI_UNUSED(obj);
    // Call the implementation function in protected area (if exists)
    // Define stopwatch_page_0_timer_cb_impl() in custom_functions protected area for custom logic
#ifdef __cplusplus
    extern "C" {
#endif
    extern void stopwatch_page_0_timer_cb_impl(void) __attribute__((weak));
#ifdef __cplusplus
}
#endif

if (stopwatch_page_0_timer_cb_impl)
{
    stopwatch_page_0_timer_cb_impl();
}
else
{
    // TODO: Implement timer callback logic
    // Or define stopwatch_page_0_timer_cb_impl() in custom_functions protected area
}
}

/**
 * Animation 1
 * Component: lst_stopwatch_item_1
 */
void stopwatch_page_1_timer_cb(void *obj)
{
    GUI_UNUSED(obj);
    // Call the implementation function in protected area (if exists)
    // Define stopwatch_page_1_timer_cb_impl() in custom_functions protected area for custom logic
#ifdef __cplusplus
    extern "C" {
#endif
    extern void stopwatch_page_1_timer_cb_impl(void) __attribute__((weak));
#ifdef __cplusplus
}
#endif

if (stopwatch_page_1_timer_cb_impl)
{
    stopwatch_page_1_timer_cb_impl();
}
else
{
    // TODO: Implement timer callback logic
    // Or define stopwatch_page_1_timer_cb_impl() in custom_functions protected area
}
}

/* @protected start custom_functions */
// Custom functions
/* @protected end custom_functions */
