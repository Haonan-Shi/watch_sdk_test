/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_compass_callbacks.h"
#include "../ui/app_compass_ui.h"
#include "../user/app_compass_user.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// Time string global variables (defined in UI file)
extern char compass_time_label_time_str[10];

// Timer animation counters
uint16_t compass_image_timer_cnt = 0;

// Event callback function implementations

void compass_window_key_0_cb(void *obj, gui_event_t *e)
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

void compass_time_label_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(compass_time_label_time_str, sizeof(compass_time_label_time_str), "%02d:%02d", t->tm_hour,
             t->tm_min);

    gui_text_content_set((gui_text_t *)compass_time_label, compass_time_label_time_str,
                         strlen(compass_time_label_time_str));
}

// Preset timer callback functions

/**
 * compass_sim
 * Component: compass_image
 * Mode: Preset actions (multi-segment animation)
 * Segments: 1
 */
void compass_image_timer_0_cb(void *obj)
{
    gui_obj_t *target = (gui_obj_t *)obj;
    const uint16_t total_cnt_max = 36;

    const uint16_t seg0_start = 0;
    const uint16_t seg0_end = 36;

    compass_image_timer_cnt++;

    // Segment 1: 36000ms, 1 action(s)
    if (compass_image_timer_cnt > seg0_start && compass_image_timer_cnt <= seg0_end)
    {
        uint16_t seg_cnt = compass_image_timer_cnt - seg0_start;
        const uint16_t seg_cnt_max = seg0_end - seg0_start;

        // Adjust rotation: 0° -> 360°
        const float angle_origin = 0;
        const float angle_target = 360;
        float angle_cur = angle_origin + (angle_target - angle_origin) * seg_cnt / seg_cnt_max;
        gui_img_rotation((gui_img_t *)target, angle_cur);

    }

    if (compass_image_timer_cnt >= total_cnt_max)
    {
        compass_image_timer_cnt = 0; // Reset counter, continue loop
    }
}


/* @protected start custom_functions */
// Custom functions
/* @protected end custom_functions */
