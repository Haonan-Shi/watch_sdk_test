/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_heart UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-06T09:11:40.823Z
 */
#include "app_heart_ui.h"
#include "../callbacks/app_heart_callbacks.h"
#include "../user/app_heart_user.h"
#include <stddef.h>
#include <time.h>

// Component handle definitions
gui_list_t *list_heart = NULL;
gui_img_t *hg_image_1769146380658_kvde = NULL;
gui_text_t *app_heart_c_text = NULL;
gui_text_t *app_heart_data_text = NULL;
gui_text_t *app_heart_data_text_copy_1769065570320 = NULL;
gui_rounded_rect_t *line12am = NULL;
gui_rounded_rect_t *line6am = NULL;
gui_rounded_rect_t *line12pm = NULL;
gui_rounded_rect_t *line6pm = NULL;
gui_rounded_rect_t *line_r = NULL;
gui_circle_t *app_heart_s_dot0 = NULL;
gui_circle_t *app_heart_s_dot1 = NULL;
gui_circle_t *app_heart_s_dot2 = NULL;
gui_circle_t *app_heart_s_dot3 = NULL;
gui_circle_t *app_heart_s_dot5 = NULL;
gui_text_t *app_heart_s_range_text = NULL;
gui_text_t *app_heart_s_range_text_copy_1769069630628 = NULL;
gui_text_t *app_heart_s_range_text_copy_1769069662567 = NULL;
gui_text_t *app_heart_s_range_text_copy_1769069694861 = NULL;
gui_circle_t *app_heart_s_dot4 = NULL;
gui_circle_t *app_heart_s_dot6 = NULL;
gui_circle_t *app_heart_s_dot7 = NULL;
gui_circle_t *app_heart_s_dot8 = NULL;
gui_circle_t *app_heart_s_dot9 = NULL;
gui_text_t *app_heart_s_range_text_copy_1769133405677 = NULL;
gui_text_t *app_heart_s_range_text_copy_1769133405677_copy_1769133430637 = NULL;
gui_text_t *app_heart_s_range_text_copy_1769133405677_copy_1769133430637_copy_1769133463877 = NULL;
gui_text_t
*app_heart_s_range_text_copy_1769133405677_copy_1769133430637_copy_1769133463877_copy_1769133509461
    = NULL;
gui_rounded_rect_t *hg_rect_data1 = NULL;
gui_rounded_rect_t *hg_rect_data0 = NULL;
gui_rounded_rect_t *hg_rect_data3 = NULL;
gui_text_t *app_heart_s_range_low_text = NULL;
gui_circle_t *app_heart_s_dot8_copy_1769134137402 = NULL;
gui_text_t *app_heart_s_range_hight_text = NULL;
gui_img_t *hg_image_1769066727540_xtif = NULL;
gui_text_t *app_heart_s_total_text = NULL;
gui_win_t *app_heart_window = NULL;
gui_text_t *hg_time_label_heart = NULL;
gui_circle_t *app_heart_circel0 = NULL;
gui_circle_t *app_heart_circel1 = NULL;

// Time string global variables
char hg_time_label_heart_time_str[10] = {0};

// List component note_design callback functions
// note_design callback function declaration
static void list_heart_note_design(gui_obj_t *obj, void *param);

// note_design callback function implementation
static void list_heart_note_design(gui_obj_t *obj, void *param)
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
            // Create hg_image_1769146380658_kvde (hg_image)
            hg_image_1769146380658_kvde = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1769146380658_kvde", "/heart/Heart.bin", 131, 126, 148, 144);
            gui_img_translate((gui_img_t *)hg_image_1769146380658_kvde, 148.0f, 144.0f);
            gui_img_set_focus((gui_img_t *)hg_image_1769146380658_kvde, 74.0f, 72.0f);

            // Bind timer: Timer animation 1
            gui_obj_create_timer((gui_obj_t *)hg_image_1769146380658_kvde, 100, true,
                                 hg_image_1769146380658_kvde_timer_0_cb);
            gui_obj_start_timer((gui_obj_t *)hg_image_1769146380658_kvde);
            // Create app_heart_c_text (hg_label)
            app_heart_c_text = gui_text_create((gui_obj_t *)note, "app_heart_c_text", 37, 327, 150, 50);
            gui_text_set((gui_text_t *)app_heart_c_text, "Current", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 7,
                         30);
            gui_text_type_set((gui_text_t *)app_heart_c_text,
                              "/font/Inter_24pt_Regular_size30_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)app_heart_c_text, LEFT);
            // Create app_heart_data_text (hg_label)
            app_heart_data_text = gui_text_create((gui_obj_t *)note, "app_heart_data_text", 37, 369, 150, 60);
            gui_text_set((gui_text_t *)app_heart_data_text, "78 BMP", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                         6, 42);
            gui_text_type_set((gui_text_t *)app_heart_data_text,
                              "/font/Inter_24pt_Regular_size42_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)app_heart_data_text, LEFT);
            // Create app_heart_data_text_copy_1769065570320 (hg_label)
            app_heart_data_text_copy_1769065570320 = gui_text_create((gui_obj_t *)note,
                                                                     "app_heart_data_text_copy_1769065570320", 37, 428, 280, 40);
            gui_text_set((gui_text_t *)app_heart_data_text_copy_1769065570320, "73 BMP just now",
                         GUI_FONT_SRC_BMP, gui_rgb(150, 150, 150), 15, 30);
            gui_text_type_set((gui_text_t *)app_heart_data_text_copy_1769065570320,
                              "/font/Inter_24pt_Regular_size30_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)app_heart_data_text_copy_1769065570320, LEFT);
            break;
        }
    case 1:
        {
            // Create line12am (hg_rect)
            line12am = gui_rect_create((gui_obj_t *)note, "line12am", 32, 104, 3, 200, 0, gui_rgb(255, 255,
                                       255));
            // Create line6am (hg_rect)
            line6am = gui_rect_create((gui_obj_t *)note, "line6am", 116, 104, 3, 200, 0, gui_rgb(255, 255,
                                      255));
            // Create line12pm (hg_rect)
            line12pm = gui_rect_create((gui_obj_t *)note, "line12pm", 204, 104, 3, 200, 0, gui_rgb(255, 255,
                                       255));
            // Create line6pm (hg_rect)
            line6pm = gui_rect_create((gui_obj_t *)note, "line6pm", 289, 104, 3, 200, 0, gui_rgb(255, 255,
                                      255));
            // Create line_r (hg_rect)
            line_r = gui_rect_create((gui_obj_t *)note, "line_r", 374, 104, 3, 200, 0, gui_rgb(255, 255, 255));
            // Create app_heart_s_dot0 (hg_circle)
            app_heart_s_dot0 = gui_circle_create((gui_obj_t *)note, "app_heart_s_dot0", 140, 202, 5,
                                                 gui_rgb(204, 0, 41));
            // Create app_heart_s_dot1 (hg_circle)
            app_heart_s_dot1 = gui_circle_create((gui_obj_t *)note, "app_heart_s_dot1", 155, 218, 5,
                                                 gui_rgb(204, 0, 41));
            // Create app_heart_s_dot2 (hg_circle)
            app_heart_s_dot2 = gui_circle_create((gui_obj_t *)note, "app_heart_s_dot2", 140, 182, 5,
                                                 gui_rgb(204, 0, 41));
            // Create app_heart_s_dot3 (hg_circle)
            app_heart_s_dot3 = gui_circle_create((gui_obj_t *)note, "app_heart_s_dot3", 140, 218, 5,
                                                 gui_rgb(204, 0, 41));
            // Create app_heart_s_dot5 (hg_circle)
            app_heart_s_dot5 = gui_circle_create((gui_obj_t *)note, "app_heart_s_dot5", 145, 162, 5,
                                                 gui_rgb(204, 0, 41));
            // Create app_heart_s_range_text (hg_label)
            app_heart_s_range_text = gui_text_create((gui_obj_t *)note, "app_heart_s_range_text", 25, 346, 125,
                                                     50);
            gui_text_set((gui_text_t *)app_heart_s_range_text, "range", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 5, 32);
            gui_text_type_set((gui_text_t *)app_heart_s_range_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)app_heart_s_range_text, LEFT);
            // Create app_heart_s_range_text_copy_1769069630628 (hg_label)
            app_heart_s_range_text_copy_1769069630628 = gui_text_create((gui_obj_t *)note,
                                                                        "app_heart_s_range_text_copy_1769069630628", 25, 387, 150, 63);
            gui_text_set((gui_text_t *)app_heart_s_range_text_copy_1769069630628, "63-110", GUI_FONT_SRC_BMP,
                         gui_rgb(255, 255, 255), 6, 42);
            gui_text_type_set((gui_text_t *)app_heart_s_range_text_copy_1769069630628,
                              "/font/Inter_24pt_Regular_size42_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)app_heart_s_range_text_copy_1769069630628, LEFT);
            // Create app_heart_s_range_text_copy_1769069662567 (hg_label)
            app_heart_s_range_text_copy_1769069662567 = gui_text_create((gui_obj_t *)note,
                                                                        "app_heart_s_range_text_copy_1769069662567", 164, 397, 125, 40);
            gui_text_set((gui_text_t *)app_heart_s_range_text_copy_1769069662567, "BMP", GUI_FONT_SRC_BMP,
                         gui_rgb(204, 0, 41), 3, 28);
            gui_text_type_set((gui_text_t *)app_heart_s_range_text_copy_1769069662567,
                              "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)app_heart_s_range_text_copy_1769069662567, LEFT);
            // Create app_heart_s_range_text_copy_1769069694861 (hg_label)
            app_heart_s_range_text_copy_1769069694861 = gui_text_create((gui_obj_t *)note,
                                                                        "app_heart_s_range_text_copy_1769069694861", 25, 438, 125, 50);
            gui_text_set((gui_text_t *)app_heart_s_range_text_copy_1769069694861, "Today", GUI_FONT_SRC_BMP,
                         gui_rgb(200, 200, 200), 5, 32);
            gui_text_type_set((gui_text_t *)app_heart_s_range_text_copy_1769069694861,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)app_heart_s_range_text_copy_1769069694861, LEFT);
            // Create app_heart_s_dot4 (hg_circle)
            app_heart_s_dot4 = gui_circle_create((gui_obj_t *)note, "app_heart_s_dot4", 165, 182, 5,
                                                 gui_rgb(204, 0, 41));
            // Create app_heart_s_dot6 (hg_circle)
            app_heart_s_dot6 = gui_circle_create((gui_obj_t *)note, "app_heart_s_dot6", 145, 204, 5,
                                                 gui_rgb(204, 0, 41));
            // Create app_heart_s_dot7 (hg_circle)
            app_heart_s_dot7 = gui_circle_create((gui_obj_t *)note, "app_heart_s_dot7", 185, 202, 5,
                                                 gui_rgb(204, 0, 41));
            // Create app_heart_s_dot8 (hg_circle)
            app_heart_s_dot8 = gui_circle_create((gui_obj_t *)note, "app_heart_s_dot8", 185, 218, 5,
                                                 gui_rgb(204, 0, 41));
            // Create app_heart_s_dot9 (hg_circle)
            app_heart_s_dot9 = gui_circle_create((gui_obj_t *)note, "app_heart_s_dot9", 226, 218, 5,
                                                 gui_rgb(204, 0, 41));
            // Create app_heart_s_range_text_copy_1769133405677 (hg_label)
            app_heart_s_range_text_copy_1769133405677 = gui_text_create((gui_obj_t *)note,
                                                                        "app_heart_s_range_text_copy_1769133405677", 36, 275, 40, 38);
            gui_text_set((gui_text_t *)app_heart_s_range_text_copy_1769133405677, "0", GUI_FONT_SRC_BMP,
                         gui_rgb(200, 200, 200), 1, 24);
            gui_text_type_set((gui_text_t *)app_heart_s_range_text_copy_1769133405677,
                              "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)app_heart_s_range_text_copy_1769133405677, LEFT);
            // Create app_heart_s_range_text_copy_1769133405677_copy_1769133430637 (hg_label)
            app_heart_s_range_text_copy_1769133405677_copy_1769133430637 = gui_text_create((gui_obj_t *)note,
                                                                                           "app_heart_s_range_text_copy_1769133405677_copy_1769133430637", 120, 275, 40, 38);
            gui_text_set((gui_text_t *)app_heart_s_range_text_copy_1769133405677_copy_1769133430637, "6",
                         GUI_FONT_SRC_BMP, gui_rgb(200, 200, 200), 1, 24);
            gui_text_type_set((gui_text_t *)app_heart_s_range_text_copy_1769133405677_copy_1769133430637,
                              "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)app_heart_s_range_text_copy_1769133405677_copy_1769133430637, LEFT);
            // Create app_heart_s_range_text_copy_1769133405677_copy_1769133430637_copy_1769133463877 (hg_label)
            app_heart_s_range_text_copy_1769133405677_copy_1769133430637_copy_1769133463877 = gui_text_create((
                        gui_obj_t *)note, "app_heart_s_range_text_copy_1769133405677_copy_1769133430637_copy_1769133463877",
                    206, 275, 40, 38);
            gui_text_set((gui_text_t *)
                         app_heart_s_range_text_copy_1769133405677_copy_1769133430637_copy_1769133463877, "12",
                         GUI_FONT_SRC_BMP, gui_rgb(200, 200, 200), 2, 24);
            gui_text_type_set((gui_text_t *)
                              app_heart_s_range_text_copy_1769133405677_copy_1769133430637_copy_1769133463877,
                              "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)
                              app_heart_s_range_text_copy_1769133405677_copy_1769133430637_copy_1769133463877, LEFT);
            // Create app_heart_s_range_text_copy_1769133405677_copy_1769133430637_copy_1769133463877_copy_1769133509461 (hg_label)
            app_heart_s_range_text_copy_1769133405677_copy_1769133430637_copy_1769133463877_copy_1769133509461 =
                gui_text_create((gui_obj_t *)note,
                                "app_heart_s_range_text_copy_1769133405677_copy_1769133430637_copy_1769133463877_copy_1769133509461",
                                293, 275, 40, 38);
            gui_text_set((gui_text_t *)
                         app_heart_s_range_text_copy_1769133405677_copy_1769133430637_copy_1769133463877_copy_1769133509461,
                         "18", GUI_FONT_SRC_BMP, gui_rgb(200, 200, 200), 2, 24);
            gui_text_type_set((gui_text_t *)
                              app_heart_s_range_text_copy_1769133405677_copy_1769133430637_copy_1769133463877_copy_1769133509461,
                              "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)
                              app_heart_s_range_text_copy_1769133405677_copy_1769133430637_copy_1769133463877_copy_1769133509461,
                              LEFT);
            // Create hg_rect_data1 (hg_rect)
            hg_rect_data1 = gui_rect_create((gui_obj_t *)note, "hg_rect_data1", 221, 180, 10, 15, 5,
                                            gui_rgb(204, 0, 41));
            // Create hg_rect_data0 (hg_rect)
            hg_rect_data0 = gui_rect_create((gui_obj_t *)note, "hg_rect_data0", 236, 170, 10, 25, 5,
                                            gui_rgb(204, 0, 41));
            // Create hg_rect_data3 (hg_rect)
            hg_rect_data3 = gui_rect_create((gui_obj_t *)note, "hg_rect_data3", 188, 162, 10, 25, 5,
                                            gui_rgb(204, 0, 41));
            // Create app_heart_s_range_low_text (hg_label)
            app_heart_s_range_low_text = gui_text_create((gui_obj_t *)note, "app_heart_s_range_low_text", 342,
                                                         237, 30, 38);
            gui_text_set((gui_text_t *)app_heart_s_range_low_text, "44", GUI_FONT_SRC_BMP, gui_rgb(204, 0, 41),
                         2, 24);
            gui_text_type_set((gui_text_t *)app_heart_s_range_low_text,
                              "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)app_heart_s_range_low_text, LEFT);
            // Create app_heart_s_dot8_copy_1769134137402 (hg_circle)
            app_heart_s_dot8_copy_1769134137402 = gui_circle_create((gui_obj_t *)note,
                                                                    "app_heart_s_dot8_copy_1769134137402", 140, 251, 5, gui_rgb(204, 0, 41));
            // Create app_heart_s_range_hight_text (hg_label)
            app_heart_s_range_hight_text = gui_text_create((gui_obj_t *)note, "app_heart_s_range_hight_text",
                                                           333, 123, 40, 38);
            gui_text_set((gui_text_t *)app_heart_s_range_hight_text, "114", GUI_FONT_SRC_BMP, gui_rgb(204, 0,
                         41), 3, 24);
            gui_text_type_set((gui_text_t *)app_heart_s_range_hight_text,
                              "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)app_heart_s_range_hight_text, LEFT);
            // Create hg_image_1769066727540_xtif (hg_image)
            hg_image_1769066727540_xtif = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1769066727540_xtif", "/heart/icon_information.bin", 20, 10, 72, 72);
            // Create app_heart_s_total_text (hg_label)
            app_heart_s_total_text = gui_text_create((gui_obj_t *)note, "app_heart_s_total_text", 275, 51, 125,
                                                     42);
            gui_text_set((gui_text_t *)app_heart_s_total_text, "heart rate", GUI_FONT_SRC_BMP, gui_rgb(204, 0,
                         41), 10, 24);
            gui_text_type_set((gui_text_t *)app_heart_s_total_text,
                              "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)app_heart_s_total_text, RIGHT);
            break;
        }
    default:
        break;
    }
}


// Create app_heart_view (hg_view)
static void app_heart_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_heart_view_switch_in(gui_view_t *view)
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



    // Create list_heart (hg_list)
    list_heart = gui_list_create((gui_obj_t *)view, "list_heart", 0, 0, 410, 502, 502, 0, VERTICAL,
                                 list_heart_note_design, NULL, false);
    gui_list_set_style(list_heart, LIST_FADE);
    gui_list_set_note_num(list_heart, 2);
    gui_list_set_auto_align(list_heart, true);
    gui_list_set_inertia(list_heart, false);

    // Create app_heart_window (hg_window)
    app_heart_window = gui_win_create((gui_obj_t *)view, "app_heart_window", 0, 0, 410, 110);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(hg_time_label_heart_time_str, sizeof(hg_time_label_heart_time_str), "%02d:%02d",
                 t->tm_hour, t->tm_min);
    }


    // Create hg_time_label_heart (hg_time_label)
    hg_time_label_heart = gui_text_create(app_heart_window, "hg_time_label_heart", 287, 20, 100, 32);
    gui_text_set((gui_text_t *)hg_time_label_heart, hg_time_label_heart_time_str, GUI_FONT_SRC_BMP,
                 gui_rgb(255, 255, 255), strlen(hg_time_label_heart_time_str), 28);
    gui_text_type_set((gui_text_t *)hg_time_label_heart,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_time_label_heart, RIGHT);

    gui_obj_add_event_cb(GUI_BASE(app_heart_window), (gui_event_cb_t)app_heart_window_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)app_heart_window);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(hg_time_label_heart), 30000, true,
                         hg_time_label_heart_time_update_cb);

    // Create app_heart_circel0 (hg_circle)
    app_heart_circel0 = gui_circle_create((gui_obj_t *)view, "app_heart_circel0", 392, 95, 5,
                                          gui_rgb(255, 255, 255));
    // Bind timer: Animation 1
    gui_obj_create_timer((gui_obj_t *)app_heart_circel0, 10, true, app_heart_circel0_timer_0_cb);
    gui_obj_start_timer((gui_obj_t *)app_heart_circel0);

    // Create app_heart_circel1 (hg_circle)
    app_heart_circel1 = gui_circle_create((gui_obj_t *)view, "app_heart_circel1", 392, 116, 5,
                                          gui_rgb(66, 62, 62));
}
GUI_VIEW_INSTANCE("app_heart_view", false, app_heart_view_switch_in, app_heart_view_switch_out,
                  false);
