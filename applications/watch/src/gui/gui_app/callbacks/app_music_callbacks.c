/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_music_callbacks.h"
#include "../ui/app_music_ui.h"
#include "../user/app_music_user.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// Time string global variables (defined in UI file)
extern char music_time_main_text_time_str[10];
extern char music_time_ctr_text_time_str[10];

// Event callback function implementations

void hg_window_1768286822421_xea9_key_0_cb(void *obj, gui_event_t *e)
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

void hg_image_window_music_ctr_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_music_ctr_view", SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
}

void hg_image_1768283673697_8ra5_key_cb(void *obj, gui_event_t *e)
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

void hg_image_1768283673697_8ra5_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_music_view", SWITCH_OUT_TO_LEFT_USE_TRANSLATION,
                           SWITCH_IN_FROM_RIGHT_USE_TRANSLATION);
}

void hg_image_1768283680575_lql6_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    app_music_play_prev(obj, e);
}

void hg_image_1768283684092_idby_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    app_music_play_next(obj, e);
}

void music_time_ctr_text_key_cb(void *obj, gui_event_t *e)
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

void music_time_main_text_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(music_time_main_text_time_str, sizeof(music_time_main_text_time_str), "%02d:%02d",
             t->tm_hour, t->tm_min);

    gui_text_content_set((gui_text_t *)music_time_main_text, music_time_main_text_time_str,
                         strlen(music_time_main_text_time_str));
}

void music_time_ctr_text_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(music_time_ctr_text_time_str, sizeof(music_time_ctr_text_time_str), "%02d:%02d",
             t->tm_hour, t->tm_min);

    gui_text_content_set((gui_text_t *)music_time_ctr_text, music_time_ctr_text_time_str,
                         strlen(music_time_ctr_text_time_str));
}

// Toggle button state callback functions

/* USER CODE BEGIN hg_button_1768981147980_ml83_on_callback */
/**
 * hg_button_1768981147980_ml83 ON state callback
 * Called when button switches to ON state
 */
void hg_button_1768981147980_ml83_on_callback(void)
{
    // TODO: Implement ON state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_play();
}
/* USER CODE END hg_button_1768981147980_ml83_on_callback */

/* USER CODE BEGIN hg_button_1768981147980_ml83_off_callback */
/**
 * hg_button_1768981147980_ml83 OFF state callback
 * Called when button switches to OFF state
 */
void hg_button_1768981147980_ml83_off_callback(void)
{
    // TODO: Implement OFF state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_pause();
}
/* USER CODE END hg_button_1768981147980_ml83_off_callback */

/* @protected start custom_functions */
// Custom functions
/* @protected end custom_functions */
