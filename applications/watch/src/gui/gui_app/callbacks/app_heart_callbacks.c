/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_heart_callbacks.h"
#include "../ui/app_heart_ui.h"
#include "../user/app_heart_user.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// Time string global variables (defined in UI file)
extern char hg_time_label_heart_time_str[10];

// Timer animation counters
uint16_t hg_image_1769146380658_kvde_timer_cnt = 0;
uint16_t app_heart_circel0_timer_cnt = 0;

// Event callback function implementations

void app_heart_window_key_0_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    // Check key name
    if (strcmp(e->indev_name, "Home") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "SmartWatchTemplateMainView",
                               SWITCH_OUT_NONE_ANIMATION, SWITCH_IN_ANIMATION_FADE);
    }
    else if (strcmp(e->indev_name, "Menu") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "app_menu_view", SWITCH_OUT_NONE_ANIMATION,
                               SWITCH_IN_ANIMATION_FADE);
    }
}

void hg_time_label_heart_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(hg_time_label_heart_time_str, sizeof(hg_time_label_heart_time_str), "%02d:%02d",
             t->tm_hour, t->tm_min);

    gui_text_content_set((gui_text_t *)hg_time_label_heart, hg_time_label_heart_time_str,
                         strlen(hg_time_label_heart_time_str));
}

// Preset timer callback functions

/**
 * Timer animation 1
 * Component: hg_image_1769146380658_kvde
 * Mode: Preset actions (multi-segment animation)
 * Segments: 1
 */
void hg_image_1769146380658_kvde_timer_0_cb(void *obj)
{
    gui_obj_t *target = (gui_obj_t *)obj;
    const uint16_t total_cnt_max = 10;

    const uint16_t seg0_start = 0;
    const uint16_t seg0_end = 10;

    hg_image_1769146380658_kvde_timer_cnt++;

    // Segment 1: 1000ms, 1 action(s)
    if (hg_image_1769146380658_kvde_timer_cnt > seg0_start &&
        hg_image_1769146380658_kvde_timer_cnt <= seg0_end)
    {
        uint16_t seg_cnt = hg_image_1769146380658_kvde_timer_cnt - seg0_start;
        const uint16_t seg_cnt_max = seg0_end - seg0_start;

        // Adjust scale: (1, 1) -> (1.3, 1.3)
        const float zoom_x_origin = 1;
        const float zoom_x_target = 1.3;
        const float zoom_y_origin = 1;
        const float zoom_y_target = 1.3;
        float zoom_x_cur = zoom_x_origin + (zoom_x_target - zoom_x_origin) * seg_cnt / seg_cnt_max;
        float zoom_y_cur = zoom_y_origin + (zoom_y_target - zoom_y_origin) * seg_cnt / seg_cnt_max;
        gui_img_scale((gui_img_t *)target, zoom_x_cur, zoom_y_cur);

    }

    if (hg_image_1769146380658_kvde_timer_cnt >= total_cnt_max)
    {
        hg_image_1769146380658_kvde_timer_cnt = 0; // Reset counter, continue loop
    }
}


/* @protected start custom_functions */
// Custom function

void app_heart_circel0_timer_0_cb(void *obj)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(app_heart_circel0_timer_cnt);
    if (list_heart->offset > -251)
    {
        gui_circle_set_color(app_heart_circel0, gui_rgb(255, 255, 255));
        gui_circle_set_color(app_heart_circel1, gui_rgb(66, 62, 62));
    }
    else
    {
        gui_circle_set_color(app_heart_circel1, gui_rgb(255, 255, 255));
        gui_circle_set_color(app_heart_circel0, gui_rgb(66, 62, 62));
    }
}
/* @protected end custom_functions */
