/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_workout_callbacks.h"
#include "../ui/app_workout_ui.h"
#include "../user/app_workout_user.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// Time string global variables (defined in UI file)
extern char app_workout_time_text_time_str[10];
extern char hg_time_label_1769406976200_a8x9_time_str[10];

// Timer animation counters
uint16_t app_workout_icon_bg1_timer_cnt = 0;
uint16_t app_workout_icon1_timer_cnt = 0;
uint16_t app_workout_start_icon_timer_cnt = 0;
uint16_t workout_countdown_arc_timer_cnt = 0;

// Event callback function implementations

void app_workout_menu_topwindow_key_0_cb(void *obj, gui_event_t *e)
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

void app_workout_start_icon_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_workout_start_engine_view",
                           SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
}

void app_workout_start_engine_bg_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    // TODO: Implement event handling logic
}

void workout_countdown_arc_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_workout_view", SWITCH_OUT_NONE_ANIMATION,
                           SWITCH_IN_NONE_ANIMATION);
}

void app_workout_time_text_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(app_workout_time_text_time_str, sizeof(app_workout_time_text_time_str), "%02d:%02d",
             t->tm_hour, t->tm_min);

    gui_text_content_set((gui_text_t *)app_workout_time_text, app_workout_time_text_time_str,
                         strlen(app_workout_time_text_time_str));
}

void hg_time_label_1769406976200_a8x9_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(hg_time_label_1769406976200_a8x9_time_str,
             sizeof(hg_time_label_1769406976200_a8x9_time_str), "%02d:%02d", t->tm_hour, t->tm_min);

    gui_text_content_set((gui_text_t *)hg_time_label_1769406976200_a8x9,
                         hg_time_label_1769406976200_a8x9_time_str, strlen(hg_time_label_1769406976200_a8x9_time_str));
}

// Preset timer callback functions

/**
 * Timer animation 1
 * Component: app_workout_icon_bg1
 */
void hg_image_1769161039267_t63r_timer_0_cb(void *obj)
{
    GUI_UNUSED(obj);
    // Call the implementation function in protected area (if exists)
    // Define hg_image_1769161039267_t63r_timer_0_cb_impl() in custom_functions protected area for custom logic
#ifdef __cplusplus
    extern "C" {
#endif
    extern void hg_image_1769161039267_t63r_timer_0_cb_impl(void) __attribute__((weak));
#ifdef __cplusplus
}
#endif

if (hg_image_1769161039267_t63r_timer_0_cb_impl)
{
    hg_image_1769161039267_t63r_timer_0_cb_impl();
}
else
{
    // TODO: Implement timer callback logic
    // Or define hg_image_1769161039267_t63r_timer_0_cb_impl() in custom_functions protected area
}
}

/**
 * Timer animation 2
 * Component: app_workout_icon_bg1
 * Mode: Preset actions (multi-segment animation)
 * Segments: 1
 */
void app_workout_icon_bg1_timer_1_cb(void *obj)
{
    gui_obj_t *target = (gui_obj_t *)obj;
    const uint16_t total_cnt_max = 1;

    const uint16_t seg0_start = 0;
    const uint16_t seg0_end = 1;

    app_workout_icon_bg1_timer_cnt++;

    // Segment 1: 1000ms, 1 action(s)
    if (app_workout_icon_bg1_timer_cnt > seg0_start && app_workout_icon_bg1_timer_cnt <= seg0_end)
    {
        uint16_t seg_cnt = app_workout_icon_bg1_timer_cnt - seg0_start;
        const uint16_t seg_cnt_max = seg0_end - seg0_start;

        // Adjust opacity: 255 -> 128
        const uint8_t opacity_origin = 255;
        const uint8_t opacity_target = 128;
        int16_t opacity_cur = opacity_origin + (opacity_target - opacity_origin) * seg_cnt / seg_cnt_max;
        gui_img_set_opacity((gui_img_t *)target, opacity_cur);

    }

    if (app_workout_icon_bg1_timer_cnt >= total_cnt_max)
    {
        gui_obj_stop_timer(target);
        app_workout_icon_bg1_timer_cnt = 0; // Reset counter
    }
}


/**
 * press
 * Component: app_workout_start_icon
 * Mode: Preset actions (multi-segment animation)
 * Segments: 1
 */
void app_workout_start_icon_timer_0_cb(void *obj)
{
    gui_obj_t *target = (gui_obj_t *)obj;
    const uint16_t total_cnt_max = 10;

    const uint16_t seg0_start = 0;
    const uint16_t seg0_end = 10;

    app_workout_start_icon_timer_cnt++;

    // Segment 1: 1000ms, 1 action(s)
    if (app_workout_start_icon_timer_cnt > seg0_start && app_workout_start_icon_timer_cnt <= seg0_end)
    {
        uint16_t seg_cnt = app_workout_start_icon_timer_cnt - seg0_start;
        const uint16_t seg_cnt_max = seg0_end - seg0_start;

        // Adjust scale: (1, 1) -> (1.2, 1.2)
        const float zoom_x_origin = 1;
        const float zoom_x_target = 1.2;
        const float zoom_y_origin = 1;
        const float zoom_y_target = 1.2;
        float zoom_x_cur = zoom_x_origin + (zoom_x_target - zoom_x_origin) * seg_cnt / seg_cnt_max;
        float zoom_y_cur = zoom_y_origin + (zoom_y_target - zoom_y_origin) * seg_cnt / seg_cnt_max;
        gui_img_scale((gui_img_t *)target, zoom_x_cur, zoom_y_cur);

    }

    if (app_workout_start_icon_timer_cnt >= total_cnt_max)
    {
        gui_obj_stop_timer(target);
        app_workout_start_icon_timer_cnt = 0; // Reset counter
    }
}


/* @protected start custom_functions */
// Custom functions
/* @protected end custom_functions */
