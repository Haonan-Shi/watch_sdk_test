/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_workout UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-06T09:11:41.013Z
 */
#include "app_workout_ui.h"
#include "../callbacks/app_workout_callbacks.h"
#include "../user/app_workout_user.h"
#include <stddef.h>
#include <time.h>

// Component handle definitions
gui_rounded_rect_t *app_workout_bg = NULL;
gui_list_t *app_workout_menu_list = NULL;
gui_img_t *workout_badminton_icon = NULL;
gui_text_t *workout_badminton_text = NULL;
gui_img_t *workout_basketball_icon = NULL;
gui_text_t *workout_basketball_text = NULL;
gui_img_t *workout_bicycle_icon = NULL;
gui_text_t *workout_bicycle_text = NULL;
gui_img_t *workout_dance_icon = NULL;
gui_text_t *workout_dance_text = NULL;
gui_img_t *workout_indoor_run_icon = NULL;
gui_text_t *workout_indoor_run_text = NULL;
gui_img_t *workout_indoor_walk_icon = NULL;
gui_text_t *workout_indoor_walk_text = NULL;
gui_img_t *workout_jump_rope_icon = NULL;
gui_text_t *workout_jump_rope_text = NULL;
gui_img_t *workout_on_foot_icon = NULL;
gui_text_t *workout_on_foot_text = NULL;
gui_img_t *workout_outdoor_climb_icon = NULL;
gui_text_t *workout_outdoor_climb_text = NULL;
gui_img_t *workout_outdoor_run_icon = NULL;
gui_text_t *workout_outdoor_run_text = NULL;
gui_img_t *workout_outdoor_walk_icon = NULL;
gui_text_t *workout_outdoor_walk_text = NULL;
gui_img_t *workout_sit_up_icon = NULL;
gui_text_t *workout_sit_up_text = NULL;
gui_img_t *workout_stretch_icon = NULL;
gui_text_t *workout_stretch_text = NULL;
gui_img_t *workout_swim_icon = NULL;
gui_text_t *workout_swim_text = NULL;
gui_img_t *workout_taekwondo_icon = NULL;
gui_text_t *workout_taekwondo_text = NULL;
gui_img_t *workout_weightlifte_icon = NULL;
gui_text_t *workout_weightlifte_text = NULL;
gui_img_t *workout_yoga_icon = NULL;
gui_text_t *workout_yoga_text = NULL;
gui_circle_t *workout_mode_add_bg = NULL;
gui_img_t *hg_image_1769492435275_x96q = NULL;
gui_win_t *app_workout_menu_topwindow = NULL;
gui_img_t *app_workout_icon_bg0 = NULL;
gui_img_t *app_workout_icon0 = NULL;
gui_img_t *app_workout_icon_bg1 = NULL;
gui_img_t *app_workout_icon1 = NULL;
gui_text_t *app_workout_time_text = NULL;
gui_win_t *app_workout_menu_bottomwindow = NULL;
gui_img_t *app_workout_start_icon = NULL;
gui_img_t *app_workout_icon_bg2 = NULL;
gui_img_t *app_workout_icon2 = NULL;
gui_img_t *app_workout_icon_bg3 = NULL;
gui_img_t *app_workout_icon3 = NULL;
gui_rounded_rect_t *app_workout_start_engine_bg = NULL;
gui_arc_t *workout_countdown_arc = NULL;
gui_rounded_rect_t *app_workout_remind_bg = NULL;
gui_text_t *hg_time_label_1769406976200_a8x9 = NULL;
gui_img_t *hg_image_1769407041833_6sxz = NULL;
gui_img_t *hg_image_1769407067590_35cr = NULL;

// Time string global variables
char app_workout_time_text_time_str[10] = {0};
char hg_time_label_1769406976200_a8x9_time_str[10] = {0};

// List component note_design callback functions
// note_design callback function declaration
static void app_workout_menu_list_note_design(gui_obj_t *obj, void *param);

// note_design callback function implementation
static void app_workout_menu_list_note_design(gui_obj_t *obj, void *param)
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
            // Create workout_badminton_icon (hg_image)
            workout_badminton_icon = gui_img_create_from_fs((gui_obj_t *)note, "workout_badminton_icon",
                                                            "/app_workout/workout_badminton_icon.bin", 140, 85, 130, 130);
            // Create workout_badminton_text (hg_label)
            workout_badminton_text = gui_text_create((gui_obj_t *)note, "workout_badminton_text", 0, 208, 410,
                                                     42);
            gui_text_set((gui_text_t *)workout_badminton_text, "badminton", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 9, 32);
            gui_text_type_set((gui_text_t *)workout_badminton_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)workout_badminton_text, CENTER);
            break;
        }
    case 1:
        {
            // Create workout_basketball_icon (hg_image)
            workout_basketball_icon = gui_img_create_from_fs((gui_obj_t *)note, "workout_basketball_icon",
                                                             "/app_workout/workout_basketball_icon.bin", 140, 85, 130, 130);
            // Create workout_basketball_text (hg_label)
            workout_basketball_text = gui_text_create((gui_obj_t *)note, "workout_basketball_text", 0, 225, 410,
                                                      42);
            gui_text_set((gui_text_t *)workout_basketball_text, "basketball", GUI_FONT_SRC_BMP, gui_rgb(255,
                         255, 255), 10, 32);
            gui_text_type_set((gui_text_t *)workout_basketball_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)workout_basketball_text, CENTER);
            break;
        }
    case 2:
        {
            // Create workout_bicycle_icon (hg_image)
            workout_bicycle_icon = gui_img_create_from_fs((gui_obj_t *)note, "workout_bicycle_icon",
                                                          "/app_workout/workout_bicycle_icon.bin", 140, 85, 130, 130);
            // Create workout_bicycle_text (hg_label)
            workout_bicycle_text = gui_text_create((gui_obj_t *)note, "workout_bicycle_text", 0, 211, 410, 42);
            gui_text_set((gui_text_t *)workout_bicycle_text, "bicycle", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 7, 32);
            gui_text_type_set((gui_text_t *)workout_bicycle_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)workout_bicycle_text, CENTER);
            break;
        }
    case 3:
        {
            // Create workout_dance_icon (hg_image)
            workout_dance_icon = gui_img_create_from_fs((gui_obj_t *)note, "workout_dance_icon",
                                                        "/app_workout/workout_dance_icon.bin", 140, 85, 130, 130);
            // Create workout_dance_text (hg_label)
            workout_dance_text = gui_text_create((gui_obj_t *)note, "workout_dance_text", 0, 222, 410, 42);
            gui_text_set((gui_text_t *)workout_dance_text, "dance", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                         32);
            gui_text_type_set((gui_text_t *)workout_dance_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)workout_dance_text, CENTER);
            break;
        }
    case 4:
        {
            // Create workout_indoor_run_icon (hg_image)
            workout_indoor_run_icon = gui_img_create_from_fs((gui_obj_t *)note, "workout_indoor_run_icon",
                                                             "/app_workout/workout_indoor_run_icon.bin", 140, 85, 130, 130);
            // Create workout_indoor_run_text (hg_label)
            workout_indoor_run_text = gui_text_create((gui_obj_t *)note, "workout_indoor_run_text", 0, 238, 410,
                                                      42);
            gui_text_set((gui_text_t *)workout_indoor_run_text, "indoor run", GUI_FONT_SRC_BMP, gui_rgb(255,
                         255, 255), 10, 32);
            gui_text_type_set((gui_text_t *)workout_indoor_run_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)workout_indoor_run_text, CENTER);
            break;
        }
    case 5:
        {
            // Create workout_indoor_walk_icon (hg_image)
            workout_indoor_walk_icon = gui_img_create_from_fs((gui_obj_t *)note, "workout_indoor_walk_icon",
                                                              "/app_workout/workout_indoor_walk.bin", 140, 85, 130, 130);
            // Create workout_indoor_walk_text (hg_label)
            workout_indoor_walk_text = gui_text_create((gui_obj_t *)note, "workout_indoor_walk_text", 0, 211,
                                                       410, 42);
            gui_text_set((gui_text_t *)workout_indoor_walk_text, "indoor walk", GUI_FONT_SRC_BMP, gui_rgb(255,
                         255, 255), 11, 32);
            gui_text_type_set((gui_text_t *)workout_indoor_walk_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)workout_indoor_walk_text, CENTER);
            break;
        }
    case 6:
        {
            // Create workout_jump_rope_icon (hg_image)
            workout_jump_rope_icon = gui_img_create_from_fs((gui_obj_t *)note, "workout_jump_rope_icon",
                                                            "/app_workout/workout_jump_rope_icon.bin", 140, 85, 130, 130);
            // Create workout_jump_rope_text (hg_label)
            workout_jump_rope_text = gui_text_create((gui_obj_t *)note, "workout_jump_rope_text", 0, 229, 410,
                                                     42);
            gui_text_set((gui_text_t *)workout_jump_rope_text, "jump rope", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 9, 32);
            gui_text_type_set((gui_text_t *)workout_jump_rope_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)workout_jump_rope_text, CENTER);
            break;
        }
    case 7:
        {
            // Create workout_on_foot_icon (hg_image)
            workout_on_foot_icon = gui_img_create_from_fs((gui_obj_t *)note, "workout_on_foot_icon",
                                                          "/app_workout/workout_on_foot_icon.bin", 140, 85, 130, 130);
            // Create workout_on_foot_text (hg_label)
            workout_on_foot_text = gui_text_create((gui_obj_t *)note, "workout_on_foot_text", 0, 221, 410, 42);
            gui_text_set((gui_text_t *)workout_on_foot_text, "on foot", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 7, 32);
            gui_text_type_set((gui_text_t *)workout_on_foot_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)workout_on_foot_text, CENTER);
            break;
        }
    case 8:
        {
            // Create workout_outdoor_climb_icon (hg_image)
            workout_outdoor_climb_icon = gui_img_create_from_fs((gui_obj_t *)note, "workout_outdoor_climb_icon",
                                                                "/app_workout/workout_outdoor_climb_icon.bin", 140, 85, 130, 130);
            // Create workout_outdoor_climb_text (hg_label)
            workout_outdoor_climb_text = gui_text_create((gui_obj_t *)note, "workout_outdoor_climb_text", 0,
                                                         221, 410, 42);
            gui_text_set((gui_text_t *)workout_outdoor_climb_text, "outdoor climb", GUI_FONT_SRC_BMP,
                         gui_rgb(255, 255, 255), 13, 32);
            gui_text_type_set((gui_text_t *)workout_outdoor_climb_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)workout_outdoor_climb_text, CENTER);
            break;
        }
    case 9:
        {
            // Create workout_outdoor_run_icon (hg_image)
            workout_outdoor_run_icon = gui_img_create_from_fs((gui_obj_t *)note, "workout_outdoor_run_icon",
                                                              "/app_workout/workout_outdoor_run_icon.bin", 140, 85, 130, 130);
            // Create workout_outdoor_run_text (hg_label)
            workout_outdoor_run_text = gui_text_create((gui_obj_t *)note, "workout_outdoor_run_text", 0, 235,
                                                       410, 42);
            gui_text_set((gui_text_t *)workout_outdoor_run_text, "outdoor run", GUI_FONT_SRC_BMP, gui_rgb(255,
                         255, 255), 11, 32);
            gui_text_type_set((gui_text_t *)workout_outdoor_run_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)workout_outdoor_run_text, CENTER);
            break;
        }
    case 10:
        {
            // Create workout_outdoor_walk_icon (hg_image)
            workout_outdoor_walk_icon = gui_img_create_from_fs((gui_obj_t *)note, "workout_outdoor_walk_icon",
                                                               "/app_workout/workout_outdoor_walk.bin", 140, 85, 130, 130);
            // Create workout_outdoor_walk_text (hg_label)
            workout_outdoor_walk_text = gui_text_create((gui_obj_t *)note, "workout_outdoor_walk_text", 0, 239,
                                                        410, 42);
            gui_text_set((gui_text_t *)workout_outdoor_walk_text, "outdoor walk", GUI_FONT_SRC_BMP, gui_rgb(255,
                         255, 255), 12, 32);
            gui_text_type_set((gui_text_t *)workout_outdoor_walk_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)workout_outdoor_walk_text, CENTER);
            break;
        }
    case 11:
        {
            // Create workout_sit_up_icon (hg_image)
            workout_sit_up_icon = gui_img_create_from_fs((gui_obj_t *)note, "workout_sit_up_icon",
                                                         "/app_workout/workout_sit_up_icon.bin", 140, 85, 130, 130);
            // Create workout_sit_up_text (hg_label)
            workout_sit_up_text = gui_text_create((gui_obj_t *)note, "workout_sit_up_text", 0, 228, 410, 42);
            gui_text_set((gui_text_t *)workout_sit_up_text, "sit up", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                         6, 32);
            gui_text_type_set((gui_text_t *)workout_sit_up_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)workout_sit_up_text, CENTER);
            break;
        }
    case 12:
        {
            // Create workout_stretch_icon (hg_image)
            workout_stretch_icon = gui_img_create_from_fs((gui_obj_t *)note, "workout_stretch_icon",
                                                          "/app_workout/workout_stretch_icon.bin", 140, 85, 130, 130);
            // Create workout_stretch_text (hg_label)
            workout_stretch_text = gui_text_create((gui_obj_t *)note, "workout_stretch_text", 0, 231, 410, 42);
            gui_text_set((gui_text_t *)workout_stretch_text, "stretch", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 7, 32);
            gui_text_type_set((gui_text_t *)workout_stretch_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)workout_stretch_text, CENTER);
            break;
        }
    case 13:
        {
            // Create workout_swim_icon (hg_image)
            workout_swim_icon = gui_img_create_from_fs((gui_obj_t *)note, "workout_swim_icon",
                                                       "/app_workout/workout_swim_icon.bin", 140, 85, 130, 130);
            // Create workout_swim_text (hg_label)
            workout_swim_text = gui_text_create((gui_obj_t *)note, "workout_swim_text", 0, 207, 410, 42);
            gui_text_set((gui_text_t *)workout_swim_text, "swiming", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                         7, 32);
            gui_text_type_set((gui_text_t *)workout_swim_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)workout_swim_text, CENTER);
            break;
        }
    case 14:
        {
            // Create workout_taekwondo_icon (hg_image)
            workout_taekwondo_icon = gui_img_create_from_fs((gui_obj_t *)note, "workout_taekwondo_icon",
                                                            "/app_workout/workout_taekwondo_icon.bin", 140, 85, 130, 130);
            // Create workout_taekwondo_text (hg_label)
            workout_taekwondo_text = gui_text_create((gui_obj_t *)note, "workout_taekwondo_text", 0, 219, 410,
                                                     42);
            gui_text_set((gui_text_t *)workout_taekwondo_text, "taekwondo", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 9, 32);
            gui_text_type_set((gui_text_t *)workout_taekwondo_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)workout_taekwondo_text, CENTER);
            break;
        }
    case 15:
        {
            // Create workout_weightlifte_icon (hg_image)
            workout_weightlifte_icon = gui_img_create_from_fs((gui_obj_t *)note, "workout_weightlifte_icon",
                                                              "/app_workout/workout_weightlifte_icon.bin", 140, 85, 130, 130);
            // Create workout_weightlifte_text (hg_label)
            workout_weightlifte_text = gui_text_create((gui_obj_t *)note, "workout_weightlifte_text", 0, 214,
                                                       410, 42);
            gui_text_set((gui_text_t *)workout_weightlifte_text, "weightlifte", GUI_FONT_SRC_BMP, gui_rgb(255,
                         255, 255), 11, 32);
            gui_text_type_set((gui_text_t *)workout_weightlifte_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)workout_weightlifte_text, CENTER);
            break;
        }
    case 16:
        {
            // Create workout_yoga_icon (hg_image)
            workout_yoga_icon = gui_img_create_from_fs((gui_obj_t *)note, "workout_yoga_icon",
                                                       "/app_workout/workout_yoga_icon.bin", 151, 85, 130, 130);
            // Create workout_yoga_text (hg_label)
            workout_yoga_text = gui_text_create((gui_obj_t *)note, "workout_yoga_text", 0, 227, 410, 42);
            gui_text_set((gui_text_t *)workout_yoga_text, "yoga", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 4,
                         32);
            gui_text_type_set((gui_text_t *)workout_yoga_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)workout_yoga_text, CENTER);
            break;
        }
    case 17:
        {
            // Create workout_mode_add_bg (hg_circle)
            workout_mode_add_bg = gui_circle_create((gui_obj_t *)note, "workout_mode_add_bg", 212, 61, 40,
                                                    gui_rgb(185, 251, 79));
            // Create hg_image_1769492435275_x96q (hg_image)
            hg_image_1769492435275_x96q = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1769492435275_x96q", "/app_workout/workout_mode_add_icon.bin", 192, 41, 40, 40);
            break;
        }
    default:
        break;
    }
}


// Create app_workout_view (hg_view)
static void app_workout_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_workout_view_switch_in(gui_view_t *view)
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
        snprintf(app_workout_time_text_time_str, sizeof(app_workout_time_text_time_str), "%02d:%02d",
                 t->tm_hour, t->tm_min);
    }



    // Create app_workout_bg (hg_rect)
    app_workout_bg = gui_rect_create((gui_obj_t *)view, "app_workout_bg", 0, 0, 410, 502, 0, gui_rgb(32,
                                     36, 17));

    // Create app_workout_menu_list (hg_list)
    app_workout_menu_list = gui_list_create((gui_obj_t *)view, "app_workout_menu_list", 0, 127, 410,
                                            375, 300, 5, VERTICAL, app_workout_menu_list_note_design, NULL, false);
    gui_list_set_style(app_workout_menu_list, LIST_CLASSIC);
    gui_list_set_note_num(app_workout_menu_list, 18);
    gui_list_set_auto_align(app_workout_menu_list, true);
    gui_list_set_out_scope(app_workout_menu_list, 80);

    // Create app_workout_menu_topwindow (hg_window)
    app_workout_menu_topwindow = gui_win_create((gui_obj_t *)view, "app_workout_menu_topwindow", 0, 0,
                                                410, 100);
    gui_win_enable_blur((gui_win_t *)app_workout_menu_topwindow, true);
    gui_win_set_blur_degree((gui_win_t *)app_workout_menu_topwindow, 225);


    // Create app_workout_icon_bg0 (hg_image)
    app_workout_icon_bg0 = gui_img_create_from_fs(app_workout_menu_topwindow, "app_workout_icon_bg0",
                                                  "/app_workout/workout_Left_Control.bin", 25, 10, 72, 72);

    // Create app_workout_icon0 (hg_image)
    app_workout_icon0 = gui_img_create_from_fs(app_workout_menu_topwindow, "app_workout_icon0",
                                               "/app_workout/workout_view_icon.bin", 43, 28, 36, 36);

    // Create app_workout_icon_bg1 (hg_image)
    app_workout_icon_bg1 = gui_img_create_from_fs(app_workout_menu_topwindow, "app_workout_icon_bg1",
                                                  "/app_workout/workout_Left_Control.bin", 323, 10, 72, 72);
    // Bind timer: Timer animation 1
    gui_obj_create_timer((gui_obj_t *)app_workout_icon_bg1, 1000, true,
                         hg_image_1769161039267_t63r_timer_0_cb);
    gui_obj_start_timer((gui_obj_t *)app_workout_icon_bg1);

    // Create app_workout_icon1 (hg_image)
    app_workout_icon1 = gui_img_create_from_fs(app_workout_menu_topwindow, "app_workout_icon1",
                                               "/app_workout/workout_target_icon.bin", 341, 28, 36, 36);

    gui_obj_add_event_cb(GUI_BASE(app_workout_menu_topwindow),
                         (gui_event_cb_t)app_workout_menu_topwindow_key_0_cb, GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)app_workout_menu_topwindow);

    // Create app_workout_time_text (hg_time_label)
    app_workout_time_text = gui_text_create((gui_obj_t *)view, "app_workout_time_text", 155, 15, 100,
                                            36);
    gui_text_set((gui_text_t *)app_workout_time_text, app_workout_time_text_time_str, GUI_FONT_SRC_BMP,
                 gui_rgb(255, 255, 255), strlen(app_workout_time_text_time_str), 32);
    gui_text_type_set((gui_text_t *)app_workout_time_text,
                      "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)app_workout_time_text, CENTER);

    // Create app_workout_menu_bottomwindow (hg_window)
    app_workout_menu_bottomwindow = gui_win_create((gui_obj_t *)view, "app_workout_menu_bottomwindow",
                                                   0, 402, 410, 100);
    gui_win_enable_blur((gui_win_t *)app_workout_menu_bottomwindow, true);
    gui_win_set_blur_degree((gui_win_t *)app_workout_menu_bottomwindow, 225);


    // Create app_workout_start_icon (hg_image)
    app_workout_start_icon = gui_img_create_from_fs(app_workout_menu_bottomwindow,
                                                    "app_workout_start_icon", "/app_workout/workout_start_icon.bin", 134, -30, 72, 72);
    gui_img_translate((gui_img_t *)app_workout_start_icon, 72.0f, 72.0f);
    gui_img_set_focus((gui_img_t *)app_workout_start_icon, 36.0f, 36.0f);
    gui_obj_add_event_cb(app_workout_start_icon, (gui_event_cb_t)app_workout_start_icon_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create app_workout_icon_bg2 (hg_image)
    app_workout_icon_bg2 = gui_img_create_from_fs(app_workout_menu_bottomwindow, "app_workout_icon_bg2",
                                                  "/app_workout/workout_Left_Control.bin", 25, 20, 72, 72);

    // Create app_workout_icon2 (hg_image)
    app_workout_icon2 = gui_img_create_from_fs(app_workout_menu_bottomwindow, "app_workout_icon2",
                                               "/app_workout/workout_media_icon.bin", 48, 40, 26, 32);

    // Create app_workout_icon_bg3 (hg_image)
    app_workout_icon_bg3 = gui_img_create_from_fs(app_workout_menu_bottomwindow, "app_workout_icon_bg3",
                                                  "/app_workout/workout_Left_Control.bin", 323, 20, 72, 72);

    // Create app_workout_icon3 (hg_image)
    app_workout_icon3 = gui_img_create_from_fs(app_workout_menu_bottomwindow, "app_workout_icon3",
                                               "/app_workout/workout_remind_icon.bin", 341, 40, 36, 36);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(app_workout_time_text), 30000, true,
                         app_workout_time_text_time_update_cb);
}
GUI_VIEW_INSTANCE("app_workout_view", false, app_workout_view_switch_in,
                  app_workout_view_switch_out, false);

// Create app_workout_start_engine_view (hg_view)
static void app_workout_start_engine_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_workout_start_engine_view_switch_in(gui_view_t *view)
{
    // Set animation step
    gui_view_set_animate_step(view, 50);

    // Set opacity
    gui_view_set_opacity(view, 255);

    // Set background color
    gui_set_bg_color(gui_rgb(0, 0, 0));

    GUI_UNUSED(view);


    // Create app_workout_start_engine_bg (hg_rect)
    app_workout_start_engine_bg = gui_rect_create((gui_obj_t *)view, "app_workout_start_engine_bg", 0,
                                                  0, 410, 502, 0, gui_rgb(32, 36, 17));
    gui_obj_add_event_cb(app_workout_start_engine_bg,
                         (gui_event_cb_t)app_workout_start_engine_bg_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create workout_countdown_arc (hg_arc)
    workout_countdown_arc = gui_arc_create((gui_obj_t *)view, "workout_countdown_arc", 205, 251, 150,
                                           -90, 270, 20, gui_rgb(184, 250, 79));
    gui_obj_add_event_cb(workout_countdown_arc, (gui_event_cb_t)workout_countdown_arc_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);
}
GUI_VIEW_INSTANCE("app_workout_start_engine_view", false, app_workout_start_engine_view_switch_in,
                  app_workout_start_engine_view_switch_out, false);

// Create app_workout_remind_view (hg_view)
static void app_workout_remind_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_workout_remind_view_switch_in(gui_view_t *view)
{
    // Set animation step
    gui_view_set_animate_step(view, 50);

    // Set opacity
    gui_view_set_opacity(view, 255);

    // Set background color
    gui_set_bg_color(gui_rgb(32, 36, 17));

    GUI_UNUSED(view);

    // Initialize time strings
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t != NULL)
    {
        snprintf(hg_time_label_1769406976200_a8x9_time_str,
                 sizeof(hg_time_label_1769406976200_a8x9_time_str), "%02d:%02d", t->tm_hour, t->tm_min);
    }



    // Create app_workout_remind_bg (hg_rect)
    app_workout_remind_bg = gui_rect_create((gui_obj_t *)view, "app_workout_remind_bg", 1066, 888, 410,
                                            502, 0, gui_rgb(32, 36, 17));

    // Create hg_time_label_1769406976200_a8x9 (hg_time_label)
    hg_time_label_1769406976200_a8x9 = gui_text_create((gui_obj_t *)view,
                                                       "hg_time_label_1769406976200_a8x9", 287, 25, 100, 36);
    gui_text_set((gui_text_t *)hg_time_label_1769406976200_a8x9,
                 hg_time_label_1769406976200_a8x9_time_str, GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                 strlen(hg_time_label_1769406976200_a8x9_time_str), 32);
    gui_text_type_set((gui_text_t *)hg_time_label_1769406976200_a8x9,
                      "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_time_label_1769406976200_a8x9, RIGHT);

    // Create hg_image_1769407041833_6sxz (hg_image)
    hg_image_1769407041833_6sxz = gui_img_create_from_fs((gui_obj_t *)view,
                                                         "hg_image_1769407041833_6sxz", "/app_workout/workout_Left_Control.bin", 20, 15, 72, 72);

    // Create hg_image_1769407067590_35cr (hg_image)
    hg_image_1769407067590_35cr = gui_img_create_from_fs((gui_obj_t *)view,
                                                         "hg_image_1769407067590_35cr", "/app_workout/workout_outdoor_walk.bin", 37, 34, 130, 130);
    gui_img_scale((gui_img_t *)hg_image_1769407067590_35cr, 0.300000f, 0.300000f);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(hg_time_label_1769406976200_a8x9), 30000, true,
                         hg_time_label_1769406976200_a8x9_time_update_cb);
}
GUI_VIEW_INSTANCE("app_workout_remind_view", false, app_workout_remind_view_switch_in,
                  app_workout_remind_view_switch_out, false);
