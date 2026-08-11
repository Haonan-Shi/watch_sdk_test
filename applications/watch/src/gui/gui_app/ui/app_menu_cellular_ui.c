/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_menu_cellular UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-06T09:11:40.856Z
 */
#include "app_menu_cellular_ui.h"
#include "../callbacks/app_menu_cellular_callbacks.h"
#include "../user/app_menu_cellular_user.h"
#include <stddef.h>

// Component handle definitions
gui_menu_cellular_t *menu_cellular = NULL;

// menu_cellular menu cellular icon switch view callbacks
void menu_cellular_icon_5_switch_view_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_phoneDialerView", SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
}

void menu_cellular_icon_24_switch_view_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_recordingMainView", SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
}

void menu_cellular_icon_25_switch_view_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_video_callIdleView", SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
}


// Create app_menu_cellular_view (hg_view)
static void app_menu_cellular_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_menu_cellular_view_switch_in(gui_view_t *view)
{
    // Set animation step
    gui_view_set_animate_step(view, 50);

    // Set opacity
    gui_view_set_opacity(view, 255);

    // Set background color
    gui_set_bg_color(gui_rgb(0, 0, 0));

    GUI_UNUSED(view);


    // Create menu_cellular (hg_menu_cellular)
    void *menu_cellular_icons[] =
    {
        "/UI_app_list/app_activity_icon.bin",
        "/UI_app_list/app_heart_rate_icon.bin",
        "/UI_app_list/app_weather_icon.bin",
        "/UI_app_list/app_reminders_icon.bin",
        "/UI_app_list/app_message_icon.bin",
        "/UI_app_list/app_phone_icon.bin",
        "/UI_app_list/app_noise_icon.bin",
        "/UI_app_list/app_music_icon.bin",
        "/UI_app_list/app_map_icon.bin",
        "/UI_app_list/app_stopwatch_icon.bin",
        "/UI_app_list/app_timers_icon.bin",
        "/UI_app_list/app_alarm_clock_icon.bin",
        "/UI_app_list/app_audio_books_icon.bin",
        "/UI_app_list/app_workout_icon.bin",
        "/UI_app_list/app_compass_icon.bin",
        "/UI_app_list/app_calendar_icon.bin",
        "/UI_app_list/app_home_icon.bin",
        "/UI_app_list/app_contacts_icon.bin",
        "/UI_app_list/music_scan_icon.bin",
        "/UI_app_list/app_now_playing_icon.bin",
        "/UI_app_list/app_news_icon.bin",
        "/UI_app_list/app_photo_icon.bin",
        "/UI_app_list/app_podcasts_icon.bin",
        "/UI_app_list/app_sleep_icon.bin",
        "/UI_app_list/recording_icon.bin",
        "/app_video_call/video_call_btn.bin"
    };
    menu_cellular = gui_menu_cellular_create((gui_obj_t *)view, 100, menu_cellular_icons, 26,
                                             IMG_SRC_FILESYS);
    static struct gui_menu_cellular_gesture_parameter menu_cellular_params[] =
    {
        { NULL, NULL },
        { NULL, NULL },
        { NULL, NULL },
        { NULL, NULL },
        { NULL, NULL },
        { menu_cellular_icon_5_switch_view_cb, NULL },
        { NULL, NULL },
        { NULL, NULL },
        { NULL, NULL },
        { NULL, NULL },
        { NULL, NULL },
        { NULL, NULL },
        { NULL, NULL },
        { NULL, NULL },
        { NULL, NULL },
        { NULL, NULL },
        { NULL, NULL },
        { NULL, NULL },
        { NULL, NULL },
        { NULL, NULL },
        { NULL, NULL },
        { NULL, NULL },
        { NULL, NULL },
        { NULL, NULL },
        { menu_cellular_icon_24_switch_view_cb, NULL },
        { menu_cellular_icon_25_switch_view_cb, NULL }
    };
    gui_menu_cellular_on_click(menu_cellular, menu_cellular_params, 26);

    gui_obj_add_event_cb((gui_obj_t *)view, (gui_event_cb_t)app_menu_cellular_view_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)view);
}
GUI_VIEW_INSTANCE("app_menu_cellular_view", false, app_menu_cellular_view_switch_in,
                  app_menu_cellular_view_switch_out, false);
