/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_audio_book UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-06T09:11:40.758Z
 */
#include "app_audio_book_ui.h"
#include "../callbacks/app_audio_book_callbacks.h"
#include "../user/app_audio_book_user.h"
#include <stddef.h>
#include <time.h>

// Component handle definitions
gui_list_t *audio_book_main_list = NULL;
gui_rounded_rect_t *hg_rect_1770013258959_u987 = NULL;
gui_img_t *hg_image_1770013149823_nhbr = NULL;
gui_text_t *audiouBook_main_homepage_text = NULL;
gui_rounded_rect_t *hg_rect_1770013258959_u987_copy_1770013287522 = NULL;
gui_img_t *hg_image_1770013196825_t2ia = NULL;
gui_text_t *audiouBook_main_lib_text = NULL;
gui_win_t *audio_book_mian_window = NULL;
gui_text_t *audiouBook_main_time_text = NULL;
gui_text_t *audiouBook_main_titel_text = NULL;
gui_img_t *hg_image_1770013057275_aevg = NULL;

// Time string global variables
char audiouBook_main_time_text_time_str[10] = {0};

// List component note_design callback functions
// note_design callback function declaration
static void audio_book_main_list_note_design(gui_obj_t *obj, void *param);

// note_design callback function implementation
static void audio_book_main_list_note_design(gui_obj_t *obj, void *param)
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
            // Create hg_rect_1770013258959_u987 (hg_rect)
            hg_rect_1770013258959_u987 = gui_rect_create((gui_obj_t *)note, "hg_rect_1770013258959_u987", 0, 3,
                                                         352, 115, 20, gui_rgb(98, 101, 98));
            // Create hg_image_1770013149823_nhbr (hg_image)
            hg_image_1770013149823_nhbr = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1770013149823_nhbr", "/app_audio_book/audioBook_homepage_icon.bin", 22, 20, 80, 80);
            // Create audiouBook_main_homepage_text (hg_label)
            audiouBook_main_homepage_text = gui_text_create((gui_obj_t *)note, "audiouBook_main_homepage_text",
                                                            126, 39, 192, 55);
            gui_text_set((gui_text_t *)audiouBook_main_homepage_text, "Homepage", GUI_FONT_SRC_BMP, gui_rgb(255,
                         255, 255), 8, 36);
            gui_text_type_set((gui_text_t *)audiouBook_main_homepage_text,
                              "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)audiouBook_main_homepage_text, LEFT);
            break;
        }
    case 1:
        {
            // Create hg_rect_1770013258959_u987_copy_1770013287522 (hg_rect)
            hg_rect_1770013258959_u987_copy_1770013287522 = gui_rect_create((gui_obj_t *)note,
                                                                            "hg_rect_1770013258959_u987_copy_1770013287522", 0, 1, 352, 115, 20, gui_rgb(98, 101, 98));
            // Create hg_image_1770013196825_t2ia (hg_image)
            hg_image_1770013196825_t2ia = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1770013196825_t2ia", "/app_audio_book/audioBook_lib_icon.bin", 27, 22, 80, 72);
            // Create audiouBook_main_lib_text (hg_label)
            audiouBook_main_lib_text = gui_text_create((gui_obj_t *)note, "audiouBook_main_lib_text", 126, 39,
                                                       192, 55);
            gui_text_set((gui_text_t *)audiouBook_main_lib_text, "Library", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 7, 36);
            gui_text_type_set((gui_text_t *)audiouBook_main_lib_text,
                              "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)audiouBook_main_lib_text, LEFT);
            break;
        }
    default:
        break;
    }
}


// Create audioBook_view (hg_view)
static void audioBook_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void audioBook_view_switch_in(gui_view_t *view)
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



    // Create audio_book_main_list (hg_list)
    audio_book_main_list = gui_list_create((gui_obj_t *)view, "audio_book_main_list", 29, 115, 352, 387,
                                           115, 5, VERTICAL, audio_book_main_list_note_design, NULL, false);
    gui_list_set_style(audio_book_main_list, LIST_CLASSIC);
    gui_list_set_note_num(audio_book_main_list, 2);
    gui_list_set_out_scope(audio_book_main_list, 80);

    // Create audio_book_mian_window (hg_window)
    audio_book_mian_window = gui_win_create((gui_obj_t *)view, "audio_book_mian_window", 0, 0, 410,
                                            110);
    gui_win_enable_blur((gui_win_t *)audio_book_mian_window, true);
    gui_win_set_blur_degree((gui_win_t *)audio_book_mian_window, 225);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(audiouBook_main_time_text_time_str, sizeof(audiouBook_main_time_text_time_str),
                 "%02d:%02d", t->tm_hour, t->tm_min);
    }


    // Create audiouBook_main_time_text (hg_time_label)
    audiouBook_main_time_text = gui_text_create(audio_book_mian_window, "audiouBook_main_time_text",
                                                140, 20, 140, 60);
    gui_text_set((gui_text_t *)audiouBook_main_time_text, audiouBook_main_time_text_time_str,
                 GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), strlen(audiouBook_main_time_text_time_str), 48);
    gui_text_type_set((gui_text_t *)audiouBook_main_time_text,
                      "/font/Inter_24pt_Regular_size48_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)audiouBook_main_time_text, CENTER);

    // Create audiouBook_main_titel_text (hg_label)
    audiouBook_main_titel_text = gui_text_create(audio_book_mian_window, "audiouBook_main_titel_text",
                                                 130, 70, 160, 40);
    gui_text_set((gui_text_t *)audiouBook_main_titel_text, "Audio Book", GUI_FONT_SRC_BMP, gui_rgb(229,
                 148, 55), 10, 28);
    gui_text_type_set((gui_text_t *)audiouBook_main_titel_text,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)audiouBook_main_titel_text, CENTER);

    // Create hg_image_1770013057275_aevg (hg_image)
    hg_image_1770013057275_aevg = gui_img_create_from_fs(audio_book_mian_window,
                                                         "hg_image_1770013057275_aevg", "/app_music/music_ctr_icon.bin", 308, 7, 72, 73);

    gui_obj_add_event_cb(GUI_BASE(audio_book_mian_window),
                         (gui_event_cb_t)audio_book_mian_window_key_0_cb, GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)audio_book_mian_window);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(audiouBook_main_time_text), 30000, true,
                         audiouBook_main_time_text_time_update_cb);
}
GUI_VIEW_INSTANCE("audioBook_view", false, audioBook_view_switch_in, audioBook_view_switch_out,
                  false);
