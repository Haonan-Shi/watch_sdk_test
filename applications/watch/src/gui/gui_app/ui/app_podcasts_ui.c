/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_podcasts UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-06T09:11:40.926Z
 */
#include "app_podcasts_ui.h"
#include "../callbacks/app_podcasts_callbacks.h"
#include "../user/app_podcasts_user.h"
#include <stddef.h>
#include <time.h>

// Component handle definitions
gui_list_t *app_podcast_menu_list = NULL;
gui_rounded_rect_t *podcats_menu_tag0 = NULL;
gui_text_t *app_podcast_homepage_text = NULL;
gui_img_t *hg_image_1769566502906_2bxr = NULL;
gui_rounded_rect_t *podcats_menu_tag1 = NULL;
gui_text_t *app_podcast_menu_lib_text = NULL;
gui_img_t *hg_image_1769566513496_qv6d = NULL;
gui_rounded_rect_t *podcats_menu_tag2 = NULL;
gui_text_t *app_podcast_menu_search_text = NULL;
gui_img_t *hg_image_1769566525433_jtg5 = NULL;
gui_win_t *app_podcast_menu_window = NULL;
gui_img_t *hg_image_window_podcast_ctr = NULL;
gui_text_t *app_podcast_menu_text0 = NULL;
gui_text_t *app_podcast_menu_time_text = NULL;
gui_img_t *app_podcast_ctr_delete_icon = NULL;
gui_arc_t *app_podcast_ctr_arc = NULL;
gui_text_t *app_podcast_ctr_time_text = NULL;
gui_obj_t *app_podcast_ctr_button = NULL;
gui_img_t *app_podcast_cover_art = NULL;
gui_img_t *hg_image_1769564084391_reur = NULL;
gui_img_t *hg_image_1769564088845_muwb = NULL;
gui_text_t *hg_label_1769564186120_0wdb = NULL;
gui_text_t *hg_label_1769564290281_kezn = NULL;
gui_list_t *podcast_homepage_list = NULL;
gui_rounded_rect_t *hg_rect_1769592366392_pck6 = NULL;
gui_img_t *podcast_homepage_list0_covert = NULL;
gui_img_t *podcast_homepage_list0_iconbg = NULL;
gui_img_t *podcast_homepage_list0_icon = NULL;
gui_text_t *podcast_homepage_list0_text0 = NULL;
gui_text_t *podcast_homepage_list0_text1 = NULL;
gui_rounded_rect_t *podcast_homepage_list_bg1 = NULL;
gui_img_t *podcast_homepage_list1_iconbg = NULL;
gui_img_t *podcast_homepage_list1_icon = NULL;
gui_img_t *podcast_homepage_list1_covert = NULL;
gui_text_t *podcast_homepage_list1_text0 = NULL;
gui_text_t *podcast_homepage_list1_text1 = NULL;
gui_rounded_rect_t *podcast_homepage_list_bg2 = NULL;
gui_img_t *podcast_homepage_list2_covert = NULL;
gui_img_t *podcast_homepage_list2_iconbg = NULL;
gui_img_t *podcast_homepage_list2_icon = NULL;
gui_text_t *podcast_homepage_list2_text0 = NULL;
gui_text_t *podcast_homepage_list2_text1 = NULL;
gui_win_t *app_podcast_homepage_window = NULL;
gui_text_t *app_podcast_homepage_text0 = NULL;
gui_img_t *podcast_homepage_return_bg = NULL;
gui_img_t *podcast_homepage_return_icon = NULL;
gui_text_t *app_podcast_homepage_time_text = NULL;
gui_img_t *hg_image_window_podcast_homepage = NULL;

// Time string global variables
char app_podcast_menu_time_text_time_str[10] = {0};
char app_podcast_ctr_time_text_time_str[10] = {0};
char app_podcast_homepage_time_text_time_str[10] = {0};

// Toggle button callback functions

// app_podcast_ctr_button dual-state button callback
static bool app_podcast_ctr_button_state = false;

void app_podcast_ctr_button_toggle_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    // Toggle state
    app_podcast_ctr_button_state = !app_podcast_ctr_button_state;

    // Switch image based on state and call corresponding callback
    if (app_podcast_ctr_button_state)
    {
        gui_img_set_src((gui_img_t *)app_podcast_ctr_button,
                        (const uint8_t *)"/app_music/pause_button_icon.bin", IMG_SRC_FILESYS);
    }
    else
    {
        gui_img_set_src((gui_img_t *)app_podcast_ctr_button,
                        (const uint8_t *)"/app_music/start_button_icon.bin", IMG_SRC_FILESYS);
    }
    gui_fb_change();
}

// Get current state
bool app_podcast_ctr_button_get_state(void)
{
    return app_podcast_ctr_button_state;
}

// Set state (external call)
void app_podcast_ctr_button_set_state(bool state)
{
    if (app_podcast_ctr_button_state != state)
    {
        app_podcast_ctr_button_state = state;

        if (state)
        {
            gui_img_set_src((gui_img_t *)app_podcast_ctr_button,
                            (const uint8_t *)"/app_music/pause_button_icon.bin", IMG_SRC_FILESYS);
        }
        else
        {
            gui_img_set_src((gui_img_t *)app_podcast_ctr_button,
                            (const uint8_t *)"/app_music/start_button_icon.bin", IMG_SRC_FILESYS);
        }
    }
}
// List component note_design callback functions
// note_design callback function declaration
static void app_podcast_menu_list_note_design(gui_obj_t *obj, void *param);

// note_design callback function implementation
static void app_podcast_menu_list_note_design(gui_obj_t *obj, void *param)
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
            // Create podcats_menu_tag0 (hg_rect)
            podcats_menu_tag0 = gui_rect_create((gui_obj_t *)note, "podcats_menu_tag0", 0, 0, 352, 115, 30,
                                                gui_rgba(98, 101, 98, 200));
            gui_obj_add_event_cb(podcats_menu_tag0, (gui_event_cb_t)podcats_menu_tag0_clicked_cb,
                                 GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create app_podcast_homepage_text (hg_label)
            app_podcast_homepage_text = gui_text_create((gui_obj_t *)note, "app_podcast_homepage_text", 123, 41,
                                                        210, 60);
            gui_text_set((gui_text_t *)app_podcast_homepage_text, "Homepage", GUI_FONT_SRC_BMP, gui_rgb(255,
                         255, 255), 8, 36);
            gui_text_type_set((gui_text_t *)app_podcast_homepage_text,
                              "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)app_podcast_homepage_text, LEFT);
            // Create hg_image_1769566502906_2bxr (hg_image)
            hg_image_1769566502906_2bxr = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1769566502906_2bxr", "/app_podcast/podcast_homepage_icon.bin", 15, 18, 80, 80);
            break;
        }
    case 1:
        {
            // Create podcats_menu_tag1 (hg_rect)
            podcats_menu_tag1 = gui_rect_create((gui_obj_t *)note, "podcats_menu_tag1", 0, 0, 352, 115, 30,
                                                gui_rgba(98, 101, 98, 200));
            // Create app_podcast_menu_lib_text (hg_label)
            app_podcast_menu_lib_text = gui_text_create((gui_obj_t *)note, "app_podcast_menu_lib_text", 123, 41,
                                                        210, 60);
            gui_text_set((gui_text_t *)app_podcast_menu_lib_text, "Library", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 7, 36);
            gui_text_type_set((gui_text_t *)app_podcast_menu_lib_text,
                              "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)app_podcast_menu_lib_text, LEFT);
            // Create hg_image_1769566513496_qv6d (hg_image)
            hg_image_1769566513496_qv6d = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1769566513496_qv6d", "/app_podcast/podcast_lib_icon.bin", 15, 18, 80, 72);
            break;
        }
    case 2:
        {
            // Create podcats_menu_tag2 (hg_rect)
            podcats_menu_tag2 = gui_rect_create((gui_obj_t *)note, "podcats_menu_tag2", 0, 0, 352, 115, 30,
                                                gui_rgba(98, 101, 98, 200));
            // Create app_podcast_menu_search_text (hg_label)
            app_podcast_menu_search_text = gui_text_create((gui_obj_t *)note, "app_podcast_menu_search_text",
                                                           123, 41, 210, 60);
            gui_text_set((gui_text_t *)app_podcast_menu_search_text, "Search", GUI_FONT_SRC_BMP, gui_rgb(255,
                         255, 255), 6, 36);
            gui_text_type_set((gui_text_t *)app_podcast_menu_search_text,
                              "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)app_podcast_menu_search_text, LEFT);
            // Create hg_image_1769566525433_jtg5 (hg_image)
            hg_image_1769566525433_jtg5 = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1769566525433_jtg5", "/app_podcast/podcast_search_icon.bin", 15, 18, 80, 80);
            gui_img_scale((gui_img_t *)hg_image_1769566525433_jtg5, 0.800000f, 0.800000f);
            break;
        }
    default:
        break;
    }
}

// note_design callback function declaration
static void podcast_homepage_list_note_design(gui_obj_t *obj, void *param);

// note_design callback function implementation
static void podcast_homepage_list_note_design(gui_obj_t *obj, void *param)
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
            // Create hg_rect_1769592366392_pck6 (hg_rect)
            hg_rect_1769592366392_pck6 = gui_rect_create((gui_obj_t *)note, "hg_rect_1769592366392_pck6", 29, 0,
                                                         352, 250, 30, gui_rgb(208, 204, 203));
            gui_obj_add_event_cb(hg_rect_1769592366392_pck6,
                                 (gui_event_cb_t)hg_rect_1769592366392_pck6_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create podcast_homepage_list0_covert (hg_image)
            podcast_homepage_list0_covert = gui_img_create_from_fs((gui_obj_t *)note,
                                                                   "podcast_homepage_list0_covert", "/app_podcast/Cover_Art.bin", 42, 9, 213, 213);
            gui_img_scale((gui_img_t *)podcast_homepage_list0_covert, 0.500000f, 0.500000f);
            // Create podcast_homepage_list0_iconbg (hg_image)
            podcast_homepage_list0_iconbg = gui_img_create_from_fs((gui_obj_t *)note,
                                                                   "podcast_homepage_list0_iconbg", "/app_phone/icon_bg.bin", 295, 10, 72, 72);
            // Create podcast_homepage_list0_icon (hg_image)
            podcast_homepage_list0_icon = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "podcast_homepage_list0_icon", "/app_phone/phone_etc_icon.bin", 313, 41, 36, 9);
            // Create podcast_homepage_list0_text0 (hg_label)
            podcast_homepage_list0_text0 = gui_text_create((gui_obj_t *)note, "podcast_homepage_list0_text0",
                                                           42, 127, 320, 50);
            gui_text_set((gui_text_t *)podcast_homepage_list0_text0, "Tim Cook: what it ...", GUI_FONT_SRC_TTF,
                         gui_rgb(255, 255, 255), 21, 32);
            gui_text_type_set((gui_text_t *)podcast_homepage_list0_text0, "/font/Inter_24pt_Regular_vector.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)podcast_homepage_list0_text0, LEFT);
            // Create podcast_homepage_list0_text1 (hg_label)
            podcast_homepage_list0_text1 = gui_text_create((gui_obj_t *)note, "podcast_homepage_list0_text1",
                                                           42, 166, 320, 50);
            gui_text_set((gui_text_t *)podcast_homepage_list0_text1, "12:34 left", GUI_FONT_SRC_TTF,
                         gui_rgb(168, 168, 168), 10, 32);
            gui_text_type_set((gui_text_t *)podcast_homepage_list0_text1, "/font/Inter_24pt_Regular_vector.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)podcast_homepage_list0_text1, LEFT);
            break;
        }
    case 1:
        {
            // Create podcast_homepage_list_bg1 (hg_rect)
            podcast_homepage_list_bg1 = gui_rect_create((gui_obj_t *)note, "podcast_homepage_list_bg1", 29, 0,
                                                        352, 250, 30, gui_rgb(208, 204, 203));
            gui_obj_add_event_cb(podcast_homepage_list_bg1,
                                 (gui_event_cb_t)podcast_homepage_list_bg1_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create podcast_homepage_list1_iconbg (hg_image)
            podcast_homepage_list1_iconbg = gui_img_create_from_fs((gui_obj_t *)note,
                                                                   "podcast_homepage_list1_iconbg", "/app_phone/icon_bg.bin", 295, 10, 72, 72);
            // Create podcast_homepage_list1_icon (hg_image)
            podcast_homepage_list1_icon = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "podcast_homepage_list1_icon", "/app_phone/phone_etc_icon.bin", 313, 42, 36, 9);
            // Create podcast_homepage_list1_covert (hg_image)
            podcast_homepage_list1_covert = gui_img_create_from_fs((gui_obj_t *)note,
                                                                   "podcast_homepage_list1_covert", "/app_podcast/Cover_Art.bin", 42, 9, 213, 213);
            gui_img_scale((gui_img_t *)podcast_homepage_list1_covert, 0.500000f, 0.500000f);
            // Create podcast_homepage_list1_text0 (hg_label)
            podcast_homepage_list1_text0 = gui_text_create((gui_obj_t *)note, "podcast_homepage_list1_text0",
                                                           42, 127, 320, 50);
            gui_text_set((gui_text_t *)podcast_homepage_list1_text0, "Tim Cook: what it ...", GUI_FONT_SRC_TTF,
                         gui_rgb(255, 255, 255), 21, 32);
            gui_text_type_set((gui_text_t *)podcast_homepage_list1_text0, "/font/Inter_24pt_Regular_vector.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)podcast_homepage_list1_text0, LEFT);
            // Create podcast_homepage_list1_text1 (hg_label)
            podcast_homepage_list1_text1 = gui_text_create((gui_obj_t *)note, "podcast_homepage_list1_text1",
                                                           42, 166, 320, 50);
            gui_text_set((gui_text_t *)podcast_homepage_list1_text1, "12:34 left", GUI_FONT_SRC_TTF,
                         gui_rgb(168, 168, 168), 10, 32);
            gui_text_type_set((gui_text_t *)podcast_homepage_list1_text1, "/font/Inter_24pt_Regular_vector.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)podcast_homepage_list1_text1, LEFT);
            break;
        }
    case 2:
        {
            // Create podcast_homepage_list_bg2 (hg_rect)
            podcast_homepage_list_bg2 = gui_rect_create((gui_obj_t *)note, "podcast_homepage_list_bg2", 29, 0,
                                                        352, 250, 30, gui_rgb(208, 204, 203));
            gui_obj_add_event_cb(podcast_homepage_list_bg2,
                                 (gui_event_cb_t)podcast_homepage_list_bg2_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create podcast_homepage_list2_covert (hg_image)
            podcast_homepage_list2_covert = gui_img_create_from_fs((gui_obj_t *)note,
                                                                   "podcast_homepage_list2_covert", "/app_podcast/Cover_Art.bin", 42, 9, 213, 213);
            gui_img_scale((gui_img_t *)podcast_homepage_list2_covert, 0.500000f, 0.500000f);
            // Create podcast_homepage_list2_iconbg (hg_image)
            podcast_homepage_list2_iconbg = gui_img_create_from_fs((gui_obj_t *)note,
                                                                   "podcast_homepage_list2_iconbg", "/app_phone/icon_bg.bin", 295, 10, 72, 72);
            // Create podcast_homepage_list2_icon (hg_image)
            podcast_homepage_list2_icon = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "podcast_homepage_list2_icon", "/app_phone/phone_etc_icon.bin", 313, 42, 36, 9);
            // Create podcast_homepage_list2_text0 (hg_label)
            podcast_homepage_list2_text0 = gui_text_create((gui_obj_t *)note, "podcast_homepage_list2_text0",
                                                           47, 127, 320, 50);
            gui_text_set((gui_text_t *)podcast_homepage_list2_text0, "Tim Cook: what it ...", GUI_FONT_SRC_TTF,
                         gui_rgb(255, 255, 255), 21, 32);
            gui_text_type_set((gui_text_t *)podcast_homepage_list2_text0, "/font/Inter_24pt_Regular_vector.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)podcast_homepage_list2_text0, LEFT);
            // Create podcast_homepage_list2_text1 (hg_label)
            podcast_homepage_list2_text1 = gui_text_create((gui_obj_t *)note, "podcast_homepage_list2_text1",
                                                           42, 166, 320, 50);
            gui_text_set((gui_text_t *)podcast_homepage_list2_text1, "12:34 left", GUI_FONT_SRC_TTF,
                         gui_rgb(168, 168, 168), 10, 32);
            gui_text_type_set((gui_text_t *)podcast_homepage_list2_text1, "/font/Inter_24pt_Regular_vector.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)podcast_homepage_list2_text1, LEFT);
            break;
        }
    default:
        break;
    }
}


// Create app_podcast_view (hg_view)
static void app_podcast_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_podcast_view_switch_in(gui_view_t *view)
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



    // Create app_podcast_menu_list (hg_list)
    app_podcast_menu_list = gui_list_create((gui_obj_t *)view, "app_podcast_menu_list", 29, 115, 352,
                                            385, 115, 5, VERTICAL, app_podcast_menu_list_note_design, NULL, false);
    gui_list_set_style(app_podcast_menu_list, LIST_CLASSIC);
    gui_list_set_note_num(app_podcast_menu_list, 3);
    gui_list_set_out_scope(app_podcast_menu_list, 80);

    // Create app_podcast_menu_window (hg_window)
    app_podcast_menu_window = gui_win_create((gui_obj_t *)view, "app_podcast_menu_window", 0, 2, 410,
                                             115);
    gui_win_enable_blur((gui_win_t *)app_podcast_menu_window, true);
    gui_win_set_blur_degree((gui_win_t *)app_podcast_menu_window, 225);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(app_podcast_menu_time_text_time_str, sizeof(app_podcast_menu_time_text_time_str),
                 "%02d:%02d", t->tm_hour, t->tm_min);
    }


    // Create hg_image_window_podcast_ctr (hg_image)
    hg_image_window_podcast_ctr = gui_img_create_from_fs(app_podcast_menu_window,
                                                         "hg_image_window_podcast_ctr", "/app_music/music_ctr_icon.bin", 308, 7, 72, 73);
    gui_obj_add_event_cb(hg_image_window_podcast_ctr,
                         (gui_event_cb_t)hg_image_window_podcast_ctr_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create app_podcast_menu_text0 (hg_label)
    app_podcast_menu_text0 = gui_text_create(app_podcast_menu_window, "app_podcast_menu_text0", 124, 72,
                                             173, 40);
    gui_text_set((gui_text_t *)app_podcast_menu_text0, "Podcast", GUI_FONT_SRC_BMP, gui_rgb(106, 39,
                 208), 7, 28);
    gui_text_type_set((gui_text_t *)app_podcast_menu_text0,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)app_podcast_menu_text0, CENTER);

    // Create app_podcast_menu_time_text (hg_time_label)
    app_podcast_menu_time_text = gui_text_create(app_podcast_menu_window, "app_podcast_menu_time_text",
                                                 140, 20, 140, 60);
    gui_text_set((gui_text_t *)app_podcast_menu_time_text, app_podcast_menu_time_text_time_str,
                 GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), strlen(app_podcast_menu_time_text_time_str), 52);
    gui_text_type_set((gui_text_t *)app_podcast_menu_time_text,
                      "/font/Inter_24pt_Regular_size52_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)app_podcast_menu_time_text, CENTER);

    gui_obj_add_event_cb(GUI_BASE(app_podcast_menu_window),
                         (gui_event_cb_t)app_podcast_menu_window_key_0_cb, GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)app_podcast_menu_window);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(app_podcast_menu_time_text), 30000, true,
                         app_podcast_menu_time_text_time_update_cb);
}
GUI_VIEW_INSTANCE("app_podcast_view", false, app_podcast_view_switch_in,
                  app_podcast_view_switch_out, false);

// Create app_podcast_ctr_view (hg_view)
static void app_podcast_ctr_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_podcast_ctr_view_switch_in(gui_view_t *view)
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
        snprintf(app_podcast_ctr_time_text_time_str, sizeof(app_podcast_ctr_time_text_time_str),
                 "%02d:%02d", t->tm_hour, t->tm_min);
    }



    // Create app_podcast_ctr_delete_icon (hg_image)
    app_podcast_ctr_delete_icon = gui_img_create_from_fs((gui_obj_t *)view,
                                                         "app_podcast_ctr_delete_icon", "/app_music/delete_icon.bin", 25, 15, 72, 72);
    gui_obj_add_event_cb(app_podcast_ctr_delete_icon,
                         (gui_event_cb_t)app_podcast_ctr_delete_icon_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create app_podcast_ctr_arc (hg_arc)
    app_podcast_ctr_arc = gui_arc_create((gui_obj_t *)view, "app_podcast_ctr_arc", 205, 442, 40, 0, 360,
                                         3, gui_rgb(100, 100, 100));

    // Create app_podcast_ctr_time_text (hg_time_label)
    app_podcast_ctr_time_text = gui_text_create((gui_obj_t *)view, "app_podcast_ctr_time_text", 145, 20,
                                                138, 60);
    gui_text_set((gui_text_t *)app_podcast_ctr_time_text, app_podcast_ctr_time_text_time_str,
                 GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), strlen(app_podcast_ctr_time_text_time_str), 52);
    gui_text_type_set((gui_text_t *)app_podcast_ctr_time_text,
                      "/font/Inter_24pt_Regular_size52_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)app_podcast_ctr_time_text, CENTER);

    // Create app_podcast_ctr_button (hg_button)
    app_podcast_ctr_button = (gui_obj_t *)gui_img_create_from_fs((gui_obj_t *)view,
                                                                 "app_podcast_ctr_button", "/app_music/start_button_icon.bin", 189, 426, 36, 41);
    if (app_podcast_ctr_button_state)
    {
        gui_img_set_src((gui_img_t *)app_podcast_ctr_button,
                        (const uint8_t *)"/app_music/pause_button_icon.bin", IMG_SRC_FILESYS);
    }
    gui_obj_add_event_cb((gui_obj_t *)app_podcast_ctr_button, app_podcast_ctr_button_toggle_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create app_podcast_cover_art (hg_image)
    app_podcast_cover_art = gui_img_create_from_fs((gui_obj_t *)view, "app_podcast_cover_art",
                                                   "/app_podcast/Cover_Art.bin", 99, 109, 213, 213);
    gui_obj_add_event_cb(app_podcast_cover_art, (gui_event_cb_t)app_podcast_cover_art_key_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)app_podcast_cover_art);

    // Create hg_image_1769564084391_reur (hg_image)
    hg_image_1769564084391_reur = gui_img_create_from_fs((gui_obj_t *)view,
                                                         "hg_image_1769564084391_reur", "/app_podcast/ongoing_icon.bin", 309, 413, 36, 36);
    gui_img_scale((gui_img_t *)hg_image_1769564084391_reur, 2.000000f, 2.000000f);

    // Create hg_image_1769564088845_muwb (hg_image)
    hg_image_1769564088845_muwb = gui_img_create_from_fs((gui_obj_t *)view,
                                                         "hg_image_1769564088845_muwb", "/app_podcast/return15_icon.bin", 25, 413, 36, 36);
    gui_img_scale((gui_img_t *)hg_image_1769564088845_muwb, 2.000000f, 2.000000f);

    // Create hg_label_1769564186120_0wdb (hg_label)
    hg_label_1769564186120_0wdb = gui_text_create((gui_obj_t *)view, "hg_label_1769564186120_0wdb", 25,
                                                  313, 365, 40);
    gui_text_set((gui_text_t *)hg_label_1769564186120_0wdb, "Tim Cook: what it takes to",
                 GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 26, 30);
    gui_text_type_set((gui_text_t *)hg_label_1769564186120_0wdb,
                      "/font/Inter_24pt_Regular_size30_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_label_1769564186120_0wdb, LEFT);

    // Create hg_label_1769564290281_kezn (hg_label)
    hg_label_1769564290281_kezn = gui_text_create((gui_obj_t *)view, "hg_label_1769564290281_kezn", 164,
                                                  353, 100, 34);
    gui_text_set((gui_text_t *)hg_label_1769564290281_kezn, "12:34 left", GUI_FONT_SRC_BMP, gui_rgb(168,
                 168, 168), 10, 24);
    gui_text_type_set((gui_text_t *)hg_label_1769564290281_kezn,
                      "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_label_1769564290281_kezn, LEFT);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(app_podcast_ctr_time_text), 30000, true,
                         app_podcast_ctr_time_text_time_update_cb);
}
GUI_VIEW_INSTANCE("app_podcast_ctr_view", false, app_podcast_ctr_view_switch_in,
                  app_podcast_ctr_view_switch_out, false);

// Create app_podcast_homepage_view (hg_view)
static void app_podcast_homepage_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_podcast_homepage_view_switch_in(gui_view_t *view)
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



    // Create podcast_homepage_list (hg_list)
    podcast_homepage_list = gui_list_create((gui_obj_t *)view, "podcast_homepage_list", 0, 110, 410,
                                            392, 250, 5, VERTICAL, podcast_homepage_list_note_design, NULL, false);
    gui_list_set_style(podcast_homepage_list, LIST_ZOOM);
    gui_list_set_note_num(podcast_homepage_list, 3);
    gui_list_set_out_scope(podcast_homepage_list, 80);

    // Create app_podcast_homepage_window (hg_window)
    app_podcast_homepage_window = gui_win_create((gui_obj_t *)view, "app_podcast_homepage_window", -1,
                                                 2, 410, 110);
    gui_win_enable_blur((gui_win_t *)app_podcast_homepage_window, true);
    gui_win_set_blur_degree((gui_win_t *)app_podcast_homepage_window, 225);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(app_podcast_homepage_time_text_time_str, sizeof(app_podcast_homepage_time_text_time_str),
                 "%02d:%02d", t->tm_hour, t->tm_min);
    }


    // Create app_podcast_homepage_text0 (hg_label)
    app_podcast_homepage_text0 = gui_text_create(app_podcast_homepage_window,
                                                 "app_podcast_homepage_text0", 139, 72, 142, 40);
    gui_text_set((gui_text_t *)app_podcast_homepage_text0, "Homepage", GUI_FONT_SRC_BMP, gui_rgb(106,
                 39, 208), 8, 28);
    gui_text_type_set((gui_text_t *)app_podcast_homepage_text0,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)app_podcast_homepage_text0, CENTER);

    // Create podcast_homepage_return_bg (hg_image)
    podcast_homepage_return_bg = gui_img_create_from_fs(app_podcast_homepage_window,
                                                        "podcast_homepage_return_bg", "/app_phone/icon_bg.bin", 25, 12, 72, 72);
    gui_obj_add_event_cb(podcast_homepage_return_bg,
                         (gui_event_cb_t)podcast_homepage_return_bg_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create podcast_homepage_return_icon (hg_image)
    podcast_homepage_return_icon = gui_img_create_from_fs(app_podcast_homepage_window,
                                                          "podcast_homepage_return_icon", "/app_reminder/reminders_icon5.bin", 49, 25, 25, 47);

    // Create app_podcast_homepage_time_text (hg_time_label)
    app_podcast_homepage_time_text = gui_text_create(app_podcast_homepage_window,
                                                     "app_podcast_homepage_time_text", 140, 22, 140, 60);
    gui_text_set((gui_text_t *)app_podcast_homepage_time_text, app_podcast_homepage_time_text_time_str,
                 GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), strlen(app_podcast_homepage_time_text_time_str), 52);
    gui_text_type_set((gui_text_t *)app_podcast_homepage_time_text,
                      "/font/Inter_24pt_Regular_size52_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)app_podcast_homepage_time_text, CENTER);

    // Create hg_image_window_podcast_homepage (hg_image)
    hg_image_window_podcast_homepage = gui_img_create_from_fs(app_podcast_homepage_window,
                                                              "hg_image_window_podcast_homepage", "/app_music/music_ctr_icon.bin", 308, 9, 72, 73);
    gui_obj_add_event_cb(hg_image_window_podcast_homepage,
                         (gui_event_cb_t)hg_image_window_podcast_homepage_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    gui_obj_add_event_cb(GUI_BASE(app_podcast_homepage_window),
                         (gui_event_cb_t)app_podcast_homepage_window_key_0_cb, GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)app_podcast_homepage_window);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(app_podcast_homepage_time_text), 30000, true,
                         app_podcast_homepage_time_text_time_update_cb);
}
GUI_VIEW_INSTANCE("app_podcast_homepage_view", false, app_podcast_homepage_view_switch_in,
                  app_podcast_homepage_view_switch_out, false);
