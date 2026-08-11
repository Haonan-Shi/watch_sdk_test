/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_home_callbacks.h"
#include "../ui/app_home_ui.h"
#include "../user/app_home_user.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// Time string global variables (defined in UI file)
extern char hg_time_label_1770174775962_9onz_time_str[10];

// Event callback function implementations

void home_window_key_0_cb(void *obj, gui_event_t *e)
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

void hg_time_label_1770174775962_9onz_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(hg_time_label_1770174775962_9onz_time_str,
             sizeof(hg_time_label_1770174775962_9onz_time_str), "%02d:%02d", t->tm_hour, t->tm_min);

    gui_text_content_set((gui_text_t *)hg_time_label_1770174775962_9onz,
                         hg_time_label_1770174775962_9onz_time_str, strlen(hg_time_label_1770174775962_9onz_time_str));
}

/* @protected start custom_functions */
// Custom functions
/* @protected end custom_functions */
