/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_timer_callbacks.h"
#include "../ui/app_timer_ui.h"
#include "../user/app_timer_user.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// Time string global variables (defined in UI file)
extern char tm_lbl_1_time_str[10];
extern char tm_lbl_2_time_str[10];

// Timer animation counters
uint16_t active_timer_text_timer_cnt = 0;
uint16_t active_arc_timer_cnt = 0;

// Event callback function implementations

void app_timer_view_key_0_cb(void *obj, gui_event_t *e)
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

void img_15_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    click_button_page_all_timers(obj, e);
}

void img_16_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    click_button_page_all_timers(obj, e);
}

void img_17_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    click_button_page_all_timers(obj, e);
}

void img_18_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    click_button_page_all_timers(obj, e);
}

void img_19_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    click_button_page_all_timers(obj, e);
}

void img_20_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    click_button_page_all_timers(obj, e);
}

void img_21_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    click_button_page_all_timers(obj, e);
}

void img_22_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    click_button_page_all_timers(obj, e);
}

void bg_cancel_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_menu_view", SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
}

void img_25_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_timer_view", SWITCH_OUT_NONE_ANIMATION,
                           SWITCH_IN_NONE_ANIMATION);
}

void bg_play_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    click_button_play_stop(obj, e);
}

void tm_lbl_1_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(tm_lbl_1_time_str, sizeof(tm_lbl_1_time_str), "%02d:%02d", t->tm_hour, t->tm_min);

    gui_text_content_set((gui_text_t *)tm_lbl_1, tm_lbl_1_time_str, strlen(tm_lbl_1_time_str));
}

void tm_lbl_2_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(tm_lbl_2_time_str, sizeof(tm_lbl_2_time_str), "%02d:%02d", t->tm_hour, t->tm_min);

    gui_text_content_set((gui_text_t *)tm_lbl_2, tm_lbl_2_time_str, strlen(tm_lbl_2_time_str));
}

/* @protected start custom_functions */
// Custom functions

void active_timer_update_cb(void *obj)
{
    GUI_UNUSED(obj);
    active_timer_rec_val -= 1;
    if (active_timer_rec_val == 0)
    {
        gui_text_content_set(obj, "DONE", 4);
        gui_obj_stop_timer(obj);
        gui_img_a8_recolor(icon_play, 0xFFFFFFFF);
        gui_img_set_src(icon_play, "/app_timer/timer_button_reset.bin", IMG_SRC_FILESYS);
        gui_obj_move((void *)icon_play, 325, 417);
    }
    else
    {
        update_timer_str(active_timer_rec_str, active_timer_rec_val, preset_timer_val[preset_index]);
        gui_text_content_set(obj, active_timer_rec_str, strlen(active_timer_rec_str));
        gui_text_content_set(active_preset_text, preset_timer_str, strlen(preset_timer_str));
    }
}

void active_arc_update_cb(void *obj)
{
    GUI_UNUSED(obj);
    if (active_timer_rec_val == 0)
    {
        gui_arc_set_start_angle((gui_arc_t *)obj, -90.f);
        gui_arc_set_end_angle((gui_arc_t *)obj, 270.f);
        gui_obj_stop_timer(obj);
    }
    else
    {
        float end_angle = -90.f + active_timer_rec_val * 360.f / preset_timer_val[preset_index];
        gui_arc_set_start_angle((gui_arc_t *)obj, -89.f);
        gui_arc_set_end_angle((gui_arc_t *)obj, end_angle);
    }
}
/* @protected end custom_functions */
