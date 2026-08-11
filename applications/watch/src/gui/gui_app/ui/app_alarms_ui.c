/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_alarms UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-06T09:11:40.755Z
 */
#include "app_alarms_ui.h"
#include "../callbacks/app_alarms_callbacks.h"
#include "../user/app_alarms_user.h"
#include <stddef.h>
#include <time.h>

// Component handle definitions
gui_list_t *alarm_list = NULL;
gui_rounded_rect_t *alarm_wake_up_bg = NULL;
gui_text_t *alarm_wake_up_Timetext = NULL;
gui_text_t *alarm_wake_up_text = NULL;
gui_img_t *hg_image_1769670275206_mvfs = NULL;
gui_rounded_rect_t *alarm_alarm_bg = NULL;
gui_text_t *alarm_alarm_text = NULL;
gui_text_t *alarm_alarm_Timetext = NULL;
gui_obj_t *alarm_buttom = NULL;
gui_img_t *hg_image_1769670641672_8war = NULL;
gui_img_t *hg_image_1769670646462_ezb4 = NULL;
gui_win_t *alarm_top_window = NULL;
gui_text_t *hg_time_label_1769669369949_1hub = NULL;
gui_text_t *hg_label_1769669419002_bcvz = NULL;

// Time string global variables
char hg_time_label_1769669369949_1hub_time_str[10] = {0};

// Toggle button callback functions

// alarm_buttom dual-state button callback
static bool alarm_buttom_state = false;

void alarm_buttom_toggle_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    // Toggle state
    alarm_buttom_state = !alarm_buttom_state;

    // Switch image based on state and call corresponding callback
    if (alarm_buttom_state)
    {
        gui_img_set_src((gui_img_t *)alarm_buttom, (const uint8_t *)"/app_alarms/Switch_on_icon.bin",
                        IMG_SRC_FILESYS);
    }
    else
    {
        gui_img_set_src((gui_img_t *)alarm_buttom, (const uint8_t *)"/app_alarms/Switch_off_icon.bin",
                        IMG_SRC_FILESYS);
    }
    gui_fb_change();
}

// Get current state
bool alarm_buttom_get_state(void)
{
    return alarm_buttom_state;
}

// Set state (external call)
void alarm_buttom_set_state(bool state)
{
    if (alarm_buttom_state != state)
    {
        alarm_buttom_state = state;

        if (state)
        {
            gui_img_set_src((gui_img_t *)alarm_buttom, (const uint8_t *)"/app_alarms/Switch_on_icon.bin",
                            IMG_SRC_FILESYS);
        }
        else
        {
            gui_img_set_src((gui_img_t *)alarm_buttom, (const uint8_t *)"/app_alarms/Switch_off_icon.bin",
                            IMG_SRC_FILESYS);
        }
    }
}
// List component note_design callback functions
// note_design callback function declaration
static void alarm_list_note_design(gui_obj_t *obj, void *param);

// note_design callback function implementation
static void alarm_list_note_design(gui_obj_t *obj, void *param)
{
    GUI_UNUSED(param);

    // Cast obj to gui_list_note_t * type
    gui_list_note_t *note = (gui_list_note_t *)obj;
    uint16_t index = note->index;

    // Create different list_item content based on index
    switch (index)
    {
    case 0:
        {
            // Create alarm_wake_up_bg (hg_rect)
            alarm_wake_up_bg = gui_rect_create((gui_obj_t *)note, "alarm_wake_up_bg", 15, 0, 380, 144, 20,
                                               gui_rgb(44, 44, 46));
            // Create alarm_wake_up_Timetext (hg_label)
            alarm_wake_up_Timetext = gui_text_create((gui_obj_t *)note, "alarm_wake_up_Timetext", 37, 66, 245,
                                                     72);
            gui_text_set((gui_text_t *)alarm_wake_up_Timetext, "6:00 AM", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 7, 54);
            gui_text_type_set((gui_text_t *)alarm_wake_up_Timetext,
                              "/font/Inter_24pt_Regular_size54_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)alarm_wake_up_Timetext, LEFT);
            // Create alarm_wake_up_text (hg_label)
            alarm_wake_up_text = gui_text_create((gui_obj_t *)note, "alarm_wake_up_text", 123, 16, 200, 40);
            gui_text_set((gui_text_t *)alarm_wake_up_text, "Wake Up", GUI_FONT_SRC_BMP, gui_rgb(229, 148, 55),
                         7, 32);
            gui_text_type_set((gui_text_t *)alarm_wake_up_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)alarm_wake_up_text, LEFT);
            // Create hg_image_1769670275206_mvfs (hg_image)
            hg_image_1769670275206_mvfs = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1769670275206_mvfs", "/app_alarms/alarm_bed_icon.bin", 37, 10, 72, 46);
            break;
        }
    case 1:
        {
            // Create alarm_alarm_bg (hg_rect)
            alarm_alarm_bg = gui_rect_create((gui_obj_t *)note, "alarm_alarm_bg", 15, 0, 380, 144, 20,
                                             gui_rgb(44, 44, 46));
            // Create alarm_alarm_text (hg_label)
            alarm_alarm_text = gui_text_create((gui_obj_t *)note, "alarm_alarm_text", 37, 12, 154, 40);
            gui_text_set((gui_text_t *)alarm_alarm_text, "Alarm", GUI_FONT_SRC_BMP, gui_rgb(200, 200, 200), 5,
                         32);
            gui_text_type_set((gui_text_t *)alarm_alarm_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)alarm_alarm_text, LEFT);
            // Create alarm_alarm_Timetext (hg_label)
            alarm_alarm_Timetext = gui_text_create((gui_obj_t *)note, "alarm_alarm_Timetext", 37, 64, 246, 72);
            gui_text_set((gui_text_t *)alarm_alarm_Timetext, "3:00 PM", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 7, 54);
            gui_text_type_set((gui_text_t *)alarm_alarm_Timetext,
                              "/font/Inter_24pt_Regular_size54_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)alarm_alarm_Timetext, LEFT);
            // Create alarm_buttom (hg_button)
            alarm_buttom = (gui_obj_t *)gui_img_create_from_fs((gui_obj_t *)note, "alarm_buttom",
                                                               "/app_alarms/Switch_off_icon.bin", 314, 56, 72, 37);
            if (alarm_buttom_state)
            {
                gui_img_set_src((gui_img_t *)alarm_buttom, (const uint8_t *)"/app_alarms/Switch_on_icon.bin",
                                IMG_SRC_FILESYS);
            }
            gui_obj_add_event_cb((gui_obj_t *)alarm_buttom, alarm_buttom_toggle_cb, GUI_EVENT_TOUCH_CLICKED,
                                 NULL);
            break;
        }
    case 2:
        {
            // Create hg_image_1769670641672_8war (hg_image)
            hg_image_1769670641672_8war = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1769670641672_8war", "/app_phone/icon_bg.bin", 169, 36, 72, 72);
            // Create hg_image_1769670646462_ezb4 (hg_image)
            hg_image_1769670646462_ezb4 = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1769670646462_ezb4", "/app_phone/phone_add_icon.bin", 185, 52, 40, 40);
            break;
        }
    default:
        break;
    }
}


// Create app_alarm_view (hg_view)
static void app_alarm_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_alarm_view_switch_in(gui_view_t *view)
{
    // Set animation step
    gui_view_set_animate_step(view, 50);

    // Set opacity
    gui_view_set_opacity(view, 255);

    // Set background color
    gui_set_bg_color(gui_rgb(0, 0, 0));

    GUI_UNUSED(view);

    // Initialize time strings
    time_t now = time(NULL);
    struct tm *t = localtime(&now);



    // Create alarm_list (hg_list)
    alarm_list = gui_list_create((gui_obj_t *)view, "alarm_list", 0, 110, 410, 392, 144, 5, VERTICAL,
                                 alarm_list_note_design, NULL, false);
    gui_list_set_style(alarm_list, LIST_CLASSIC);
    gui_list_set_note_num(alarm_list, 3);
    gui_list_set_out_scope(alarm_list, 80);

    // Create alarm_top_window (hg_window)
    alarm_top_window = gui_win_create((gui_obj_t *)view, "alarm_top_window", 0, 0, 410, 110);
    gui_win_enable_blur((gui_win_t *)alarm_top_window, true);
    gui_win_set_blur_degree((gui_win_t *)alarm_top_window, 225);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(hg_time_label_1769669369949_1hub_time_str,
                 sizeof(hg_time_label_1769669369949_1hub_time_str), "%02d:%02d", t->tm_hour, t->tm_min);
    }


    // Create hg_time_label_1769669369949_1hub (hg_time_label)
    hg_time_label_1769669369949_1hub = gui_text_create(alarm_top_window,
                                                       "hg_time_label_1769669369949_1hub", 305, 20, 79, 32);
    gui_text_set((gui_text_t *)hg_time_label_1769669369949_1hub,
                 hg_time_label_1769669369949_1hub_time_str, GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                 strlen(hg_time_label_1769669369949_1hub_time_str), 28);
    gui_text_type_set((gui_text_t *)hg_time_label_1769669369949_1hub,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_time_label_1769669369949_1hub, RIGHT);

    // Create hg_label_1769669419002_bcvz (hg_label)
    hg_label_1769669419002_bcvz = gui_text_create(alarm_top_window, "hg_label_1769669419002_bcvz", 284,
                                                  61, 100, 42);
    gui_text_set((gui_text_t *)hg_label_1769669419002_bcvz, "Alarms", GUI_FONT_SRC_BMP, gui_rgb(229,
                 148, 55), 6, 28);
    gui_text_type_set((gui_text_t *)hg_label_1769669419002_bcvz,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_label_1769669419002_bcvz, RIGHT);

    gui_obj_add_event_cb(GUI_BASE(alarm_top_window), (gui_event_cb_t)alarm_top_window_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)alarm_top_window);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(hg_time_label_1769669369949_1hub), 30000, true,
                         hg_time_label_1769669369949_1hub_time_update_cb);
}
GUI_VIEW_INSTANCE("app_alarm_view", false, app_alarm_view_switch_in, app_alarm_view_switch_out,
                  false);
