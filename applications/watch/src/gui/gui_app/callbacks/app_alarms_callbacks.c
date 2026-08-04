/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_alarms_callbacks.h"
#include "../ui/app_alarms_ui.h"
#include "../user/app_alarms_user.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// Time string global variables (defined in UI file)
extern char hg_time_label_1769669369949_1hub_time_str[10];

// Event callback function implementations

void alarm_top_window_key_0_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    // Check key name
    if (strcmp(e->indev_name, "Home") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "SmartWatchTemplateMainView", SWITCH_INIT_STATE,
                               SWITCH_IN_ANIMATION_FADE);
    }
    else if (strcmp(e->indev_name, "Menu") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "app_menu_view", SWITCH_OUT_ANIMATION_FADE,
                               SWITCH_IN_ANIMATION_FADE);
    }
}

void hg_time_label_1769669369949_1hub_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(hg_time_label_1769669369949_1hub_time_str,
             sizeof(hg_time_label_1769669369949_1hub_time_str), "%02d:%02d", t->tm_hour, t->tm_min);

    gui_text_content_set((gui_text_t *)hg_time_label_1769669369949_1hub,
                         hg_time_label_1769669369949_1hub_time_str, strlen(hg_time_label_1769669369949_1hub_time_str));
}

// Toggle button state callback functions

/* USER CODE BEGIN alarm_buttom_on_callback */
/**
 * alarm_buttom ON state callback
 * Called when button switches to ON state
 */
void alarm_buttom_on_callback(void)
{
    // TODO: Implement ON state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_play();
}
/* USER CODE END alarm_buttom_on_callback */

/* USER CODE BEGIN alarm_buttom_off_callback */
/**
 * alarm_buttom OFF state callback
 * Called when button switches to OFF state
 */
void alarm_buttom_off_callback(void)
{
    // TODO: Implement OFF state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_pause();
}
/* USER CODE END alarm_buttom_off_callback */

/* @protected start custom_functions */
// Custom functions
/* @protected end custom_functions */
