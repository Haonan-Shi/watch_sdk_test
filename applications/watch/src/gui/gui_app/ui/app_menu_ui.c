/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_menu UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-06T09:11:40.846Z
 */
#include "app_menu_ui.h"
#include "../callbacks/app_menu_callbacks.h"
#include "../user/app_menu_user.h"
#include <stddef.h>

// Component handle definitions
gui_win_t *app_menu_window = NULL;
gui_list_t *app_menu_list = NULL;
gui_img_t *app_menu_activity_icon = NULL;
gui_text_t *app_menu_activity_text = NULL;
gui_img_t *hg_image_1766997222913_68q3 = NULL;
gui_text_t *app_menu_heart_text = NULL;
gui_img_t *hg_image_1766997230487_yrdp = NULL;
gui_text_t *app_menu_weather_text = NULL;
gui_img_t *hg_image_1766997236346_95l4 = NULL;
gui_text_t *app_menu_reminders_text = NULL;
gui_img_t *hg_image_1766997242674_bw37 = NULL;
gui_text_t *app_menu_message_text = NULL;
gui_img_t *hg_image_1766997251460_wgoz = NULL;
gui_text_t *app_menu_phone_text = NULL;
gui_img_t *hg_image_noise = NULL;
gui_text_t *app_menu_noise_text = NULL;
gui_img_t *hg_image_1766997263436_j1j0 = NULL;
gui_text_t *app_menu_music_text = NULL;
gui_img_t *hg_image_1766997276115_kywq = NULL;
gui_text_t *hg_label_map = NULL;
gui_img_t *hg_image_1766997286123_263r = NULL;
gui_text_t *hg_label_stopwatch = NULL;
gui_img_t *hg_image_1766997291073_d8qk = NULL;
gui_text_t *hg_label_timers = NULL;
gui_img_t *hg_image_1766997304841_dyi9 = NULL;
gui_text_t *hg_labelalarm_clock = NULL;
gui_img_t *hg_image_1766997310775_8v5d = NULL;
gui_text_t *hg_label_audio_books = NULL;
gui_img_t *hg_image_1766997321655_ab2h = NULL;
gui_text_t *hg_label_workout = NULL;
gui_img_t *hg_image_1766997330778_kikg = NULL;
gui_text_t *hg_label_compass = NULL;
gui_img_t *hg_image_1766997333608_19bg = NULL;
gui_text_t *hg_label_calendar = NULL;
gui_img_t *hg_image_1766997342134_pyhh = NULL;
gui_text_t *hg_label_home = NULL;
gui_img_t *hg_image_1766997352181_95r6 = NULL;
gui_text_t *hg_label_contacts = NULL;
gui_img_t *hg_image_1766997358698_flgg = NULL;
gui_text_t *hg_label_cycle_tracking = NULL;
gui_text_t *hg_label_mail = NULL;
gui_img_t *hg_image_1770687396151_f48s = NULL;
gui_img_t *hg_image_1766997373065_by77 = NULL;
gui_text_t *hg_label_now_playing = NULL;
gui_img_t *hg_image_1766997382728_xzti = NULL;
gui_text_t *hg_label_news = NULL;
gui_img_t *hg_image_1766997390336_u4nl = NULL;
gui_text_t *hg_label_photo = NULL;
gui_img_t *hg_image_1766997396077_p2br = NULL;
gui_text_t *hg_label_podcasts = NULL;
gui_img_t *hg_image_1766997407159_stbu = NULL;
gui_text_t *hg_label_sleep = NULL;
gui_img_t *img_1 = NULL;
gui_text_t *hg_label_videocall = NULL;
gui_img_t *img_2 = NULL;
gui_text_t *hg_label_walkie_talkie = NULL;
gui_text_t *hg_label_control_center = NULL;
gui_img_t *img_10 = NULL;
gui_text_t *hg_label_recording = NULL;
gui_img_t *img_12 = NULL;
gui_text_t *hg_label_music_player = NULL;
gui_img_t *img_11 = NULL;
gui_text_t *hg_label_ota = NULL;
gui_img_t *img_8 = NULL;
gui_img_t *img_13 = NULL;
gui_img_t *img_14 = NULL;
gui_text_t *lbl_1 = NULL;

// List component note_design callback functions
// note_design callback function declaration
static void app_menu_list_note_design(gui_obj_t *obj, void *param);

// note_design callback function implementation
static void app_menu_list_note_design(gui_obj_t *obj, void *param)
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
            // Create app_menu_activity_icon (hg_image)
            app_menu_activity_icon = gui_img_create_from_fs((gui_obj_t *)note, "app_menu_activity_icon",
                                                            "/UI_app_list/app_activity_icon.bin", 36, -6, 100, 106);
            // Create app_menu_activity_text (hg_label)
            app_menu_activity_text = gui_text_create((gui_obj_t *)note, "app_menu_activity_text", 160, 29, 130,
                                                     52);
            gui_text_set((gui_text_t *)app_menu_activity_text, "Activity", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 8, 32);
            gui_text_type_set((gui_text_t *)app_menu_activity_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)app_menu_activity_text, LEFT);
            gui_obj_add_event_cb(obj, app_menu_list_item_1_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 1:
        {
            // Create hg_image_1766997222913_68q3 (hg_image)
            hg_image_1766997222913_68q3 = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1766997222913_68q3", "/UI_app_list/app_heart_rate_icon.bin", 36, 0, 100, 100);
            gui_obj_add_event_cb(hg_image_1766997222913_68q3,
                                 (gui_event_cb_t)hg_image_1766997222913_68q3_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create app_menu_heart_text (hg_label)
            app_menu_heart_text = gui_text_create((gui_obj_t *)note, "app_menu_heart_text", 160, 29, 177, 52);
            gui_text_set((gui_text_t *)app_menu_heart_text, "Heart Rate", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 10, 32);
            gui_text_type_set((gui_text_t *)app_menu_heart_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)app_menu_heart_text, LEFT);
            gui_obj_add_event_cb(obj, app_menu_list_item_2_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 2:
        {
            // Create hg_image_1766997230487_yrdp (hg_image)
            hg_image_1766997230487_yrdp = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1766997230487_yrdp", "/UI_app_list/app_weather_icon.bin", 36, 0, 100, 100);
            gui_obj_add_event_cb(hg_image_1766997230487_yrdp,
                                 (gui_event_cb_t)hg_image_1766997230487_yrdp_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create app_menu_weather_text (hg_label)
            app_menu_weather_text = gui_text_create((gui_obj_t *)note, "app_menu_weather_text", 160, 29, 177,
                                                    52);
            gui_text_set((gui_text_t *)app_menu_weather_text, "Weather", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 7, 32);
            gui_text_type_set((gui_text_t *)app_menu_weather_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)app_menu_weather_text, LEFT);
            gui_obj_add_event_cb(obj, app_menu_list_item_3_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 3:
        {
            // Create hg_image_1766997236346_95l4 (hg_image)
            hg_image_1766997236346_95l4 = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1766997236346_95l4", "/UI_app_list/app_reminders_icon.bin", 36, 0, 100, 100);
            gui_obj_add_event_cb(hg_image_1766997236346_95l4,
                                 (gui_event_cb_t)hg_image_1766997236346_95l4_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create app_menu_reminders_text (hg_label)
            app_menu_reminders_text = gui_text_create((gui_obj_t *)note, "app_menu_reminders_text", 160, 29,
                                                      177, 52);
            gui_text_set((gui_text_t *)app_menu_reminders_text, "Reminders", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 9, 32);
            gui_text_type_set((gui_text_t *)app_menu_reminders_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)app_menu_reminders_text, LEFT);
            gui_obj_add_event_cb(obj, app_menu_list_item_4_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 4:
        {
            // Create hg_image_1766997242674_bw37 (hg_image)
            hg_image_1766997242674_bw37 = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1766997242674_bw37", "/UI_app_list/app_message_icon.bin", 36, 0, 100, 100);
            gui_obj_add_event_cb(hg_image_1766997242674_bw37,
                                 (gui_event_cb_t)hg_image_1766997242674_bw37_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create app_menu_message_text (hg_label)
            app_menu_message_text = gui_text_create((gui_obj_t *)note, "app_menu_message_text", 160, 29, 177,
                                                    52);
            gui_text_set((gui_text_t *)app_menu_message_text, "Message", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 7, 32);
            gui_text_type_set((gui_text_t *)app_menu_message_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)app_menu_message_text, LEFT);
            gui_obj_add_event_cb(obj, app_menu_list_item_5_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 5:
        {
            // Create hg_image_1766997251460_wgoz (hg_image)
            hg_image_1766997251460_wgoz = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1766997251460_wgoz", "/UI_app_list/app_phone_icon.bin", 36, 0, 100, 100);
            gui_obj_add_event_cb(hg_image_1766997251460_wgoz,
                                 (gui_event_cb_t)hg_image_1766997251460_wgoz_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create app_menu_phone_text (hg_label)
            app_menu_phone_text = gui_text_create((gui_obj_t *)note, "app_menu_phone_text", 160, 29, 177, 52);
            gui_text_set((gui_text_t *)app_menu_phone_text, "Phone", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                         5, 32);
            gui_text_type_set((gui_text_t *)app_menu_phone_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)app_menu_phone_text, LEFT);
            gui_obj_add_event_cb(obj, app_menu_list_item_6_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 6:
        {
            // Create hg_image_noise (hg_image)
            hg_image_noise = gui_img_create_from_fs((gui_obj_t *)note, "hg_image_noise",
                                                    "/UI_app_list/app_noise_icon.bin", 36, 0, 100, 100);
            gui_obj_add_event_cb(hg_image_noise, (gui_event_cb_t)hg_image_noise_clicked_cb,
                                 GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create app_menu_noise_text (hg_label)
            app_menu_noise_text = gui_text_create((gui_obj_t *)note, "app_menu_noise_text", 160, 29, 177, 52);
            gui_text_set((gui_text_t *)app_menu_noise_text, "Noise", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                         5, 32);
            gui_text_type_set((gui_text_t *)app_menu_noise_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)app_menu_noise_text, LEFT);
            gui_obj_add_event_cb(obj, app_menu_list_item_7_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 7:
        {
            // Create hg_image_1766997263436_j1j0 (hg_image)
            hg_image_1766997263436_j1j0 = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1766997263436_j1j0", "/UI_app_list/app_music_icon.bin", 36, 0, 100, 100);
            gui_obj_add_event_cb(hg_image_1766997263436_j1j0,
                                 (gui_event_cb_t)hg_image_1766997263436_j1j0_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create app_menu_music_text (hg_label)
            app_menu_music_text = gui_text_create((gui_obj_t *)note, "app_menu_music_text", 160, 29, 177, 52);
            gui_text_set((gui_text_t *)app_menu_music_text, "Music", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                         5, 32);
            gui_text_type_set((gui_text_t *)app_menu_music_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)app_menu_music_text, LEFT);
            gui_obj_add_event_cb(obj, app_menu_list_item_8_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 8:
        {
            // Create hg_image_1766997276115_kywq (hg_image)
            hg_image_1766997276115_kywq = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1766997276115_kywq", "/UI_app_list/app_map_icon.bin", 36, 0, 100, 100);
            gui_obj_add_event_cb(hg_image_1766997276115_kywq,
                                 (gui_event_cb_t)hg_image_1766997276115_kywq_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create hg_label_map (hg_label)
            hg_label_map = gui_text_create((gui_obj_t *)note, "hg_label_map", 160, 29, 177, 52);
            gui_text_set((gui_text_t *)hg_label_map, "Map", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 3, 32);
            gui_text_type_set((gui_text_t *)hg_label_map, "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_map, LEFT);
            gui_obj_add_event_cb(obj, app_menu_list_item_9_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 9:
        {
            // Create hg_image_1766997286123_263r (hg_image)
            hg_image_1766997286123_263r = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1766997286123_263r", "/UI_app_list/app_stopwatch_icon.bin", 36, 0, 100, 100);
            // Create hg_label_stopwatch (hg_label)
            hg_label_stopwatch = gui_text_create((gui_obj_t *)note, "hg_label_stopwatch", 160, 29, 177, 52);
            gui_text_set((gui_text_t *)hg_label_stopwatch, "Stopwatch", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 9, 32);
            gui_text_type_set((gui_text_t *)hg_label_stopwatch,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_stopwatch, LEFT);
            gui_obj_add_event_cb(obj, app_menu_list_item_10_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 10:
        {
            // Create hg_image_1766997291073_d8qk (hg_image)
            hg_image_1766997291073_d8qk = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1766997291073_d8qk", "/UI_app_list/app_timers_icon.bin", 36, 0, 100, 100);
            // Create hg_label_timers (hg_label)
            hg_label_timers = gui_text_create((gui_obj_t *)note, "hg_label_timers", 160, 29, 177, 52);
            gui_text_set((gui_text_t *)hg_label_timers, "Timers", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 6,
                         32);
            gui_text_type_set((gui_text_t *)hg_label_timers, "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_timers, LEFT);
            gui_obj_add_event_cb(obj, app_menu_list_item_11_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 11:
        {
            // Create hg_image_1766997304841_dyi9 (hg_image)
            hg_image_1766997304841_dyi9 = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1766997304841_dyi9", "/UI_app_list/app_alarm_clock_icon.bin", 36, 0, 100, 100);
            // Create hg_labelalarm_clock (hg_label)
            hg_labelalarm_clock = gui_text_create((gui_obj_t *)note, "hg_labelalarm_clock", 160, 29, 197, 52);
            gui_text_set((gui_text_t *)hg_labelalarm_clock, "Alarm Clock", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 11, 32);
            gui_text_type_set((gui_text_t *)hg_labelalarm_clock,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_labelalarm_clock, LEFT);
            gui_obj_add_event_cb(obj, app_menu_list_item_12_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 12:
        {
            // Create hg_image_1766997310775_8v5d (hg_image)
            hg_image_1766997310775_8v5d = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1766997310775_8v5d", "/UI_app_list/app_audio_books_icon.bin", 36, 0, 100, 100);
            // Create hg_label_audio_books (hg_label)
            hg_label_audio_books = gui_text_create((gui_obj_t *)note, "hg_label_audio_books", 160, 29, 203, 52);
            gui_text_set((gui_text_t *)hg_label_audio_books, "Audio Books", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 11, 32);
            gui_text_type_set((gui_text_t *)hg_label_audio_books,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_audio_books, LEFT);
            gui_obj_add_event_cb(obj, app_menu_list_item_13_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 13:
        {
            // Create hg_image_1766997321655_ab2h (hg_image)
            hg_image_1766997321655_ab2h = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1766997321655_ab2h", "/UI_app_list/app_workout_icon.bin", 36, 0, 100, 100);
            // Create hg_label_workout (hg_label)
            hg_label_workout = gui_text_create((gui_obj_t *)note, "hg_label_workout", 160, 29, 177, 52);
            gui_text_set((gui_text_t *)hg_label_workout, "Workout", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 7,
                         32);
            gui_text_type_set((gui_text_t *)hg_label_workout,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_workout, LEFT);
            gui_obj_add_event_cb(obj, app_menu_list_item_14_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 14:
        {
            // Create hg_image_1766997330778_kikg (hg_image)
            hg_image_1766997330778_kikg = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1766997330778_kikg", "/UI_app_list/app_compass_icon.bin", 36, 0, 100, 100);
            // Create hg_label_compass (hg_label)
            hg_label_compass = gui_text_create((gui_obj_t *)note, "hg_label_compass", 160, 29, 177, 52);
            gui_text_set((gui_text_t *)hg_label_compass, "Compass", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 7,
                         32);
            gui_text_type_set((gui_text_t *)hg_label_compass,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_compass, LEFT);
            gui_obj_add_event_cb(obj, app_menu_list_item_15_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 15:
        {
            // Create hg_image_1766997333608_19bg (hg_image)
            hg_image_1766997333608_19bg = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1766997333608_19bg", "/UI_app_list/app_calendar_icon.bin", 36, 0, 100, 100);
            // Create hg_label_calendar (hg_label)
            hg_label_calendar = gui_text_create((gui_obj_t *)note, "hg_label_calendar", 160, 29, 177, 52);
            gui_text_set((gui_text_t *)hg_label_calendar, "Calendar", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                         8, 32);
            gui_text_type_set((gui_text_t *)hg_label_calendar,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_calendar, LEFT);
            gui_obj_add_event_cb(obj, app_menu_list_item_16_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 16:
        {
            // Create hg_image_1766997342134_pyhh (hg_image)
            hg_image_1766997342134_pyhh = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1766997342134_pyhh", "/UI_app_list/app_home_icon.bin", 36, 0, 100, 100);
            // Create hg_label_home (hg_label)
            hg_label_home = gui_text_create((gui_obj_t *)note, "hg_label_home", 160, 29, 177, 52);
            gui_text_set((gui_text_t *)hg_label_home, "Home", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 4, 32);
            gui_text_type_set((gui_text_t *)hg_label_home, "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_home, LEFT);
            gui_obj_add_event_cb(obj, app_menu_list_item_17_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 17:
        {
            // Create hg_image_1766997352181_95r6 (hg_image)
            hg_image_1766997352181_95r6 = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1766997352181_95r6", "/UI_app_list/app_contacts_icon.bin", 36, 0, 100, 100);
            // Create hg_label_contacts (hg_label)
            hg_label_contacts = gui_text_create((gui_obj_t *)note, "hg_label_contacts", 160, 29, 177, 52);
            gui_text_set((gui_text_t *)hg_label_contacts, "Contacts", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                         8, 32);
            gui_text_type_set((gui_text_t *)hg_label_contacts,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_contacts, LEFT);
            gui_obj_add_event_cb(obj, app_menu_list_item_18_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 18:
        {
            // Create hg_image_1766997358698_flgg (hg_image)
            hg_image_1766997358698_flgg = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1766997358698_flgg", "/UI_app_list/app_cycle_tracking_icon.bin", 36, 0, 100, 100);
            // Create hg_label_cycle_tracking (hg_label)
            hg_label_cycle_tracking = gui_text_create((gui_obj_t *)note, "hg_label_cycle_tracking", 160, 24,
                                                      230, 52);
            gui_text_set((gui_text_t *)hg_label_cycle_tracking, "Cycle Tracking", GUI_FONT_SRC_BMP, gui_rgb(255,
                         255, 255), 14, 32);
            gui_text_type_set((gui_text_t *)hg_label_cycle_tracking,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_cycle_tracking, LEFT);
            gui_obj_add_event_cb(obj, app_menu_list_item_19_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 19:
        {
            // Create hg_label_mail (hg_label)
            hg_label_mail = gui_text_create((gui_obj_t *)note, "hg_label_mail", 160, 29, 188, 52);
            gui_text_set((gui_text_t *)hg_label_mail, "Music Scan", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                         10, 32);
            gui_text_type_set((gui_text_t *)hg_label_mail, "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_mail, LEFT);
            // Create hg_image_1770687396151_f48s (hg_image)
            hg_image_1770687396151_f48s = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1770687396151_f48s", "/UI_app_list/music_scan_icon.bin", 36, 0, 100, 100);
            gui_obj_add_event_cb(obj, app_menu_list_item_20_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 20:
        {
            // Create hg_image_1766997373065_by77 (hg_image)
            hg_image_1766997373065_by77 = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1766997373065_by77", "/UI_app_list/app_now_playing_icon.bin", 36, 0, 100, 100);
            // Create hg_label_now_playing (hg_label)
            hg_label_now_playing = gui_text_create((gui_obj_t *)note, "hg_label_now_playing", 160, 29, 200, 52);
            gui_text_set((gui_text_t *)hg_label_now_playing, "Now Playing", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 11, 32);
            gui_text_type_set((gui_text_t *)hg_label_now_playing,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_now_playing, LEFT);
            gui_obj_add_event_cb(obj, app_menu_list_item_21_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 21:
        {
            // Create hg_image_1766997382728_xzti (hg_image)
            hg_image_1766997382728_xzti = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1766997382728_xzti", "/UI_app_list/app_news_icon.bin", 36, 0, 100, 100);
            // Create hg_label_news (hg_label)
            hg_label_news = gui_text_create((gui_obj_t *)note, "hg_label_news", 160, 29, 177, 52);
            gui_text_set((gui_text_t *)hg_label_news, "News", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 4, 32);
            gui_text_type_set((gui_text_t *)hg_label_news, "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_news, LEFT);
            gui_obj_add_event_cb(obj, app_menu_list_item_22_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 22:
        {
            // Create hg_image_1766997390336_u4nl (hg_image)
            hg_image_1766997390336_u4nl = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1766997390336_u4nl", "/UI_app_list/app_photo_icon.bin", 36, 0, 100, 100);
            // Create hg_label_photo (hg_label)
            hg_label_photo = gui_text_create((gui_obj_t *)note, "hg_label_photo", 160, 29, 177, 52);
            gui_text_set((gui_text_t *)hg_label_photo, "Photo", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                         32);
            gui_text_type_set((gui_text_t *)hg_label_photo, "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_photo, LEFT);
            gui_obj_add_event_cb(obj, app_menu_list_item_23_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 23:
        {
            // Create hg_image_1766997396077_p2br (hg_image)
            hg_image_1766997396077_p2br = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1766997396077_p2br", "/UI_app_list/app_podcasts_icon.bin", 36, 0, 100, 100);
            // Create hg_label_podcasts (hg_label)
            hg_label_podcasts = gui_text_create((gui_obj_t *)note, "hg_label_podcasts", 160, 29, 177, 52);
            gui_text_set((gui_text_t *)hg_label_podcasts, "Podcasts", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                         8, 32);
            gui_text_type_set((gui_text_t *)hg_label_podcasts,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_podcasts, LEFT);
            gui_obj_add_event_cb(obj, app_menu_list_item_24_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 24:
        {
            // Create hg_image_1766997407159_stbu (hg_image)
            hg_image_1766997407159_stbu = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1766997407159_stbu", "/UI_app_list/app_sleep_icon.bin", 36, 0, 100, 100);
            // Create hg_label_sleep (hg_label)
            hg_label_sleep = gui_text_create((gui_obj_t *)note, "hg_label_sleep", 160, 29, 177, 52);
            gui_text_set((gui_text_t *)hg_label_sleep, "Sleep", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                         32);
            gui_text_type_set((gui_text_t *)hg_label_sleep, "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_sleep, LEFT);
            gui_obj_add_event_cb(obj, app_menu_list_item_25_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 25:
        {
            // Create img_1 (hg_image)
            img_1 = gui_img_create_from_fs((gui_obj_t *)note, "img_1", "/UI_app_list/app_camera.bin", 36, 0,
                                           100, 100);
            // Create hg_label_videocall (hg_label)
            hg_label_videocall = gui_text_create((gui_obj_t *)note, "hg_label_videocall", 160, 29, 177, 52);
            gui_text_set((gui_text_t *)hg_label_videocall, "Video Call", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 10, 32);
            gui_text_type_set((gui_text_t *)hg_label_videocall,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_videocall, LEFT);
            gui_obj_add_event_cb(obj, app_menu_list_item_27_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 26:
        {
            // Create img_2 (hg_image)
            img_2 = gui_img_create_from_fs((gui_obj_t *)note, "img_2", "/UI_app_list/walkie-talkie.bin", 36, 0,
                                           100, 100);
            // Create hg_label_walkie_talkie (hg_label)
            hg_label_walkie_talkie = gui_text_create((gui_obj_t *)note, "hg_label_walkie_talkie", 160, 29, 210,
                                                     52);
            gui_text_set((gui_text_t *)hg_label_walkie_talkie, "Walkie Talkie", GUI_FONT_SRC_BMP, gui_rgb(255,
                         255, 255), 13, 32);
            gui_text_type_set((gui_text_t *)hg_label_walkie_talkie,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_walkie_talkie, LEFT);
            gui_obj_add_event_cb(obj, app_menu_list_item_28_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 27:
        {
            // Create hg_label_control_center (hg_label)
            hg_label_control_center = gui_text_create((gui_obj_t *)note, "hg_label_control_center", 160, 29,
                                                      220, 52);
            gui_text_set((gui_text_t *)hg_label_control_center, "Control Center", GUI_FONT_SRC_BMP, gui_rgb(255,
                         255, 255), 14, 32);
            gui_text_type_set((gui_text_t *)hg_label_control_center,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_control_center, LEFT);
            // Create img_10 (hg_image)
            img_10 = gui_img_create_from_fs((gui_obj_t *)note, "img_10", "/UI_app_list/app_control_board.bin",
                                            36, 0, 100, 100);
            gui_obj_add_event_cb(obj, app_menu_list_item_29_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 28:
        {
            // Create hg_label_recording (hg_label)
            hg_label_recording = gui_text_create((gui_obj_t *)note, "hg_label_recording", 160, 29, 177, 52);
            gui_text_set((gui_text_t *)hg_label_recording, "Recording", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 9, 32);
            gui_text_type_set((gui_text_t *)hg_label_recording,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_recording, LEFT);
            // Create img_12 (hg_image)
            img_12 = gui_img_create_from_fs((gui_obj_t *)note, "img_12", "/UI_app_list/recording_icon.bin", 36,
                                            0, 100, 100);
            gui_obj_add_event_cb(obj, app_menu_list_item_intercom_switch_view_cb, GUI_EVENT_TOUCH_CLICKED,
                                 NULL);
            break;
        }
    case 29:
        {
            // Create hg_label_music_player (hg_label)
            hg_label_music_player = gui_text_create((gui_obj_t *)note, "hg_label_music_player", 160, 29, 208,
                                                    52);
            gui_text_set((gui_text_t *)hg_label_music_player, "Music Player", GUI_FONT_SRC_BMP, gui_rgb(255,
                         255, 255), 12, 32);
            gui_text_type_set((gui_text_t *)hg_label_music_player,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_music_player, LEFT);
            // Create img_11 (hg_image)
            img_11 = gui_img_create_from_fs((gui_obj_t *)note, "img_11", "/UI_app_list/app_music_icon.bin", 36,
                                            0, 100, 100);
            gui_obj_add_event_cb(obj, app_menu_list_item_music_player_switch_view_cb, GUI_EVENT_TOUCH_CLICKED,
                                 NULL);
            break;
        }
    case 30:
        {
            // Create hg_label_ota (hg_label)
            hg_label_ota = gui_text_create((gui_obj_t *)note, "hg_label_ota", 160, 29, 177, 52);
            gui_text_set((gui_text_t *)hg_label_ota, "OTA", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 3, 32);
            gui_text_type_set((gui_text_t *)hg_label_ota, "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_ota, LEFT);
            // Create img_8 (hg_image)
            img_8 = gui_img_create_from_fs((gui_obj_t *)note, "img_8", "/UI_app_list/OTA.bin", 36, 0, 100, 100);
            gui_obj_add_event_cb(obj, app_menu_list_item_ota_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 31:
        {
            // Create img_13 (hg_image)
            img_13 = gui_img_create_from_fs((gui_obj_t *)note, "img_13", "/tab_bg/bottom_list_menu_bg.bin", 68,
                                            12, 258, 76);
            gui_img_set_mode((gui_img_t *)img_13, IMG_2D_SW_FIX_A8_FG);
            gui_img_a8_recolor((gui_img_t *)img_13, 0xFFFFFFFF);
            gui_img_set_opacity((gui_img_t *)img_13, 167);
            gui_obj_add_event_cb(img_13, (gui_event_cb_t)img_13_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create img_14 (hg_image)
            img_14 = gui_img_create_from_fs((gui_obj_t *)note, "img_14", "/UI_app_list/cellular_icon.bin", 83,
                                            27, 48, 49);
            // Create lbl_1 (hg_label)
            lbl_1 = gui_text_create((gui_obj_t *)note, "lbl_1", 135, 30, 166, 42);
            gui_text_set((gui_text_t *)lbl_1, "Cellular Menu", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 13,
                         24);
            gui_text_type_set((gui_text_t *)lbl_1, "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)lbl_1, MID_LEFT);
            gui_obj_add_event_cb(obj, app_menu_list_item_26_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    default:
        break;
    }
}


// Create app_menu_view (hg_view)
static void app_menu_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_menu_view_switch_in(gui_view_t *view)
{
    // Set animation step
    gui_view_set_animate_step(view, 50);

    // Set opacity
    gui_view_set_opacity(view, 255);

    // Set background color
    gui_set_bg_color(gui_rgb(0, 0, 0));

    GUI_UNUSED(view);


    // Create app_menu_list (hg_list)
    app_menu_list = gui_list_create((gui_obj_t *)view, "app_menu_list", 0, 0, 410, 502, 100, 5,
                                    VERTICAL, app_menu_list_note_design, NULL, false);
    gui_list_set_style(app_menu_list, LIST_CIRCLE);
    gui_list_set_note_num(app_menu_list, 32);
    gui_list_set_circle_radius(app_menu_list, 820);
    gui_list_keep_note_alive(app_menu_list, true);
    // Bind timer: Animation 1
    gui_obj_create_timer((gui_obj_t *)app_menu_list, 10, true, app_menu_list_timer_0_cb);

    // Create app_menu_window (hg_window)
    app_menu_window = gui_win_create((gui_obj_t *)view, "app_menu_window", 0, 0, 410, 502);

    gui_obj_add_event_cb(GUI_BASE(app_menu_window), (gui_event_cb_t)app_menu_window_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)app_menu_window);

    gui_obj_add_event_cb((gui_obj_t *)view, (gui_event_cb_t)app_menu_view_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)view);
}
GUI_VIEW_INSTANCE("app_menu_view", false, app_menu_view_switch_in, app_menu_view_switch_out, false);
