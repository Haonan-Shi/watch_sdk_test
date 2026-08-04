/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "SmartWatchMain_callbacks.h"
#include "../ui/SmartWatchMain_ui.h"
#include "../user/SmartWatchMain_user.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// Time string global variables (defined in UI file)
extern char hg_time_label_hh_time_str[4];
extern char hg_time_label_mm_time_str[4];
extern char hg_time_label_1772765275313_pgx4_time_str[10];
extern char hg_time_label_1772765275313_pgx4_copy_1772765661189_2_time_str[10];

// Timer animation counters
uint16_t win_clock_big_timer_cnt = 0;
uint16_t text_date_big_timer_cnt = 0;
uint16_t text_date_small_timer_cnt = 0;

// Event callback function implementations

void SmartWatchTemplateMainView_key_0_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    // Check key name
    if (strcmp(e->indev_name, "Menu") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "app_menu_view", SWITCH_INIT_STATE,
                               SWITCH_IN_ANIMATION_FADE);
    }
}

void samrtWatch_window_key_0_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    // Check key name
    if (strcmp(e->indev_name, "Menu") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "app_menu_view", SWITCH_OUT_ANIMATION_FADE,
                               SWITCH_IN_ANIMATION_FADE);
    }
}

void bottom_View_weather_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_weatherMainView", SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
}

void hg_image_1769134788793_og20_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_activity_view", SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
}

void hg_image_1768183941920_4wxc_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_music_view", SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
}

void hg_image_1768184009679_rg6a_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_message_view", SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
}

void hg_arc_1768184103087_n36y_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_battery_view",
                           SWITCH_OUT_TO_BOTTOM_USE_TRANSLATION, SWITCH_IN_ANIMATION_FADE);
}

void bottom_View_tag_bg_menu_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_menu_view", SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
}

void hg_time_label_hh_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(hg_time_label_hh_time_str, sizeof(hg_time_label_hh_time_str), "%02d", t->tm_hour);

    gui_text_content_set((gui_text_t *)hg_time_label_hh, hg_time_label_hh_time_str,
                         strlen(hg_time_label_hh_time_str));
}

void hg_time_label_mm_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(hg_time_label_mm_time_str, sizeof(hg_time_label_mm_time_str), "%02d", t->tm_min);

    gui_text_content_set((gui_text_t *)hg_time_label_mm, hg_time_label_mm_time_str,
                         strlen(hg_time_label_mm_time_str));
}

void hg_time_label_1772765275313_pgx4_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(hg_time_label_1772765275313_pgx4_time_str,
             sizeof(hg_time_label_1772765275313_pgx4_time_str), "%02d:%02d", t->tm_hour, t->tm_min);

    gui_text_content_set((gui_text_t *)hg_time_label_1772765275313_pgx4,
                         hg_time_label_1772765275313_pgx4_time_str, strlen(hg_time_label_1772765275313_pgx4_time_str));
}

void hg_time_label_1772765275313_pgx4_copy_1772765661189_2_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(hg_time_label_1772765275313_pgx4_copy_1772765661189_2_time_str,
             sizeof(hg_time_label_1772765275313_pgx4_copy_1772765661189_2_time_str), "%02d:%02d", t->tm_hour,
             t->tm_min);

    gui_text_content_set((gui_text_t *)hg_time_label_1772765275313_pgx4_copy_1772765661189_2,
                         hg_time_label_1772765275313_pgx4_copy_1772765661189_2_time_str,
                         strlen(hg_time_label_1772765275313_pgx4_copy_1772765661189_2_time_str));
}

/* @protected start custom_functions */
// Custom function
const char *month[12] =
{
    "JAN",
    "FEB",
    "MAR",
    "APR",
    "MAY",
    "JUN",
    "JUL",
    "AUG",
    "SEP",
    "OCT",
    "NOV",
    "DEC"
};
const char *day[7] =
{
    "SUN",
    "MON",
    "TUE",
    "WED",
    "THU",
    "FRI",
    "SAT"
};

char date_content[16] = "WED\nNOV\n19";
static char date_content_short[10] = "WED 19";
void text_date_big_timer_0_cb(void *obj)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }
    sprintf(date_content, "%s\n%s\n%d", day[t->tm_wday], month[t->tm_mon], t->tm_mday);
    gui_text_content_set((gui_text_t *)obj, date_content, strlen(date_content));
}
void text_date_small_timer_0_cb(void *obj)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(text_date_small_timer_cnt);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }
    sprintf(date_content_short, "%s %d", day[t->tm_wday], t->tm_mday);
    gui_text_content_set((gui_text_t *)obj, date_content_short, strlen(date_content_short));
}

void win_clock_big_timer_0_cb(void *obj)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(text_date_big_timer_cnt);

    if (list_card->offset < 93)
    {
        gui_obj_hidden((void *)win_clock_big, true);
        gui_obj_hidden((void *)win_clock_small, false);
        gui_win_enable_blur((void *)win_clock_small, true);
    }
    else
    {
        gui_obj_hidden((void *)win_clock_big, false);
        gui_obj_hidden((void *)win_clock_small, true);
        gui_win_enable_blur((void *)win_clock_small, false);
    }
}
/* @protected end custom_functions */
