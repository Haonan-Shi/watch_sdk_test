/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_ota_callbacks.h"
#include "../ui/app_ota_ui.h"
#include "../user/app_ota_user.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// Timer animation counters
uint16_t ota_ready_dots_timer_cnt = 0;
uint16_t ota_starting_glow_timer_cnt = 0;
uint16_t ota_starting_ring_bg_timer_cnt = 0;
uint16_t ota_starting_dots_timer_cnt = 0;
uint16_t ota_updating_glow_timer_cnt = 0;
uint16_t ota_updating_ring_bg_timer_cnt = 0;
uint16_t ota_updating_dots_timer_cnt = 0;

// Event callback function implementations

void app_otaReadyView_key_0_cb(void *obj, gui_event_t *e)
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

void ota_ready_back_btn_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_menu_view", SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
}

void ota_connect_btn_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_otaStartingView", SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
}

void app_otaStartingView_key_0_cb(void *obj, gui_event_t *e)
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

void app_otaUpdatingView_key_0_cb(void *obj, gui_event_t *e)
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

void app_otaSuccessView_key_0_cb(void *obj, gui_event_t *e)
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

void ota_success_back_btn_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    ota_reset_to_ready(obj, e);
}

void ota_done_btn_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    ota_reset_to_ready(obj, e);
}

void app_otaFailedView_key_0_cb(void *obj, gui_event_t *e)
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

void ota_failed_back_btn_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    ota_reset_to_ready(obj, e);
}

void ota_retry_btn_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    ota_retry(obj, e);
}

// Preset timer callback functions

/**
 * dots_anim
 * Component: ota_ready_dots
 * Mode: Preset actions (multi-segment animation)
 * Segments: 1
 */
void ota_ready_dots_timer_0_cb(void *obj)
{
    gui_obj_t *target = (gui_obj_t *)obj;
    const uint16_t total_cnt_max = 31;

    const uint16_t seg0_start = 0;
    const uint16_t seg0_end = 31;

    ota_ready_dots_timer_cnt++;

    // Segment 1: 1000ms, 1 action(s)
    if (ota_ready_dots_timer_cnt > seg0_start && ota_ready_dots_timer_cnt <= seg0_end)
    {
        uint16_t seg_cnt = ota_ready_dots_timer_cnt - seg0_start;
        const uint16_t seg_cnt_max = seg0_end - seg0_start;

        // Image sequence animation: 30 images
        const void *img_data_array[30] =
        {
            "/app_ota/pulsing_dots/frame_00.bin",
            "/app_ota/pulsing_dots/frame_01.bin",
            "/app_ota/pulsing_dots/frame_02.bin",
            "/app_ota/pulsing_dots/frame_03.bin",
            "/app_ota/pulsing_dots/frame_04.bin",
            "/app_ota/pulsing_dots/frame_05.bin",
            "/app_ota/pulsing_dots/frame_06.bin",
            "/app_ota/pulsing_dots/frame_07.bin",
            "/app_ota/pulsing_dots/frame_08.bin",
            "/app_ota/pulsing_dots/frame_09.bin",
            "/app_ota/pulsing_dots/frame_10.bin",
            "/app_ota/pulsing_dots/frame_11.bin",
            "/app_ota/pulsing_dots/frame_12.bin",
            "/app_ota/pulsing_dots/frame_13.bin",
            "/app_ota/pulsing_dots/frame_14.bin",
            "/app_ota/pulsing_dots/frame_15.bin",
            "/app_ota/pulsing_dots/frame_16.bin",
            "/app_ota/pulsing_dots/frame_17.bin",
            "/app_ota/pulsing_dots/frame_18.bin",
            "/app_ota/pulsing_dots/frame_19.bin",
            "/app_ota/pulsing_dots/frame_20.bin",
            "/app_ota/pulsing_dots/frame_21.bin",
            "/app_ota/pulsing_dots/frame_22.bin",
            "/app_ota/pulsing_dots/frame_23.bin",
            "/app_ota/pulsing_dots/frame_24.bin",
            "/app_ota/pulsing_dots/frame_25.bin",
            "/app_ota/pulsing_dots/frame_26.bin",
            "/app_ota/pulsing_dots/frame_27.bin",
            "/app_ota/pulsing_dots/frame_28.bin",
            "/app_ota/pulsing_dots/frame_29.bin"
        };
        uint16_t index = (30 - 1) * seg_cnt / seg_cnt_max;
        gui_img_set_src((gui_img_t *)target, (const uint8_t *)img_data_array[index], IMG_SRC_FILESYS);
        gui_img_refresh_size((gui_img_t *)target);

    }

    if (ota_ready_dots_timer_cnt >= total_cnt_max)
    {
        ota_ready_dots_timer_cnt = 0; // Reset counter, continue loop
    }
}


/**
 * glow_anim
 * Component: ota_starting_glow
 * Mode: Preset actions (multi-segment animation)
 * Segments: 1
 */
void ota_starting_glow_timer_0_cb(void *obj)
{
    gui_obj_t *target = (gui_obj_t *)obj;
    const uint16_t total_cnt_max = 46;

    const uint16_t seg0_start = 0;
    const uint16_t seg0_end = 46;

    ota_starting_glow_timer_cnt++;

    // Segment 1: 1500ms, 1 action(s)
    if (ota_starting_glow_timer_cnt > seg0_start && ota_starting_glow_timer_cnt <= seg0_end)
    {
        uint16_t seg_cnt = ota_starting_glow_timer_cnt - seg0_start;
        const uint16_t seg_cnt_max = seg0_end - seg0_start;

        // Image sequence animation: 30 images
        const void *img_data_array[30] =
        {
            "/app_ota/glow_pulse/frame_00.bin",
            "/app_ota/glow_pulse/frame_01.bin",
            "/app_ota/glow_pulse/frame_02.bin",
            "/app_ota/glow_pulse/frame_03.bin",
            "/app_ota/glow_pulse/frame_04.bin",
            "/app_ota/glow_pulse/frame_05.bin",
            "/app_ota/glow_pulse/frame_06.bin",
            "/app_ota/glow_pulse/frame_07.bin",
            "/app_ota/glow_pulse/frame_08.bin",
            "/app_ota/glow_pulse/frame_09.bin",
            "/app_ota/glow_pulse/frame_10.bin",
            "/app_ota/glow_pulse/frame_11.bin",
            "/app_ota/glow_pulse/frame_12.bin",
            "/app_ota/glow_pulse/frame_13.bin",
            "/app_ota/glow_pulse/frame_14.bin",
            "/app_ota/glow_pulse/frame_15.bin",
            "/app_ota/glow_pulse/frame_16.bin",
            "/app_ota/glow_pulse/frame_17.bin",
            "/app_ota/glow_pulse/frame_18.bin",
            "/app_ota/glow_pulse/frame_19.bin",
            "/app_ota/glow_pulse/frame_20.bin",
            "/app_ota/glow_pulse/frame_21.bin",
            "/app_ota/glow_pulse/frame_22.bin",
            "/app_ota/glow_pulse/frame_23.bin",
            "/app_ota/glow_pulse/frame_24.bin",
            "/app_ota/glow_pulse/frame_25.bin",
            "/app_ota/glow_pulse/frame_26.bin",
            "/app_ota/glow_pulse/frame_27.bin",
            "/app_ota/glow_pulse/frame_28.bin",
            "/app_ota/glow_pulse/frame_29.bin"
        };
        uint16_t index = (30 - 1) * seg_cnt / seg_cnt_max;
        gui_img_set_src((gui_img_t *)target, (const uint8_t *)img_data_array[index], IMG_SRC_FILESYS);
        gui_img_refresh_size((gui_img_t *)target);

    }

    if (ota_starting_glow_timer_cnt >= total_cnt_max)
    {
        ota_starting_glow_timer_cnt = 0; // Reset counter, continue loop
    }
}


/**
 * starting_delay
 * Component: ota_starting_ring_bg
 */
void ota_starting_timer_cb(void *obj)
{
    GUI_UNUSED(obj);
    // Call the implementation function in protected area (if exists)
    // Define ota_starting_timer_cb_impl() in custom_functions protected area for custom logic
#ifdef __cplusplus
    extern "C" {
#endif
    extern void ota_starting_timer_cb_impl(void) __attribute__((weak));
#ifdef __cplusplus
}
#endif

if (ota_starting_timer_cb_impl)
{
    ota_starting_timer_cb_impl();
}
else
{
    // TODO: Implement timer callback logic
    // Or define ota_starting_timer_cb_impl() in custom_functions protected area
}
}

/**
 * dots_anim
 * Component: ota_starting_dots
 * Mode: Preset actions (multi-segment animation)
 * Segments: 1
 */
void ota_starting_dots_timer_0_cb(void *obj)
{
    gui_obj_t *target = (gui_obj_t *)obj;
    const uint16_t total_cnt_max = 31;

    const uint16_t seg0_start = 0;
    const uint16_t seg0_end = 31;

    ota_starting_dots_timer_cnt++;

    // Segment 1: 1000ms, 1 action(s)
    if (ota_starting_dots_timer_cnt > seg0_start && ota_starting_dots_timer_cnt <= seg0_end)
    {
        uint16_t seg_cnt = ota_starting_dots_timer_cnt - seg0_start;
        const uint16_t seg_cnt_max = seg0_end - seg0_start;

        // Image sequence animation: 30 images
        const void *img_data_array[30] =
        {
            "/app_ota/pulsing_dots/frame_00.bin",
            "/app_ota/pulsing_dots/frame_01.bin",
            "/app_ota/pulsing_dots/frame_02.bin",
            "/app_ota/pulsing_dots/frame_03.bin",
            "/app_ota/pulsing_dots/frame_04.bin",
            "/app_ota/pulsing_dots/frame_05.bin",
            "/app_ota/pulsing_dots/frame_06.bin",
            "/app_ota/pulsing_dots/frame_07.bin",
            "/app_ota/pulsing_dots/frame_08.bin",
            "/app_ota/pulsing_dots/frame_09.bin",
            "/app_ota/pulsing_dots/frame_10.bin",
            "/app_ota/pulsing_dots/frame_11.bin",
            "/app_ota/pulsing_dots/frame_12.bin",
            "/app_ota/pulsing_dots/frame_13.bin",
            "/app_ota/pulsing_dots/frame_14.bin",
            "/app_ota/pulsing_dots/frame_15.bin",
            "/app_ota/pulsing_dots/frame_16.bin",
            "/app_ota/pulsing_dots/frame_17.bin",
            "/app_ota/pulsing_dots/frame_18.bin",
            "/app_ota/pulsing_dots/frame_19.bin",
            "/app_ota/pulsing_dots/frame_20.bin",
            "/app_ota/pulsing_dots/frame_21.bin",
            "/app_ota/pulsing_dots/frame_22.bin",
            "/app_ota/pulsing_dots/frame_23.bin",
            "/app_ota/pulsing_dots/frame_24.bin",
            "/app_ota/pulsing_dots/frame_25.bin",
            "/app_ota/pulsing_dots/frame_26.bin",
            "/app_ota/pulsing_dots/frame_27.bin",
            "/app_ota/pulsing_dots/frame_28.bin",
            "/app_ota/pulsing_dots/frame_29.bin"
        };
        uint16_t index = (30 - 1) * seg_cnt / seg_cnt_max;
        gui_img_set_src((gui_img_t *)target, (const uint8_t *)img_data_array[index], IMG_SRC_FILESYS);
        gui_img_refresh_size((gui_img_t *)target);

    }

    if (ota_starting_dots_timer_cnt >= total_cnt_max)
    {
        ota_starting_dots_timer_cnt = 0; // Reset counter, continue loop
    }
}


/**
 * glow_anim
 * Component: ota_updating_glow
 * Mode: Preset actions (multi-segment animation)
 * Segments: 1
 */
void ota_updating_glow_timer_0_cb(void *obj)
{
    gui_obj_t *target = (gui_obj_t *)obj;
    const uint16_t total_cnt_max = 46;

    const uint16_t seg0_start = 0;
    const uint16_t seg0_end = 46;

    ota_updating_glow_timer_cnt++;

    // Segment 1: 1500ms, 1 action(s)
    if (ota_updating_glow_timer_cnt > seg0_start && ota_updating_glow_timer_cnt <= seg0_end)
    {
        uint16_t seg_cnt = ota_updating_glow_timer_cnt - seg0_start;
        const uint16_t seg_cnt_max = seg0_end - seg0_start;

        // Image sequence animation: 30 images
        const void *img_data_array[30] =
        {
            "/app_ota/glow_pulse/frame_00.bin",
            "/app_ota/glow_pulse/frame_01.bin",
            "/app_ota/glow_pulse/frame_02.bin",
            "/app_ota/glow_pulse/frame_03.bin",
            "/app_ota/glow_pulse/frame_04.bin",
            "/app_ota/glow_pulse/frame_05.bin",
            "/app_ota/glow_pulse/frame_06.bin",
            "/app_ota/glow_pulse/frame_07.bin",
            "/app_ota/glow_pulse/frame_08.bin",
            "/app_ota/glow_pulse/frame_09.bin",
            "/app_ota/glow_pulse/frame_10.bin",
            "/app_ota/glow_pulse/frame_11.bin",
            "/app_ota/glow_pulse/frame_12.bin",
            "/app_ota/glow_pulse/frame_13.bin",
            "/app_ota/glow_pulse/frame_14.bin",
            "/app_ota/glow_pulse/frame_15.bin",
            "/app_ota/glow_pulse/frame_16.bin",
            "/app_ota/glow_pulse/frame_17.bin",
            "/app_ota/glow_pulse/frame_18.bin",
            "/app_ota/glow_pulse/frame_19.bin",
            "/app_ota/glow_pulse/frame_20.bin",
            "/app_ota/glow_pulse/frame_21.bin",
            "/app_ota/glow_pulse/frame_22.bin",
            "/app_ota/glow_pulse/frame_23.bin",
            "/app_ota/glow_pulse/frame_24.bin",
            "/app_ota/glow_pulse/frame_25.bin",
            "/app_ota/glow_pulse/frame_26.bin",
            "/app_ota/glow_pulse/frame_27.bin",
            "/app_ota/glow_pulse/frame_28.bin",
            "/app_ota/glow_pulse/frame_29.bin"
        };
        uint16_t index = (30 - 1) * seg_cnt / seg_cnt_max;
        gui_img_set_src((gui_img_t *)target, (const uint8_t *)img_data_array[index], IMG_SRC_FILESYS);
        gui_img_refresh_size((gui_img_t *)target);

    }

    if (ota_updating_glow_timer_cnt >= total_cnt_max)
    {
        ota_updating_glow_timer_cnt = 0; // Reset counter, continue loop
    }
}


/**
 * progress_tick
 * Component: ota_updating_ring_bg
 */
void ota_progress_tick_cb(void *obj)
{
    GUI_UNUSED(obj);
    // Call the implementation function in protected area (if exists)
    // Define ota_progress_tick_cb_impl() in custom_functions protected area for custom logic
#ifdef __cplusplus
    extern "C" {
#endif
    extern void ota_progress_tick_cb_impl(void) __attribute__((weak));
#ifdef __cplusplus
}
#endif

if (ota_progress_tick_cb_impl)
{
    ota_progress_tick_cb_impl();
}
else
{
    // TODO: Implement timer callback logic
    // Or define ota_progress_tick_cb_impl() in custom_functions protected area
}
}

/**
 * dots_anim
 * Component: ota_updating_dots
 * Mode: Preset actions (multi-segment animation)
 * Segments: 1
 */
void ota_updating_dots_timer_0_cb(void *obj)
{
    gui_obj_t *target = (gui_obj_t *)obj;
    const uint16_t total_cnt_max = 31;

    const uint16_t seg0_start = 0;
    const uint16_t seg0_end = 31;

    ota_updating_dots_timer_cnt++;

    // Segment 1: 1000ms, 1 action(s)
    if (ota_updating_dots_timer_cnt > seg0_start && ota_updating_dots_timer_cnt <= seg0_end)
    {
        uint16_t seg_cnt = ota_updating_dots_timer_cnt - seg0_start;
        const uint16_t seg_cnt_max = seg0_end - seg0_start;

        // Image sequence animation: 30 images
        const void *img_data_array[30] =
        {
            "/app_ota/pulsing_dots/frame_00.bin",
            "/app_ota/pulsing_dots/frame_01.bin",
            "/app_ota/pulsing_dots/frame_02.bin",
            "/app_ota/pulsing_dots/frame_03.bin",
            "/app_ota/pulsing_dots/frame_04.bin",
            "/app_ota/pulsing_dots/frame_05.bin",
            "/app_ota/pulsing_dots/frame_06.bin",
            "/app_ota/pulsing_dots/frame_07.bin",
            "/app_ota/pulsing_dots/frame_08.bin",
            "/app_ota/pulsing_dots/frame_09.bin",
            "/app_ota/pulsing_dots/frame_10.bin",
            "/app_ota/pulsing_dots/frame_11.bin",
            "/app_ota/pulsing_dots/frame_12.bin",
            "/app_ota/pulsing_dots/frame_13.bin",
            "/app_ota/pulsing_dots/frame_14.bin",
            "/app_ota/pulsing_dots/frame_15.bin",
            "/app_ota/pulsing_dots/frame_16.bin",
            "/app_ota/pulsing_dots/frame_17.bin",
            "/app_ota/pulsing_dots/frame_18.bin",
            "/app_ota/pulsing_dots/frame_19.bin",
            "/app_ota/pulsing_dots/frame_20.bin",
            "/app_ota/pulsing_dots/frame_21.bin",
            "/app_ota/pulsing_dots/frame_22.bin",
            "/app_ota/pulsing_dots/frame_23.bin",
            "/app_ota/pulsing_dots/frame_24.bin",
            "/app_ota/pulsing_dots/frame_25.bin",
            "/app_ota/pulsing_dots/frame_26.bin",
            "/app_ota/pulsing_dots/frame_27.bin",
            "/app_ota/pulsing_dots/frame_28.bin",
            "/app_ota/pulsing_dots/frame_29.bin"
        };
        uint16_t index = (30 - 1) * seg_cnt / seg_cnt_max;
        gui_img_set_src((gui_img_t *)target, (const uint8_t *)img_data_array[index], IMG_SRC_FILESYS);
        gui_img_refresh_size((gui_img_t *)target);

    }

    if (ota_updating_dots_timer_cnt >= total_cnt_max)
    {
        ota_updating_dots_timer_cnt = 0; // Reset counter, continue loop
    }
}


/* @protected start custom_functions */
// Custom functions
/* @protected end custom_functions */
