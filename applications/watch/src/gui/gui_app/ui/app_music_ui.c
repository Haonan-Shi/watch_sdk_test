/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_music UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-06T09:11:40.865Z
 */
#include "app_music_ui.h"
#include "../callbacks/app_music_callbacks.h"
#include "../user/app_music_user.h"
#include <stddef.h>
#include <time.h>

// Component handle definitions
gui_list_t *hg_list_1768278317813_ni8t = NULL;
gui_rounded_rect_t *hg_rect_1768278391413_rzwv = NULL;
gui_img_t *hg_image_1768278459365_n8mq = NULL;
gui_text_t *music_homepage_text = NULL;
gui_rounded_rect_t *hg_rect_1768278391413_rzwv_copy_1768278443156 = NULL;
gui_img_t *hg_image_1768278470059_6op4 = NULL;
gui_text_t *music_broadcast_text = NULL;
gui_rounded_rect_t *hg_rect_1768278391413_rzwv_copy_1768278443156_copy_1768278446755 = NULL;
gui_img_t *hg_image_1768278479908_yvdh = NULL;
gui_text_t *music_lib_text = NULL;
gui_win_t *hg_window_1768286822421_xea9 = NULL;
gui_img_t *hg_image_window_music_ctr = NULL;
gui_text_t *hg_label_window_music = NULL;
gui_text_t *music_time_main_text = NULL;
gui_img_t *hg_image_1768283633747_exth = NULL;
gui_img_t *hg_image_1768283673697_8ra5 = NULL;
gui_img_t *hg_image_1768283680575_lql6 = NULL;
gui_img_t *hg_image_1768283684092_idby = NULL;
gui_arc_t *hg_arc_1768283890950_hrp0 = NULL;
gui_text_t *music_time_ctr_text = NULL;
gui_obj_t *hg_button_1768981147980_ml83 = NULL;

// Time string global variables
char music_time_main_text_time_str[10] = {0};
char music_time_ctr_text_time_str[10] = {0};

// Toggle button callback functions

// hg_button_1768981147980_ml83 dual-state button callback
static bool hg_button_1768981147980_ml83_state = false;

void hg_button_1768981147980_ml83_toggle_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    // Toggle state
    hg_button_1768981147980_ml83_state = !hg_button_1768981147980_ml83_state;

    // Switch image based on state and call corresponding callback
    if (hg_button_1768981147980_ml83_state)
    {
        gui_img_set_src((gui_img_t *)hg_button_1768981147980_ml83,
                        (const uint8_t *)"/app_music/pause_button_icon.bin", IMG_SRC_FILESYS);
        extern void app_music_play(void *obj, gui_event_t *e);
        app_music_play(obj, e);
    }
    else
    {
        gui_img_set_src((gui_img_t *)hg_button_1768981147980_ml83,
                        (const uint8_t *)"/app_music/start_button_icon.bin", IMG_SRC_FILESYS);
        extern void app_music_play(void *obj, gui_event_t *e);
        app_music_pause(obj, e);
    }
    gui_fb_change();
}

// Get current state
bool hg_button_1768981147980_ml83_get_state(void)
{
    return hg_button_1768981147980_ml83_state;
}

// Set state (external call)
void hg_button_1768981147980_ml83_set_state(bool state)
{
    if (hg_button_1768981147980_ml83_state != state)
    {
        hg_button_1768981147980_ml83_state = state;

        if (state)
        {
            gui_img_set_src((gui_img_t *)hg_button_1768981147980_ml83,
                            (const uint8_t *)"/app_music/pause_button_icon.bin", IMG_SRC_FILESYS);
            extern void app_music_play(void *obj, gui_event_t *e);
            app_music_play(NULL, NULL);
        }
        else
        {
            gui_img_set_src((gui_img_t *)hg_button_1768981147980_ml83,
                            (const uint8_t *)"/app_music/start_button_icon.bin", IMG_SRC_FILESYS);
            extern void app_music_play(void *obj, gui_event_t *e);
            app_music_pause(NULL, NULL);
        }
    }
}
// List component note_design callback functions
// note_design callback function declaration
static void hg_list_1768278317813_ni8t_note_design(gui_obj_t *obj, void *param);

// note_design callback function implementation
static void hg_list_1768278317813_ni8t_note_design(gui_obj_t *obj, void *param)
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
            // Create hg_rect_1768278391413_rzwv (hg_rect)
            hg_rect_1768278391413_rzwv = gui_rect_create((gui_obj_t *)note, "hg_rect_1768278391413_rzwv", 0, 0,
                                                         352, 115, 30, gui_rgba(98, 101, 98, 200));
            // Create hg_image_1768278459365_n8mq (hg_image)
            hg_image_1768278459365_n8mq = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1768278459365_n8mq", "/app_music/music_homepage.bin", 0, 14, 100, 87);
            // Create music_homepage_text (hg_label)
            music_homepage_text = gui_text_create((gui_obj_t *)note, "music_homepage_text", 123, 41, 210, 60);
            gui_text_set((gui_text_t *)music_homepage_text, "Homepage", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 8, 36);
            gui_text_type_set((gui_text_t *)music_homepage_text,
                              "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)music_homepage_text, LEFT);
            break;
        }
    case 1:
        {
            // Create hg_rect_1768278391413_rzwv_copy_1768278443156 (hg_rect)
            hg_rect_1768278391413_rzwv_copy_1768278443156 = gui_rect_create((gui_obj_t *)note,
                                                                            "hg_rect_1768278391413_rzwv_copy_1768278443156", 0, 0, 352, 115, 30, gui_rgba(98, 101, 98, 200));
            // Create hg_image_1768278470059_6op4 (hg_image)
            hg_image_1768278470059_6op4 = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1768278470059_6op4", "/app_music/music_broadcast_icon.bin", 10, 8, 100, 100);
            // Create music_broadcast_text (hg_label)
            music_broadcast_text = gui_text_create((gui_obj_t *)note, "music_broadcast_text", 123, 38, 210, 60);
            gui_text_set((gui_text_t *)music_broadcast_text, "Broadcast", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 9, 36);
            gui_text_type_set((gui_text_t *)music_broadcast_text,
                              "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)music_broadcast_text, LEFT);
            break;
        }
    case 2:
        {
            // Create hg_rect_1768278391413_rzwv_copy_1768278443156_copy_1768278446755 (hg_rect)
            hg_rect_1768278391413_rzwv_copy_1768278443156_copy_1768278446755 = gui_rect_create((
                                                                                   gui_obj_t *)note, "hg_rect_1768278391413_rzwv_copy_1768278443156_copy_1768278446755", 0, 0, 352,
                                                                               115, 30, gui_rgba(98, 101, 98, 200));
            // Create hg_image_1768278479908_yvdh (hg_image)
            hg_image_1768278479908_yvdh = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1768278479908_yvdh", "/app_music/music_lib.bin", 15, 15, 100, 116);
            gui_img_scale((gui_img_t *)hg_image_1768278479908_yvdh, 0.700000f, 0.700000f);
            // Create music_lib_text (hg_label)
            music_lib_text = gui_text_create((gui_obj_t *)note, "music_lib_text", 123, 43, 210, 60);
            gui_text_set((gui_text_t *)music_lib_text, "Library", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 7,
                         36);
            gui_text_type_set((gui_text_t *)music_lib_text, "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)music_lib_text, LEFT);
            break;
        }
    default:
        break;
    }
}


// Create app_music_view (hg_view)
static void app_music_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_music_view_switch_in(gui_view_t *view)
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



    // Create hg_list_1768278317813_ni8t (hg_list)
    hg_list_1768278317813_ni8t = gui_list_create((gui_obj_t *)view, "hg_list_1768278317813_ni8t", 29,
                                                 115, 352, 385, 115, 5, VERTICAL, hg_list_1768278317813_ni8t_note_design, NULL, false);
    gui_list_set_style(hg_list_1768278317813_ni8t, LIST_CLASSIC);
    gui_list_set_note_num(hg_list_1768278317813_ni8t, 3);
    gui_list_set_out_scope(hg_list_1768278317813_ni8t, 80);

    // Create hg_window_1768286822421_xea9 (hg_window)
    hg_window_1768286822421_xea9 = gui_win_create((gui_obj_t *)view, "hg_window_1768286822421_xea9", 0,
                                                  2, 410, 115);
    gui_win_enable_blur((gui_win_t *)hg_window_1768286822421_xea9, true);
    gui_win_set_blur_degree((gui_win_t *)hg_window_1768286822421_xea9, 225);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(music_time_main_text_time_str, sizeof(music_time_main_text_time_str), "%02d:%02d",
                 t->tm_hour, t->tm_min);
    }


    // Create hg_image_window_music_ctr (hg_image)
    hg_image_window_music_ctr = gui_img_create_from_fs(hg_window_1768286822421_xea9,
                                                       "hg_image_window_music_ctr", "/app_music/music_ctr_icon.bin", 308, 7, 72, 73);
    gui_obj_add_event_cb(hg_image_window_music_ctr,
                         (gui_event_cb_t)hg_image_window_music_ctr_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create hg_label_window_music (hg_label)
    hg_label_window_music = gui_text_create(hg_window_1768286822421_xea9, "hg_label_window_music", 160,
                                            69, 100, 40);
    gui_text_set((gui_text_t *)hg_label_window_music, "Music", GUI_FONT_SRC_BMP, gui_rgb(228, 28, 64),
                 5, 26);
    gui_text_type_set((gui_text_t *)hg_label_window_music,
                      "/font/Inter_24pt_Regular_size26_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_label_window_music, MID_CENTER);

    // Create music_time_main_text (hg_time_label)
    music_time_main_text = gui_text_create(hg_window_1768286822421_xea9, "music_time_main_text", 140,
                                           20, 140, 60);
    gui_text_set((gui_text_t *)music_time_main_text, music_time_main_text_time_str, GUI_FONT_SRC_BMP,
                 gui_rgb(255, 255, 255), strlen(music_time_main_text_time_str), 52);
    gui_text_type_set((gui_text_t *)music_time_main_text,
                      "/font/Inter_24pt_Regular_size52_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)music_time_main_text, CENTER);

    gui_obj_add_event_cb(GUI_BASE(hg_window_1768286822421_xea9),
                         (gui_event_cb_t)hg_window_1768286822421_xea9_key_0_cb, GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)hg_window_1768286822421_xea9);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(music_time_main_text), 30000, true,
                         music_time_main_text_time_update_cb);
}
GUI_VIEW_INSTANCE("app_music_view", false, app_music_view_switch_in, app_music_view_switch_out,
                  false);

// Create app_music_ctr_view (hg_view)
static void app_music_ctr_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_music_ctr_view_switch_in(gui_view_t *view)
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
    if (t != NULL)
    {
        snprintf(music_time_ctr_text_time_str, sizeof(music_time_ctr_text_time_str), "%02d:%02d",
                 t->tm_hour, t->tm_min);
    }



    // Create hg_image_1768283633747_exth (hg_image)
    hg_image_1768283633747_exth = gui_img_create_from_fs((gui_obj_t *)view,
                                                         "hg_image_1768283633747_exth", "/app_music/Rectangle86.bin", 88, 104, 300, 300);
    gui_img_scale((gui_img_t *)hg_image_1768283633747_exth, 0.700000f, 0.700000f);

    // Create hg_image_1768283673697_8ra5 (hg_image)
    hg_image_1768283673697_8ra5 = gui_img_create_from_fs((gui_obj_t *)view,
                                                         "hg_image_1768283673697_8ra5", "/app_music/delete_icon.bin", 25, 15, 72, 72);
    gui_obj_add_event_cb(hg_image_1768283673697_8ra5,
                         (gui_event_cb_t)hg_image_1768283673697_8ra5_key_cb, GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_add_event_cb(hg_image_1768283673697_8ra5,
                         (gui_event_cb_t)hg_image_1768283673697_8ra5_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
    gui_obj_focus_set((gui_obj_t *)hg_image_1768283673697_8ra5);

    // Create hg_image_1768283680575_lql6 (hg_image)
    hg_image_1768283680575_lql6 = gui_img_create_from_fs((gui_obj_t *)view,
                                                         "hg_image_1768283680575_lql6", "/app_music/music_pre.bin", 44, 406, 72, 72);
    gui_obj_add_event_cb(hg_image_1768283680575_lql6,
                         (gui_event_cb_t)hg_image_1768283680575_lql6_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create hg_image_1768283684092_idby (hg_image)
    hg_image_1768283684092_idby = gui_img_create_from_fs((gui_obj_t *)view,
                                                         "hg_image_1768283684092_idby", "/app_music/music_next.bin", 296, 406, 72, 72);
    gui_obj_add_event_cb(hg_image_1768283684092_idby,
                         (gui_event_cb_t)hg_image_1768283684092_idby_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create hg_arc_1768283890950_hrp0 (hg_arc)
    hg_arc_1768283890950_hrp0 = gui_arc_create((gui_obj_t *)view, "hg_arc_1768283890950_hrp0", 205, 442,
                                               40, 0, 360, 3, gui_rgb(100, 100, 100));

    // Create music_time_ctr_text (hg_time_label)
    music_time_ctr_text = gui_text_create((gui_obj_t *)view, "music_time_ctr_text", 145, 20, 138, 60);
    gui_text_set((gui_text_t *)music_time_ctr_text, music_time_ctr_text_time_str, GUI_FONT_SRC_BMP,
                 gui_rgb(255, 255, 255), strlen(music_time_ctr_text_time_str), 52);
    gui_text_type_set((gui_text_t *)music_time_ctr_text,
                      "/font/Inter_24pt_Regular_size52_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)music_time_ctr_text, CENTER);
    gui_obj_add_event_cb(music_time_ctr_text, (gui_event_cb_t)music_time_ctr_text_key_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)music_time_ctr_text);

    // Create hg_button_1768981147980_ml83 (hg_button)
    hg_button_1768981147980_ml83 = (gui_obj_t *)gui_img_create_from_fs((gui_obj_t *)view,
                                                                       "hg_button_1768981147980_ml83", "/app_music/start_button_icon.bin", 187, 422, 36, 41);
    if (hg_button_1768981147980_ml83_state)
    {
        gui_img_set_src((gui_img_t *)hg_button_1768981147980_ml83,
                        (const uint8_t *)"/app_music/pause_button_icon.bin", IMG_SRC_FILESYS);
    }
    gui_obj_add_event_cb((gui_obj_t *)hg_button_1768981147980_ml83,
                         hg_button_1768981147980_ml83_toggle_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(music_time_ctr_text), 30000, true,
                         music_time_ctr_text_time_update_cb);
}
GUI_VIEW_INSTANCE("app_music_ctr_view", false, app_music_ctr_view_switch_in,
                  app_music_ctr_view_switch_out, false);
