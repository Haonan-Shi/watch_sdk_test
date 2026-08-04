/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_reminders UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-06T09:11:40.941Z
 */
#include "app_reminders_ui.h"
#include "../callbacks/app_reminders_callbacks.h"
#include "../user/app_reminders_user.h"
#include <stddef.h>
#include <time.h>

// Component handle definitions
gui_list_t *hg_list_1768376200937_b4db = NULL;
gui_rounded_rect_t *hg_rect_1768377692400_gf4l = NULL;
gui_circle_t *hg_circle_1768378695800_g5us = NULL;
gui_img_t *hg_image_1768378673990_4p6o = NULL;
gui_text_t *hg_label_1768378942196_18fm = NULL;
gui_text_t *hg_label_1768379934657_dkt4 = NULL;
gui_rounded_rect_t *hg_rect_1768377692400_gf4l_copy_1768377813989 = NULL;
gui_circle_t *hg_circle_1768378695800_g5us_copy_1768378781720 = NULL;
gui_img_t *hg_image_1768378677885_4ne1 = NULL;
gui_text_t *hg_label_1768378942196_18fm_copy_1768378990845 = NULL;
gui_text_t *hg_label_1768380068543_kz2a = NULL;
gui_rounded_rect_t *hg_rect_1768378071624_epqe = NULL;
gui_circle_t *hg_circle_1768378695800_g5us_copy_1768378781720_copy_1768378806762 = NULL;
gui_img_t *hg_image_1768378685018_wd11 = NULL;
gui_text_t *hg_label_1768378942196_18fm_copy_1768378990845_copy_1768379028761_copy_1768379106308 =
    NULL;
gui_text_t *hg_label_1768380068543_kz2a_copy_1768380153587 = NULL;
gui_rounded_rect_t *hg_rect_1768378521798_m5t2 = NULL;
gui_circle_t
*hg_circle_1768378695800_g5us_copy_1768378781720_copy_1768378806762_copy_1768378854856_copy_1768378911226
    = NULL;
gui_img_t *hg_image_1768378687266_vr7i = NULL;
gui_text_t
*hg_label_1768378942196_18fm_copy_1768378990845_copy_1768379028761_copy_1768379106308_copy_1768379124513
    = NULL;
gui_rounded_rect_t *hg_rect_1768377692400_gf4l_copy_1768377813989_copy_1768377843821 = NULL;
gui_circle_t *hg_circle_1768378695800_g5us_copy_1768378781720_copy_1768378806762_copy_1768378854856
    = NULL;
gui_text_t *hg_label_1768378942196_18fm_copy_1768378990845_copy_1768379028761 = NULL;
gui_img_t *hg_image_1768378680855_05ey = NULL;
gui_text_t *hg_label_1768380068543_kz2a_copy_1768380153587_copy_1768380157594_copy_1768380161650 =
    NULL;
gui_win_t *hg_window_1768375945225_5zqu = NULL;
gui_text_t *hg_label_1768376091766_um28 = NULL;
gui_text_t *hg_time_label_1768896918843_c995 = NULL;
gui_rounded_rect_t *hg_rect_1768381023374_vc4v = NULL;
gui_text_t *hg_label_1768381142245_i2iu_copy_1768381267042 = NULL;
gui_text_t *hg_label_1768381142245_i2iu = NULL;
gui_win_t *app_reminder_today_window = NULL;
gui_circle_t *hg_circle_1768383603782_kadm = NULL;
gui_img_t *hg_image_1768380819956_fhsk = NULL;
gui_text_t *hg_time_label_1768896992562_xhlr = NULL;
gui_text_t *hg_label_1768376091766_um28_copy_1768380290090 = NULL;

// Time string global variables
char hg_time_label_1768896918843_c995_time_str[10] = {0};
char hg_time_label_1768896992562_xhlr_time_str[10] = {0};

// List component note_design callback functions
// note_design callback function declaration
static void hg_list_1768376200937_b4db_note_design(gui_obj_t *obj, void *param);

// note_design callback function implementation
static void hg_list_1768376200937_b4db_note_design(gui_obj_t *obj, void *param)
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
            // Create hg_rect_1768377692400_gf4l (hg_rect)
            hg_rect_1768377692400_gf4l = gui_rect_create((gui_obj_t *)note, "hg_rect_1768377692400_gf4l", 0, 0,
                                                         410, 150, 50, gui_rgb(0, 122, 204));
            // Set linear gradient
            gui_rect_set_linear_gradient(hg_rect_1768377692400_gf4l, RECT_GRADIENT_VERTICAL);
            gui_rect_add_gradient_stop(hg_rect_1768377692400_gf4l, 0.0f, gui_rgba(78, 146, 228, 255));
            gui_rect_add_gradient_stop(hg_rect_1768377692400_gf4l, 1.0f, gui_rgba(49, 108, 194, 255));
            gui_obj_add_event_cb(hg_rect_1768377692400_gf4l,
                                 (gui_event_cb_t)hg_rect_1768377692400_gf4l_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create hg_circle_1768378695800_g5us (hg_circle)
            hg_circle_1768378695800_g5us = gui_circle_create((gui_obj_t *)note, "hg_circle_1768378695800_g5us",
                                                             59, 49, 36, gui_rgba(255, 255, 255, 128));
            // Create hg_image_1768378673990_4p6o (hg_image)
            hg_image_1768378673990_4p6o = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1768378673990_4p6o", "/app_reminder/reminders_icon0.bin", 39, 30, 40, 37);
            // Create hg_label_1768378942196_18fm (hg_label)
            hg_label_1768378942196_18fm = gui_text_create((gui_obj_t *)note, "hg_label_1768378942196_18fm", 39,
                                                          97, 100, 46);
            gui_text_set((gui_text_t *)hg_label_1768378942196_18fm, "Today", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 5, 36);
            gui_text_type_set((gui_text_t *)hg_label_1768378942196_18fm,
                              "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_1768378942196_18fm, LEFT);
            // Create hg_label_1768379934657_dkt4 (hg_label)
            hg_label_1768379934657_dkt4 = gui_text_create((gui_obj_t *)note, "hg_label_1768379934657_dkt4", 337,
                                                          19, 50, 80);
            gui_text_set((gui_text_t *)hg_label_1768379934657_dkt4, "0", GUI_FONT_SRC_BMP, gui_rgb(201, 201,
                         201), 1, 70);
            gui_text_type_set((gui_text_t *)hg_label_1768379934657_dkt4,
                              "/font/Inter_24pt_Regular_size70_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_1768379934657_dkt4, LEFT);
            break;
        }
    case 1:
        {
            // Create hg_rect_1768377692400_gf4l_copy_1768377813989 (hg_rect)
            hg_rect_1768377692400_gf4l_copy_1768377813989 = gui_rect_create((gui_obj_t *)note,
                                                                            "hg_rect_1768377692400_gf4l_copy_1768377813989", 0, 0, 410, 150, 50, gui_rgb(0, 122, 204));
            // Set linear gradient
            gui_rect_set_linear_gradient(hg_rect_1768377692400_gf4l_copy_1768377813989, RECT_GRADIENT_VERTICAL);
            gui_rect_add_gradient_stop(hg_rect_1768377692400_gf4l_copy_1768377813989, 0.0f, gui_rgba(226, 87,
                                       72, 255));
            gui_rect_add_gradient_stop(hg_rect_1768377692400_gf4l_copy_1768377813989, 1.0f, gui_rgba(188, 53,
                                       38, 255));
            // Create hg_circle_1768378695800_g5us_copy_1768378781720 (hg_circle)
            hg_circle_1768378695800_g5us_copy_1768378781720 = gui_circle_create((gui_obj_t *)note,
                                                                                "hg_circle_1768378695800_g5us_copy_1768378781720", 59, 48, 36, gui_rgba(255, 255, 255, 128));
            // Create hg_image_1768378677885_4ne1 (hg_image)
            hg_image_1768378677885_4ne1 = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1768378677885_4ne1", "/app_reminder/reminders_icon1.bin", 39, 28, 40, 40);
            // Create hg_label_1768378942196_18fm_copy_1768378990845 (hg_label)
            hg_label_1768378942196_18fm_copy_1768378990845 = gui_text_create((gui_obj_t *)note,
                                                                             "hg_label_1768378942196_18fm_copy_1768378990845", 39, 97, 207, 46);
            gui_text_set((gui_text_t *)hg_label_1768378942196_18fm_copy_1768378990845, "Scheduled",
                         GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 9, 36);
            gui_text_type_set((gui_text_t *)hg_label_1768378942196_18fm_copy_1768378990845,
                              "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_1768378942196_18fm_copy_1768378990845, LEFT);
            // Create hg_label_1768380068543_kz2a (hg_label)
            hg_label_1768380068543_kz2a = gui_text_create((gui_obj_t *)note, "hg_label_1768380068543_kz2a", 337,
                                                          19, 50, 80);
            gui_text_set((gui_text_t *)hg_label_1768380068543_kz2a, "0", GUI_FONT_SRC_BMP, gui_rgb(201, 201,
                         201), 1, 70);
            gui_text_type_set((gui_text_t *)hg_label_1768380068543_kz2a,
                              "/font/Inter_24pt_Regular_size70_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_1768380068543_kz2a, LEFT);
            break;
        }
    case 2:
        {
            // Create hg_rect_1768378071624_epqe (hg_rect)
            hg_rect_1768378071624_epqe = gui_rect_create((gui_obj_t *)note, "hg_rect_1768378071624_epqe", 0, 0,
                                                         410, 150, 50, gui_rgb(0, 122, 204));
            // Set linear gradient
            gui_rect_set_linear_gradient(hg_rect_1768378071624_epqe, RECT_GRADIENT_VERTICAL);
            gui_rect_add_gradient_stop(hg_rect_1768378071624_epqe, 0.0f, gui_rgba(64, 63, 79, 255));
            gui_rect_add_gradient_stop(hg_rect_1768378071624_epqe, 1.0f, gui_rgba(59, 55, 73, 255));
            // Create hg_circle_1768378695800_g5us_copy_1768378781720_copy_1768378806762 (hg_circle)
            hg_circle_1768378695800_g5us_copy_1768378781720_copy_1768378806762 = gui_circle_create((
                                                                                     gui_obj_t *)note, "hg_circle_1768378695800_g5us_copy_1768378781720_copy_1768378806762", 59, 48, 36,
                                                                                 gui_rgba(255, 255, 255, 128));
            // Create hg_image_1768378685018_wd11 (hg_image)
            hg_image_1768378685018_wd11 = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1768378685018_wd11", "/app_reminder/reminders_icon3.bin", 41, 32, 36, 33);
            // Create hg_label_1768378942196_18fm_copy_1768378990845_copy_1768379028761_copy_1768379106308 (hg_label)
            hg_label_1768378942196_18fm_copy_1768378990845_copy_1768379028761_copy_1768379106308 =
                gui_text_create((gui_obj_t *)note,
                                "hg_label_1768378942196_18fm_copy_1768378990845_copy_1768379028761_copy_1768379106308", 39, 104,
                                160, 46);
            gui_text_set((gui_text_t *)
                         hg_label_1768378942196_18fm_copy_1768378990845_copy_1768379028761_copy_1768379106308, "All",
                         GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 3, 36);
            gui_text_type_set((gui_text_t *)
                              hg_label_1768378942196_18fm_copy_1768378990845_copy_1768379028761_copy_1768379106308,
                              "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)
                              hg_label_1768378942196_18fm_copy_1768378990845_copy_1768379028761_copy_1768379106308, LEFT);
            // Create hg_label_1768380068543_kz2a_copy_1768380153587 (hg_label)
            hg_label_1768380068543_kz2a_copy_1768380153587 = gui_text_create((gui_obj_t *)note,
                                                                             "hg_label_1768380068543_kz2a_copy_1768380153587", 337, 19, 50, 80);
            gui_text_set((gui_text_t *)hg_label_1768380068543_kz2a_copy_1768380153587, "0", GUI_FONT_SRC_BMP,
                         gui_rgb(201, 201, 201), 1, 70);
            gui_text_type_set((gui_text_t *)hg_label_1768380068543_kz2a_copy_1768380153587,
                              "/font/Inter_24pt_Regular_size70_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_1768380068543_kz2a_copy_1768380153587, LEFT);
            break;
        }
    case 3:
        {
            // Create hg_rect_1768378521798_m5t2 (hg_rect)
            hg_rect_1768378521798_m5t2 = gui_rect_create((gui_obj_t *)note, "hg_rect_1768378521798_m5t2", 0, 0,
                                                         410, 150, 50, gui_rgb(0, 122, 204));
            // Set linear gradient
            gui_rect_set_linear_gradient(hg_rect_1768378521798_m5t2, RECT_GRADIENT_VERTICAL);
            gui_rect_add_gradient_stop(hg_rect_1768378521798_m5t2, 0.0f, gui_rgba(105, 110, 151, 255));
            gui_rect_add_gradient_stop(hg_rect_1768378521798_m5t2, 1.0f, gui_rgba(89, 94, 143, 255));
            // Create hg_circle_1768378695800_g5us_copy_1768378781720_copy_1768378806762_copy_1768378854856_copy_1768378911226 (hg_circle)
            hg_circle_1768378695800_g5us_copy_1768378781720_copy_1768378806762_copy_1768378854856_copy_1768378911226
                = gui_circle_create((gui_obj_t *)note,
                                    "hg_circle_1768378695800_g5us_copy_1768378781720_copy_1768378806762_copy_1768378854856_copy_1768378911226",
                                    59, 59, 36, gui_rgba(255, 255, 255, 128));
            // Create hg_image_1768378687266_vr7i (hg_image)
            hg_image_1768378687266_vr7i = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1768378687266_vr7i", "/app_reminder/reminders_icon4.bin", 41, 48, 36, 23);
            // Create hg_label_1768378942196_18fm_copy_1768378990845_copy_1768379028761_copy_1768379106308_copy_1768379124513 (hg_label)
            hg_label_1768378942196_18fm_copy_1768378990845_copy_1768379028761_copy_1768379106308_copy_1768379124513
                = gui_text_create((gui_obj_t *)note,
                                  "hg_label_1768378942196_18fm_copy_1768378990845_copy_1768379028761_copy_1768379106308_copy_1768379124513",
                                  39, 97, 230, 46);
            gui_text_set((gui_text_t *)
                         hg_label_1768378942196_18fm_copy_1768378990845_copy_1768379028761_copy_1768379106308_copy_1768379124513,
                         "Completed", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 9, 36);
            gui_text_type_set((gui_text_t *)
                              hg_label_1768378942196_18fm_copy_1768378990845_copy_1768379028761_copy_1768379106308_copy_1768379124513,
                              "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)
                              hg_label_1768378942196_18fm_copy_1768378990845_copy_1768379028761_copy_1768379106308_copy_1768379124513,
                              LEFT);
            break;
        }
    case 4:
        {
            // Create hg_rect_1768377692400_gf4l_copy_1768377813989_copy_1768377843821 (hg_rect)
            hg_rect_1768377692400_gf4l_copy_1768377813989_copy_1768377843821 = gui_rect_create((
                                                                                   gui_obj_t *)note, "hg_rect_1768377692400_gf4l_copy_1768377813989_copy_1768377843821", 0, 0, 410,
                                                                               150, 50, gui_rgb(0, 122, 204));
            // Set linear gradient
            gui_rect_set_linear_gradient(hg_rect_1768377692400_gf4l_copy_1768377813989_copy_1768377843821,
                                         RECT_GRADIENT_VERTICAL);
            gui_rect_add_gradient_stop(hg_rect_1768377692400_gf4l_copy_1768377813989_copy_1768377843821, 0.0f,
                                       gui_rgba(254, 167, 98, 255));
            gui_rect_add_gradient_stop(hg_rect_1768377692400_gf4l_copy_1768377813989_copy_1768377843821, 1.0f,
                                       gui_rgba(255, 159, 10, 255));
            // Create hg_circle_1768378695800_g5us_copy_1768378781720_copy_1768378806762_copy_1768378854856 (hg_circle)
            hg_circle_1768378695800_g5us_copy_1768378781720_copy_1768378806762_copy_1768378854856 =
                gui_circle_create((gui_obj_t *)note,
                                  "hg_circle_1768378695800_g5us_copy_1768378781720_copy_1768378806762_copy_1768378854856", 59, 48, 36,
                                  gui_rgba(255, 255, 255, 128));
            // Create hg_label_1768378942196_18fm_copy_1768378990845_copy_1768379028761 (hg_label)
            hg_label_1768378942196_18fm_copy_1768378990845_copy_1768379028761 = gui_text_create((
                                                                                    gui_obj_t *)note, "hg_label_1768378942196_18fm_copy_1768378990845_copy_1768379028761", 39, 97, 204,
                                                                                46);
            gui_text_set((gui_text_t *)hg_label_1768378942196_18fm_copy_1768378990845_copy_1768379028761,
                         "Reminder", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 8, 36);
            gui_text_type_set((gui_text_t *)hg_label_1768378942196_18fm_copy_1768378990845_copy_1768379028761,
                              "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_1768378942196_18fm_copy_1768378990845_copy_1768379028761,
                              LEFT);
            // Create hg_image_1768378680855_05ey (hg_image)
            hg_image_1768378680855_05ey = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1768378680855_05ey", "/app_reminder/reminders_icon2.bin", 39, 30, 36, 36);
            // Create hg_label_1768380068543_kz2a_copy_1768380153587_copy_1768380157594_copy_1768380161650 (hg_label)
            hg_label_1768380068543_kz2a_copy_1768380153587_copy_1768380157594_copy_1768380161650 =
                gui_text_create((gui_obj_t *)note,
                                "hg_label_1768380068543_kz2a_copy_1768380153587_copy_1768380157594_copy_1768380161650", 337, 19, 50,
                                80);
            gui_text_set((gui_text_t *)
                         hg_label_1768380068543_kz2a_copy_1768380153587_copy_1768380157594_copy_1768380161650, "0",
                         GUI_FONT_SRC_BMP, gui_rgb(201, 201, 201), 1, 70);
            gui_text_type_set((gui_text_t *)
                              hg_label_1768380068543_kz2a_copy_1768380153587_copy_1768380157594_copy_1768380161650,
                              "/font/Inter_24pt_Regular_size70_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)
                              hg_label_1768380068543_kz2a_copy_1768380153587_copy_1768380157594_copy_1768380161650, LEFT);
            break;
        }
    default:
        break;
    }
}


// Create app_reminder_view (hg_view)
static void app_reminder_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_reminder_view_switch_in(gui_view_t *view)
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



    // Create hg_list_1768376200937_b4db (hg_list)
    hg_list_1768376200937_b4db = gui_list_create((gui_obj_t *)view, "hg_list_1768376200937_b4db", 0,
                                                 115, 410, 387, 150, 5, VERTICAL, hg_list_1768376200937_b4db_note_design, NULL, false);
    gui_list_set_style(hg_list_1768376200937_b4db, LIST_CLASSIC);
    gui_list_set_note_num(hg_list_1768376200937_b4db, 5);
    gui_list_set_out_scope(hg_list_1768376200937_b4db, 80);

    // Create hg_window_1768375945225_5zqu (hg_window)
    hg_window_1768375945225_5zqu = gui_win_create((gui_obj_t *)view, "hg_window_1768375945225_5zqu", 0,
                                                  0, 410, 100);
    gui_win_enable_blur((gui_win_t *)hg_window_1768375945225_5zqu, true);
    gui_win_set_blur_degree((gui_win_t *)hg_window_1768375945225_5zqu, 225);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(hg_time_label_1768896918843_c995_time_str,
                 sizeof(hg_time_label_1768896918843_c995_time_str), "%02d:%02d", t->tm_hour, t->tm_min);
    }


    // Create hg_label_1768376091766_um28 (hg_label)
    hg_label_1768376091766_um28 = gui_text_create(hg_window_1768375945225_5zqu,
                                                  "hg_label_1768376091766_um28", 267, 64, 119, 46);
    gui_text_set((gui_text_t *)hg_label_1768376091766_um28, "Lists", GUI_FONT_SRC_BMP, gui_rgb(66, 148,
                 244), 5, 28);
    gui_text_type_set((gui_text_t *)hg_label_1768376091766_um28,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_label_1768376091766_um28, RIGHT);

    // Create hg_time_label_1768896918843_c995 (hg_time_label)
    hg_time_label_1768896918843_c995 = gui_text_create(hg_window_1768375945225_5zqu,
                                                       "hg_time_label_1768896918843_c995", 306, 20, 80, 36);
    gui_text_set((gui_text_t *)hg_time_label_1768896918843_c995,
                 hg_time_label_1768896918843_c995_time_str, GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                 strlen(hg_time_label_1768896918843_c995_time_str), 28);
    gui_text_type_set((gui_text_t *)hg_time_label_1768896918843_c995,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_time_label_1768896918843_c995, RIGHT);

    gui_obj_add_event_cb(GUI_BASE(hg_window_1768375945225_5zqu),
                         (gui_event_cb_t)hg_window_1768375945225_5zqu_key_0_cb, GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)hg_window_1768375945225_5zqu);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(hg_time_label_1768896918843_c995), 30000, true,
                         hg_time_label_1768896918843_c995_time_update_cb);
}
GUI_VIEW_INSTANCE("app_reminder_view", false, app_reminder_view_switch_in,
                  app_reminder_view_switch_out, false);

// Create app_reminder_today_view (hg_view)
static void app_reminder_today_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_reminder_today_view_switch_in(gui_view_t *view)
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



    // Create hg_rect_1768381023374_vc4v (hg_rect)
    hg_rect_1768381023374_vc4v = gui_rect_create((gui_obj_t *)view, "hg_rect_1768381023374_vc4v", 58,
                                                 345, 300, 100, 50, gui_rgb(55, 117, 202));

    // Create app_reminder_today_window (hg_window)
    app_reminder_today_window = gui_win_create((gui_obj_t *)view, "app_reminder_today_window", 0, 0,
                                               410, 110);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(hg_time_label_1768896992562_xhlr_time_str,
                 sizeof(hg_time_label_1768896992562_xhlr_time_str), "%02d:%02d", t->tm_hour, t->tm_min);
    }


    // Create hg_label_1768376091766_um28_copy_1768380290090 (hg_label)
    hg_label_1768376091766_um28_copy_1768380290090 = gui_text_create(app_reminder_today_window,
                                                                     "hg_label_1768376091766_um28_copy_1768380290090", 306, 64, 84, 46);
    gui_text_set((gui_text_t *)hg_label_1768376091766_um28_copy_1768380290090, "Today",
                 GUI_FONT_SRC_BMP, gui_rgb(66, 148, 244), 5, 28);
    gui_text_type_set((gui_text_t *)hg_label_1768376091766_um28_copy_1768380290090,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_label_1768376091766_um28_copy_1768380290090, RIGHT);

    // Create hg_circle_1768383603782_kadm (hg_circle)
    hg_circle_1768383603782_kadm = gui_circle_create(app_reminder_today_window,
                                                     "hg_circle_1768383603782_kadm", 71, 48, 36, gui_rgba(255, 255, 255, 128));
    gui_obj_add_event_cb(hg_circle_1768383603782_kadm,
                         (gui_event_cb_t)hg_circle_1768383603782_kadm_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create hg_image_1768380819956_fhsk (hg_image)
    hg_image_1768380819956_fhsk = gui_img_create_from_fs(app_reminder_today_window,
                                                         "hg_image_1768380819956_fhsk", "/app_reminder/reminders_icon5.bin", 59, 25, 25, 47);

    // Create hg_time_label_1768896992562_xhlr (hg_time_label)
    hg_time_label_1768896992562_xhlr = gui_text_create(app_reminder_today_window,
                                                       "hg_time_label_1768896992562_xhlr", 308, 25, 80, 36);
    gui_text_set((gui_text_t *)hg_time_label_1768896992562_xhlr,
                 hg_time_label_1768896992562_xhlr_time_str, GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                 strlen(hg_time_label_1768896992562_xhlr_time_str), 28);
    gui_text_type_set((gui_text_t *)hg_time_label_1768896992562_xhlr,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_time_label_1768896992562_xhlr, RIGHT);

    gui_obj_add_event_cb(GUI_BASE(app_reminder_today_window),
                         (gui_event_cb_t)app_reminder_today_window_key_0_cb, GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)app_reminder_today_window);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(hg_time_label_1768896992562_xhlr), 30000, true,
                         hg_time_label_1768896992562_xhlr_time_update_cb);

    // Create hg_label_1768381142245_i2iu_copy_1768381267042 (hg_label)
    hg_label_1768381142245_i2iu_copy_1768381267042 = gui_text_create((gui_obj_t *)view,
                                                                     "hg_label_1768381142245_i2iu_copy_1768381267042", 40, 208, 330, 40);
    gui_text_set((gui_text_t *)hg_label_1768381142245_i2iu_copy_1768381267042, "No reminder items",
                 GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 17, 36);
    gui_text_type_set((gui_text_t *)hg_label_1768381142245_i2iu_copy_1768381267042,
                      "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_label_1768381142245_i2iu_copy_1768381267042, CENTER);

    // Create hg_label_1768381142245_i2iu (hg_label)
    hg_label_1768381142245_i2iu = gui_text_create((gui_obj_t *)view, "hg_label_1768381142245_i2iu", 57,
                                                  347, 301, 96);
    gui_text_set((gui_text_t *)hg_label_1768381142245_i2iu, "Add reminder items", GUI_FONT_SRC_BMP,
                 gui_rgb(255, 255, 255), 18, 28);
    gui_text_type_set((gui_text_t *)hg_label_1768381142245_i2iu,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_label_1768381142245_i2iu, MID_CENTER);
}
GUI_VIEW_INSTANCE("app_reminder_today_view", false, app_reminder_today_view_switch_in,
                  app_reminder_today_view_switch_out, false);
