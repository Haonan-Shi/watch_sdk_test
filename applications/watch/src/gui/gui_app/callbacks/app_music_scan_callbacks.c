/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_music_scan_callbacks.h"
#include "../ui/app_music_scan_ui.h"
#include "../user/app_music_scan_user.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// Time string global variables (defined in UI file)
extern char music_scan_time_text_time_str[10];

// Timer animation counters
uint16_t music_scan_icon_bg_timer_cnt = 0;

// Event callback function implementations

void app_music_scan_view_key_0_cb(void *obj, gui_event_t *e)
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

void music_scan_icon_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    music_scan_icon_bg_timer_cnt = 0; // Reset counter
    gui_obj_create_timer(GUI_BASE(music_scan_icon_bg), 1000, true, music_scan_icon_bg_timer_0_cb);
    gui_obj_start_timer(GUI_BASE(music_scan_icon_bg));
}

void music_scan_icon_key_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    // Check key name
    if (strcmp(e->indev_name, "Home") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "SmartWatchTemplateMainView",
                               SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
    }
}

void music_scan_time_text_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(music_scan_time_text_time_str, sizeof(music_scan_time_text_time_str), "%02d:%02d",
             t->tm_hour, t->tm_min);

    gui_text_content_set((gui_text_t *)music_scan_time_text, music_scan_time_text_time_str,
                         strlen(music_scan_time_text_time_str));
}

// Preset timer callback functions

/**
 * music_scan_bg_change
 * Component: music_scan_icon_bg
 * Mode: Preset actions (multi-segment animation)
 * Segments: 1
 */
void music_scan_icon_bg_timer_0_cb(void *obj)
{
    gui_obj_t *target = (gui_obj_t *)obj;
    const uint16_t total_cnt_max = 3;

    const uint16_t seg0_start = 0;
    const uint16_t seg0_end = 3;

    music_scan_icon_bg_timer_cnt++;

    // Segment 1: 3000ms, 2 action(s)
    if (music_scan_icon_bg_timer_cnt > seg0_start && music_scan_icon_bg_timer_cnt <= seg0_end)
    {
        uint16_t seg_cnt = music_scan_icon_bg_timer_cnt - seg0_start;
        const uint16_t seg_cnt_max = seg0_end - seg0_start;

        // Adjust opacity: 255 -> 128
        const uint8_t opacity_origin = 255;
        const uint8_t opacity_target = 128;
        int16_t opacity_cur = opacity_origin + (opacity_target - opacity_origin) * seg_cnt / seg_cnt_max;
        gui_img_set_opacity((gui_img_t *)target, opacity_cur);

        // Adjust scale: (1, 1) -> (1.5, 1.5)
        const float zoom_x_origin = 1;
        const float zoom_x_target = 1.5;
        const float zoom_y_origin = 1;
        const float zoom_y_target = 1.5;
        float zoom_x_cur = zoom_x_origin + (zoom_x_target - zoom_x_origin) * seg_cnt / seg_cnt_max;
        float zoom_y_cur = zoom_y_origin + (zoom_y_target - zoom_y_origin) * seg_cnt / seg_cnt_max;
        gui_img_scale((gui_img_t *)target, zoom_x_cur, zoom_y_cur);

    }

    if (music_scan_icon_bg_timer_cnt >= total_cnt_max)
    {
        music_scan_icon_bg_timer_cnt = 0; // Reset counter, continue loop
    }
}


/* @protected start custom_functions */
// Custom functions
/* @protected end custom_functions */
