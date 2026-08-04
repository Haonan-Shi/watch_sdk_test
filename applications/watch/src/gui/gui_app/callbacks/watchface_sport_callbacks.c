/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "watchface_sport_callbacks.h"
#include "../ui/watchface_sport_ui.h"
#include "../user/watchface_sport_user.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// Time string global variables (defined in UI file)
extern char hg_time_label_1768897114762_8ilu_time_str[10];

// Event callback function implementations

void hg_image_1766555014041_zggx_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    // TODO: Implement event handling logic
}

void hg_arc_1766556753455_tg7q_msg_cb_0(gui_obj_t *obj, const char *topic, void *data, uint16_t len)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(topic);
    GUI_UNUSED(data);
    GUI_UNUSED(len);
    // TODO: Implement message handling logic
}

void hg_time_label_1768897114762_8ilu_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(hg_time_label_1768897114762_8ilu_time_str,
             sizeof(hg_time_label_1768897114762_8ilu_time_str), "%02d:%02d:%02d", t->tm_hour, t->tm_min,
             t->tm_sec);

    gui_text_content_set((gui_text_t *)hg_time_label_1768897114762_8ilu,
                         hg_time_label_1768897114762_8ilu_time_str, strlen(hg_time_label_1768897114762_8ilu_time_str));
}

/* @protected start custom_functions */
// Custom functions
/* @protected end custom_functions */
