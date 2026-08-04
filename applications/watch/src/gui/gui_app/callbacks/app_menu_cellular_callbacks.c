/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_menu_cellular_callbacks.h"
#include "../ui/app_menu_cellular_ui.h"
#include "../user/app_menu_cellular_user.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// Event callback function implementations

void app_menu_cellular_view_key_0_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    // Check key name
    if (strcmp(e->indev_name, "Home") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "SmartWatchTemplateMainView",
                               SWITCH_OUT_NONE_ANIMATION, SWITCH_IN_ANIMATION_FADE);
    }
}

/* @protected start custom_functions */
// Custom functions
/* @protected end custom_functions */
