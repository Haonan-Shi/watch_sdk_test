/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_ota_user.h"
#include "../ui/app_ota_ui.h"
#include "gui_arc.h"
#include "gui_text.h"
#include "gui_view.h"
#include "gui_obj.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* === State === */
static int ota_progress = 0;       /* 0~1000 (x10 for integer precision, 1000 = 100%) */
static int ota_result_delay = 0;   /* Countdown ticks (100ms each) for post-100% delay */

/* === Helpers === */

static void update_progress_ui(void)
{
    int pct = ota_progress / 10;

    gui_text_t *pct_label = ota_updating_percent;
    if (pct_label)
    {
        static char buf[16];
        snprintf(buf, sizeof(buf), "%d", pct);
        // gui_text_set(pct_label, buf, GUI_FONT_SRC_BMP, gui_rgb(0xFF, 0xFF, 0xFF),
        //              strlen(buf), 54);
        // gui_text_type_set(pct_label, "/font/Font_size54_bits4_bitmap.bin", FONT_SRC_FILESYS);
        // gui_text_mode_set(pct_label, MID_CENTER);
        gui_text_content_set(pct_label, buf, strlen(buf));
    }

    gui_arc_t *ring = ota_updating_ring;
    if (ring)
    {
        gui_obj_hidden((gui_obj_t *)ring, !(pct > 0));
        if (pct > 0)
        {
            float end_angle = -90.0f + 360.0f * pct / 100.0f;
            gui_arc_set_end_angle(ring, end_angle);
        }
    }

    gui_fb_change();
}

/* === Timer callbacks === */

void ota_starting_timer_cb_impl(void)
{
    ota_progress = 0;
    ota_result_delay = 0;
    gui_view_switch_direct(gui_view_get_current(), "app_otaUpdatingView",
                           SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
}

void ota_progress_tick_cb_impl(void)
{
    /* Phase 2: post-100% delay (500ms = 10 ticks of 50ms) */
    if (ota_result_delay > 0)
    {
        ota_result_delay--;
        if (ota_result_delay == 0)
        {
            if ((rand() % 100) < 85)
            {
                gui_view_switch_direct(gui_view_get_current(), "app_otaSuccessView",
                                       SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
            }
            else
            {
                gui_view_switch_direct(gui_view_get_current(), "app_otaFailedView",
                                       SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
            }
        }
        return;
    }

    /* Phase 1: progress increment */
    if (ota_progress >= 1000)
    {
        return;
    }

    int increment = (rand() % 5) + 2;  /* 0.2~0.6% in x10 scale, smoother steps */
    ota_progress += increment;

    if (ota_progress >= 1000)
    {
        ota_progress = 1000;
        update_progress_ui();
        ota_result_delay = 10;  /* Start 500ms delay (10 ticks × 50ms) */
        return;
    }

    update_progress_ui();
}

/* === Event callbacks === */

void ota_retry(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    ota_progress = 0;
    ota_result_delay = 0;
    gui_view_switch_direct(gui_view_get_current(), "app_otaStartingView",
                           SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
}

void ota_reset_to_ready(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    ota_progress = 0;
    ota_result_delay = 0;
    gui_view_switch_direct(gui_view_get_current(), "app_otaReadyView",
                           SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
}
