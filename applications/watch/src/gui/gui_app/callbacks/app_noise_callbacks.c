/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_noise_callbacks.h"
#include "../ui/app_noise_ui.h"
#include "../user/app_noise_user.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// Time string global variables (defined in UI file)
extern char hg_time_label_1769151874591_kcfw_time_str[10];

// Timer animation counters
uint16_t Nois_Level_Meter_bg_timer_cnt = 0;
uint16_t Nois_Level_Meter0_timer_cnt = 0;
uint16_t hg_image_1769156756841_h11r_timer_cnt = 0;

// Event callback function implementations

void Noise_ok_bg_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    // TODO: Implement event handling logic
}

void hg_image_1769151866129_h3az_key_cb(void *obj, gui_event_t *e)
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
        gui_view_switch_direct(gui_view_get_current(), "app_menu_view", SWITCH_OUT_ANIMATION_FADE,
                               SWITCH_IN_ANIMATION_FADE);
    }
}

void hg_time_label_1769151874591_kcfw_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(hg_time_label_1769151874591_kcfw_time_str,
             sizeof(hg_time_label_1769151874591_kcfw_time_str), "%02d:%02d", t->tm_hour, t->tm_min);

    gui_text_content_set((gui_text_t *)hg_time_label_1769151874591_kcfw,
                         hg_time_label_1769151874591_kcfw_time_str, strlen(hg_time_label_1769151874591_kcfw_time_str));
}

// Preset timer callback functions

/**
 * Timer animation 1
 * Component: Nois_Level_Meter_bg
 * Mode: Preset actions (multi-segment animation)
 * Segments: 1
 */
void Nois_Level_Meter_bg_timer_0_cb(void *obj)
{
    gui_obj_t *target = (gui_obj_t *)obj;
    const uint16_t total_cnt_max = 1;

    const uint16_t seg0_start = 0;
    const uint16_t seg0_end = 1;

    Nois_Level_Meter_bg_timer_cnt++;

    // Segment 1: 1000ms, 1 action(s)
    if (Nois_Level_Meter_bg_timer_cnt > seg0_start && Nois_Level_Meter_bg_timer_cnt <= seg0_end)
    {
        uint16_t seg_cnt = Nois_Level_Meter_bg_timer_cnt - seg0_start;
        const uint16_t seg_cnt_max = seg0_end - seg0_start;

        // Adjust opacity: 255 -> 128
        const uint8_t opacity_origin = 255;
        const uint8_t opacity_target = 128;
        int16_t opacity_cur = opacity_origin + (opacity_target - opacity_origin) * seg_cnt / seg_cnt_max;
        gui_img_set_opacity((gui_img_t *)target, opacity_cur);

    }

    if (Nois_Level_Meter_bg_timer_cnt >= total_cnt_max)
    {
        gui_obj_stop_timer(target);
        Nois_Level_Meter_bg_timer_cnt = 0; // Reset counter
    }
}


/**
 * Initialize timer
 * Component: Nois_Level_Meter0
 */
void app_noise_view_init(void *obj)
{
    GUI_UNUSED(obj);
    // Call the implementation function in protected area (if exists)
    // Define app_noise_view_init_impl() in custom_functions protected area for custom logic
#ifdef __cplusplus
    extern "C" {
#endif
    extern void app_noise_view_init_impl(void) __attribute__((weak));
#ifdef __cplusplus
}
#endif

if (app_noise_view_init_impl)
{
    app_noise_view_init_impl();
}
else
{
    // TODO: Implement timer callback logic
    // Or define app_noise_view_init_impl() in custom_functions protected area
}
}

/**
 * Animation 1
 * Component: hg_image_1769156756841_h11r
 * Mode: Preset actions (multi-segment animation)
 * Segments: 1
 */
void hg_image_1769156756841_h11r_timer_0_cb(void *obj)
{
    gui_obj_t *target = (gui_obj_t *)obj;
    const uint16_t total_cnt_max = 1;

    const uint16_t seg0_start = 0;
    const uint16_t seg0_end = 1;

    hg_image_1769156756841_h11r_timer_cnt++;

    // Segment 1: 1000ms, 1 action(s)
    if (hg_image_1769156756841_h11r_timer_cnt > seg0_start &&
        hg_image_1769156756841_h11r_timer_cnt <= seg0_end)
    {
        // Change image: /app_noise/Noise_warning_icon.bin
        gui_img_set_src((gui_img_t *)target, (const uint8_t *)"/app_noise/Noise_warning_icon.bin",
                        IMG_SRC_FILESYS);
        gui_img_refresh_size((gui_img_t *)target);

    }

    if (hg_image_1769156756841_h11r_timer_cnt >= total_cnt_max)
    {
        gui_obj_stop_timer(target);
        hg_image_1769156756841_h11r_timer_cnt = 0; // Reset counter
    }
}


/* @protected start custom_functions */
// Custom functions
/* @protected end custom_functions */
