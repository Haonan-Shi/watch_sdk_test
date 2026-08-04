/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_weather UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-06T09:11:40.978Z
 */
#include "app_weather_ui.h"
#include "../callbacks/app_weather_callbacks.h"
#include "../user/app_weather_user.h"
#include <stddef.h>
#include <time.h>

// Component handle definitions
gui_list_t *hg_list_weather = NULL;
gui_img_t *hg_image_weather_bg1 = NULL;
gui_text_t *hg_label_city = NULL;
gui_text_t *hg_label_temperature1 = NULL;
gui_text_t *hg_label_temperature_degree1 = NULL;
gui_text_t *hg_label_weather_condition1 = NULL;
gui_text_t *hg_label_temperature_range = NULL;
gui_img_t *hg_image_weather_row = NULL;
gui_img_t *hg_image_weather_circle = NULL;
gui_text_t *hg_label_UVI_value = NULL;
gui_text_t *hg_label_AQI_value = NULL;
gui_text_t *hg_label_UVI = NULL;
gui_text_t *hg_label_AQI = NULL;
gui_text_t *hg_label_MPH_value = NULL;
gui_text_t *hg_label_MPH = NULL;
gui_arc_t *hg_arc_UVI = NULL;
gui_arc_t *hg_arc_AQI = NULL;
gui_circle_t *hg_circle_UVI = NULL;
gui_circle_t *hg_circle_AQI = NULL;
gui_img_t *hg_image_weather_bg2 = NULL;
gui_img_t *hg_image_weather_ring = NULL;
gui_text_t *hg_label_temperature_range2 = NULL;
gui_text_t *hg_label_temperature2 = NULL;
gui_text_t *hg_label_temperature_degree2 = NULL;
gui_img_t *hg_image_weather_condition12 = NULL;
gui_img_t *hg_image_weather_condition1 = NULL;
gui_img_t *hg_image_weather_condition2 = NULL;
gui_img_t *hg_image_weather_condition3 = NULL;
gui_img_t *hg_image_weather_condition4 = NULL;
gui_img_t *hg_image_weather_condition5 = NULL;
gui_img_t *hg_image_weather_condition6 = NULL;
gui_img_t *hg_image_weather_condition7 = NULL;
gui_img_t *hg_image_weather_condition8 = NULL;
gui_img_t *hg_image_weather_condition9 = NULL;
gui_img_t *hg_image_weather_condition10 = NULL;
gui_img_t *hg_image_weather_condition11 = NULL;
gui_text_t *hg_label_weather_condition2 = NULL;
gui_img_t *hg_image_weather_bg3 = NULL;
gui_win_t *hg_window_weather_forecast = NULL;
gui_win_t *hg_window_weather1 = NULL;
gui_text_t *hg_label_weather_today = NULL;
gui_img_t *hg_image_weather_sun1 = NULL;
gui_img_t *hg_image_weather_line1 = NULL;
gui_text_t *hg_label_temperature_low1 = NULL;
gui_text_t *hg_label_temperature_high1 = NULL;
gui_win_t *hg_window_weather2 = NULL;
gui_text_t *hg_label_weather_thu = NULL;
gui_img_t *hg_image_weather_line2 = NULL;
gui_text_t *hg_label_temperature_low2 = NULL;
gui_text_t *hg_label_temperature_high2 = NULL;
gui_img_t *hg_image_weather_cloudy = NULL;
gui_win_t *hg_window_weather3 = NULL;
gui_text_t *hg_label_weather_fri = NULL;
gui_img_t *hg_image_weather_sun2 = NULL;
gui_img_t *hg_image_weather_line3 = NULL;
gui_text_t *hg_label_temperature_low3 = NULL;
gui_text_t *hg_label_temperature_high3 = NULL;
gui_win_t *hg_window_weather4 = NULL;
gui_text_t *hg_label_weather_sat = NULL;
gui_img_t *hg_image_weather_sun3 = NULL;
gui_img_t *hg_image_weather_line4 = NULL;
gui_text_t *hg_label_temperature_low4 = NULL;
gui_text_t *hg_label_temperature_high4 = NULL;
gui_win_t *hg_window_weather5 = NULL;
gui_text_t *hg_label_weather_sunday = NULL;
gui_img_t *hg_image_weather_sun4 = NULL;
gui_img_t *hg_image_weather_line5 = NULL;
gui_text_t *hg_label_temperature_low5 = NULL;
gui_text_t *hg_label_temperature_high5 = NULL;
gui_win_t *hg_window_weather6 = NULL;
gui_text_t *hg_label_weather_mon = NULL;
gui_img_t *hg_image_weather_line6 = NULL;
gui_text_t *hg_label_temperature_low6 = NULL;
gui_text_t *hg_label_temperature_high6 = NULL;
gui_img_t *hg_image_weather_windy1 = NULL;
gui_win_t *hg_window_weather7 = NULL;
gui_text_t *hg_label_weather_tue = NULL;
gui_img_t *hg_image_weather_line7 = NULL;
gui_text_t *hg_label_temperature_low7 = NULL;
gui_text_t *hg_label_temperature_high7 = NULL;
gui_img_t *hg_image_weather_windy2 = NULL;
gui_win_t *hg_window_weather_top = NULL;
gui_img_t *hg_image_weather_function = NULL;
gui_img_t *hg_image_weather_right = NULL;
gui_text_t *hg_time_label_weather = NULL;

// Time string global variables
char hg_time_label_weather_time_str[10] = {0};

// List component note_design callback functions
// note_design callback function declaration
static void hg_list_weather_note_design(gui_obj_t *obj, void *param);

// note_design callback function implementation
static void hg_list_weather_note_design(gui_obj_t *obj, void *param)
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
            // Create hg_image_weather_bg1 (hg_image)
            hg_image_weather_bg1 = gui_img_create_from_fs((gui_obj_t *)note, "hg_image_weather_bg1",
                                                          "/weather/weather_background1.bin", 0, 0, 410, 502);
            // Create hg_label_city (hg_label)
            hg_label_city = gui_text_create((gui_obj_t *)note, "hg_label_city", 130, 145, 150, 45);
            gui_text_set((gui_text_t *)hg_label_city, "Sonoma", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 6,
                         40);
            gui_text_type_set((gui_text_t *)hg_label_city, "/font/Inter_24pt_Regular_size40_bits2_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_city, MID_CENTER);
            // Create hg_label_temperature1 (hg_label)
            hg_label_temperature1 = gui_text_create((gui_obj_t *)note, "hg_label_temperature1", 130, 187, 150,
                                                    105);
            gui_text_set((gui_text_t *)hg_label_temperature1, "76", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 2,
                         90);
            gui_text_type_set((gui_text_t *)hg_label_temperature1,
                              "/font/Inter_24pt_Regular_size90_bits2_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_temperature1, MID_CENTER);
            // Create hg_label_temperature_degree1 (hg_label)
            hg_label_temperature_degree1 = gui_text_create((gui_obj_t *)note, "hg_label_temperature_degree1",
                                                           186, 179, 177, 105);
            gui_text_set((gui_text_t *)hg_label_temperature_degree1, "°", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 2, 80);
            gui_text_type_set((gui_text_t *)hg_label_temperature_degree1,
                              "/font/Inter_24pt_Regular_size80_bits2_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_temperature_degree1, MID_CENTER);
            // Create hg_label_weather_condition1 (hg_label)
            hg_label_weather_condition1 = gui_text_create((gui_obj_t *)note, "hg_label_weather_condition1", 130,
                                                          276, 150, 45);
            gui_text_set((gui_text_t *)hg_label_weather_condition1, "Sunny", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 5, 33);
            gui_text_type_set((gui_text_t *)hg_label_weather_condition1,
                              "/font/Inter_24pt_Regular_size33_bits2_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_weather_condition1, MID_CENTER);
            // Create hg_label_temperature_range (hg_label)
            hg_label_temperature_range = gui_text_create((gui_obj_t *)note, "hg_label_temperature_range", 0,
                                                         316, 410, 45);
            gui_text_set((gui_text_t *)hg_label_temperature_range, "H:88° L:57°", GUI_FONT_SRC_BMP,
                         gui_rgb(255, 255, 255), 13, 33);
            gui_text_type_set((gui_text_t *)hg_label_temperature_range,
                              "/font/Inter_24pt_Regular_size33_bits2_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_temperature_range, MID_CENTER);
            // Create hg_image_weather_row (hg_image)
            hg_image_weather_row = gui_img_create_from_fs((gui_obj_t *)note, "hg_image_weather_row",
                                                          "/weather/Wind_Arrow.bin", 166, 419, 78, 34);
            // Create hg_image_weather_circle (hg_image)
            hg_image_weather_circle = gui_img_create_from_fs((gui_obj_t *)note, "hg_image_weather_circle",
                                                             "/weather/Wind_Blur.bin", 185, 419, 41, 41);
            // Create hg_label_UVI_value (hg_label)
            hg_label_UVI_value = gui_text_create((gui_obj_t *)note, "hg_label_UVI_value", 16, 417, 50, 45);
            gui_text_set((gui_text_t *)hg_label_UVI_value, "4", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 1,
                         31);
            gui_text_type_set((gui_text_t *)hg_label_UVI_value,
                              "/font/Inter_24pt_Regular_size31_bits2_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_UVI_value, MID_CENTER);
            // Create hg_label_AQI_value (hg_label)
            hg_label_AQI_value = gui_text_create((gui_obj_t *)note, "hg_label_AQI_value", 328, 417, 50, 45);
            gui_text_set((gui_text_t *)hg_label_AQI_value, "32", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 2,
                         31);
            gui_text_type_set((gui_text_t *)hg_label_AQI_value,
                              "/font/Inter_24pt_Regular_size31_bits2_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_AQI_value, MID_CENTER);
            // Create hg_label_UVI (hg_label)
            hg_label_UVI = gui_text_create((gui_obj_t *)note, "hg_label_UVI", -34, 441, 150, 45);
            gui_text_set((gui_text_t *)hg_label_UVI, "UVI", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 3, 16);
            gui_text_type_set((gui_text_t *)hg_label_UVI, "/font/Inter_24pt_Regular_size16_bits2_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_UVI, MID_CENTER);
            // Create hg_label_AQI (hg_label)
            hg_label_AQI = gui_text_create((gui_obj_t *)note, "hg_label_AQI", 278, 441, 150, 45);
            gui_text_set((gui_text_t *)hg_label_AQI, "AQI", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 3, 16);
            gui_text_type_set((gui_text_t *)hg_label_AQI, "/font/Inter_24pt_Regular_size16_bits2_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_AQI, MID_CENTER);
            // Create hg_label_MPH_value (hg_label)
            hg_label_MPH_value = gui_text_create((gui_obj_t *)note, "hg_label_MPH_value", 131, 408, 150, 45);
            gui_text_set((gui_text_t *)hg_label_MPH_value, "7", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 1,
                         18);
            gui_text_type_set((gui_text_t *)hg_label_MPH_value,
                              "/font/Inter_24pt_Regular_size18_bits2_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_MPH_value, MID_CENTER);
            // Create hg_label_MPH (hg_label)
            hg_label_MPH = gui_text_create((gui_obj_t *)note, "hg_label_MPH", 131, 425, 150, 45);
            gui_text_set((gui_text_t *)hg_label_MPH, "MPH", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 3, 10);
            gui_text_type_set((gui_text_t *)hg_label_MPH, "/font/Inter_24pt_Regular_size10_bits2_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_MPH, MID_CENTER);
            // Create hg_arc_UVI (hg_arc)
            hg_arc_UVI = gui_arc_create((gui_obj_t *)note, "hg_arc_UVI", 42, 448, 34, 150, 30, 8, gui_rgba(255,
                                        255, 255, 178));
            // Create hg_arc_AQI (hg_arc)
            hg_arc_AQI = gui_arc_create((gui_obj_t *)note, "hg_arc_AQI", 354, 448, 34, 150, 30, 8, gui_rgba(255,
                                        255, 255, 178));
            // Create hg_circle_UVI (hg_circle)
            hg_circle_UVI = gui_circle_create((gui_obj_t *)note, "hg_circle_UVI", 21, 421, 5, gui_rgb(255, 255,
                                              255));
            // Create hg_circle_AQI (hg_circle)
            hg_circle_AQI = gui_circle_create((gui_obj_t *)note, "hg_circle_AQI", 333, 421, 5, gui_rgb(255, 255,
                                              255));
            break;
        }
    case 1:
        {
            // Create hg_image_weather_bg2 (hg_image)
            hg_image_weather_bg2 = gui_img_create_from_fs((gui_obj_t *)note, "hg_image_weather_bg2",
                                                          "/weather/weather_background2.bin", 0, 0, 410, 502);
            // Create hg_image_weather_ring (hg_image)
            hg_image_weather_ring = gui_img_create_from_fs((gui_obj_t *)note, "hg_image_weather_ring",
                                                           "/weather/weather_dial.bin", 22, 72, 366, 361);
            // Create hg_label_temperature_range2 (hg_label)
            hg_label_temperature_range2 = gui_text_create((gui_obj_t *)note, "hg_label_temperature_range2", 130,
                                                          266, 150, 45);
            gui_text_set((gui_text_t *)hg_label_temperature_range2, "H:88° L:57°", GUI_FONT_SRC_BMP,
                         gui_rgb(255, 255, 255), 13, 23);
            gui_text_type_set((gui_text_t *)hg_label_temperature_range2,
                              "/font/Inter_24pt_Regular_size23_bits2_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_temperature_range2, MID_CENTER);
            // Create hg_label_temperature2 (hg_label)
            hg_label_temperature2 = gui_text_create((gui_obj_t *)note, "hg_label_temperature2", 130, 175, 150,
                                                    105);
            gui_text_set((gui_text_t *)hg_label_temperature2, "76", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 2,
                         83);
            gui_text_type_set((gui_text_t *)hg_label_temperature2,
                              "/font/Inter_24pt_Regular_size83_bits2_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_temperature2, MID_CENTER);
            // Create hg_label_temperature_degree2 (hg_label)
            hg_label_temperature_degree2 = gui_text_create((gui_obj_t *)note, "hg_label_temperature_degree2",
                                                           193, 175, 150, 105);
            gui_text_set((gui_text_t *)hg_label_temperature_degree2, "°", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 2, 80);
            gui_text_type_set((gui_text_t *)hg_label_temperature_degree2,
                              "/font/Inter_24pt_Regular_size80_bits2_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_temperature_degree2, MID_CENTER);
            // Create hg_image_weather_condition12 (hg_image)
            hg_image_weather_condition12 = gui_img_create_from_fs((gui_obj_t *)note,
                                                                  "hg_image_weather_condition12", "/weather/weather_condition_partly_cloudy.bin", 183, 115, 44, 31);
            // Create hg_image_weather_condition1 (hg_image)
            hg_image_weather_condition1 = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_weather_condition1", "/weather/weather_condition_partly_cloudy.bin", 253, 133, 44, 31);
            // Create hg_image_weather_condition2 (hg_image)
            hg_image_weather_condition2 = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_weather_condition2", "/weather/weather_condition_sunny.bin", 297, 174, 33, 33);
            // Create hg_image_weather_condition3 (hg_image)
            hg_image_weather_condition3 = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_weather_condition3", "/weather/weather_condition_sunny.bin", 313, 237, 33, 33);
            // Create hg_image_weather_condition4 (hg_image)
            hg_image_weather_condition4 = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_weather_condition4", "/weather/weather_condition_windy.bin", 297, 307, 33, 29);
            // Create hg_image_weather_condition5 (hg_image)
            hg_image_weather_condition5 = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_weather_condition5", "/weather/weather_condition_windy.bin", 259, 345, 33, 29);
            // Create hg_image_weather_condition6 (hg_image)
            hg_image_weather_condition6 = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_weather_condition6", "/weather/weather_condition_partly_cloudy.bin", 183, 358, 44, 31);
            // Create hg_image_weather_condition7 (hg_image)
            hg_image_weather_condition7 = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_weather_condition7", "/weather/weather_condition_partly_cloudy.bin", 118, 344, 44, 31);
            // Create hg_image_weather_condition8 (hg_image)
            hg_image_weather_condition8 = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_weather_condition8", "/weather/weather_condition_cloudy.bin", 86, 309, 39, 26);
            // Create hg_image_weather_condition9 (hg_image)
            hg_image_weather_condition9 = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_weather_condition9", "/weather/weather_condition_cloudy.bin", 66, 240, 39, 26);
            // Create hg_image_weather_condition10 (hg_image)
            hg_image_weather_condition10 = gui_img_create_from_fs((gui_obj_t *)note,
                                                                  "hg_image_weather_condition10", "/weather/weather_condition_sunny.bin", 83, 170, 33, 33);
            // Create hg_image_weather_condition11 (hg_image)
            hg_image_weather_condition11 = gui_img_create_from_fs((gui_obj_t *)note,
                                                                  "hg_image_weather_condition11", "/weather/weather_condition_sunny.bin", 129, 126, 33, 33);
            // Create hg_label_weather_condition2 (hg_label)
            hg_label_weather_condition2 = gui_text_create((gui_obj_t *)note, "hg_label_weather_condition2", 129,
                                                          441, 150, 45);
            gui_text_set((gui_text_t *)hg_label_weather_condition2, "Sunny", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 5, 29);
            gui_text_type_set((gui_text_t *)hg_label_weather_condition2,
                              "/font/Inter_24pt_Regular_size29_bits2_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_weather_condition2, MID_CENTER);
            break;
        }
    case 2:
        {
            // Create hg_image_weather_bg3 (hg_image)
            hg_image_weather_bg3 = gui_img_create_from_fs((gui_obj_t *)note, "hg_image_weather_bg3",
                                                          "/weather/weather_background3.bin", 0, 0, 410, 502);
            // Create hg_window_weather_forecast (hg_window)
            hg_window_weather_forecast = gui_win_create((gui_obj_t *)note, "hg_window_weather_forecast", 0, 160,
                                                        410, 502);

            // Create hg_window_weather1 (hg_window)
            hg_window_weather1 = gui_win_create((gui_obj_t *)hg_window_weather_forecast, "hg_window_weather1",
                                                0, 0, 410, 76);

            // Create hg_label_weather_today (hg_label)
            hg_label_weather_today = gui_text_create((gui_obj_t *)hg_window_weather1, "hg_label_weather_today",
                                                     25, 0, 150, 45);
            gui_text_set((gui_text_t *)hg_label_weather_today, "Today", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 5, 31);
            gui_text_type_set((gui_text_t *)hg_label_weather_today,
                              "/font/Inter_24pt_Regular_size31_bits2_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_weather_today, MID_LEFT);
            // Create hg_image_weather_sun1 (hg_image)
            hg_image_weather_sun1 = gui_img_create_from_fs((gui_obj_t *)hg_window_weather1,
                                                           "hg_image_weather_sun1", "/weather/weather_condition_sunny.bin", 185, 0, 33, 33);
            // Create hg_image_weather_line1 (hg_image)
            hg_image_weather_line1 = gui_img_create_from_fs((gui_obj_t *)hg_window_weather1,
                                                            "hg_image_weather_line1", "/weather/weather_separator.bin", 25, 45, 360, 2);
            // Create hg_label_temperature_low1 (hg_label)
            hg_label_temperature_low1 = gui_text_create((gui_obj_t *)hg_window_weather1,
                                                        "hg_label_temperature_low1", 300, 0, 50, 45);
            gui_text_set((gui_text_t *)hg_label_temperature_low1, "57°", GUI_FONT_SRC_BMP, gui_rgb(142, 185,
                         241), 4, 31);
            gui_text_type_set((gui_text_t *)hg_label_temperature_low1,
                              "/font/Inter_24pt_Regular_size31_bits2_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_temperature_low1, MID_LEFT);
            // Create hg_label_temperature_high1 (hg_label)
            hg_label_temperature_high1 = gui_text_create((gui_obj_t *)hg_window_weather1,
                                                         "hg_label_temperature_high1", 350, 0, 50, 45);
            gui_text_set((gui_text_t *)hg_label_temperature_high1, "88°", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 4, 31);
            gui_text_type_set((gui_text_t *)hg_label_temperature_high1,
                              "/font/Inter_24pt_Regular_size31_bits2_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_temperature_high1, MID_LEFT);
            // Create hg_window_weather2 (hg_window)
            hg_window_weather2 = gui_win_create((gui_obj_t *)hg_window_weather_forecast, "hg_window_weather2",
                                                0, 76, 410, 76);

            // Create hg_label_weather_thu (hg_label)
            hg_label_weather_thu = gui_text_create((gui_obj_t *)hg_window_weather2, "hg_label_weather_thu", 25,
                                                   0, 150, 45);
            gui_text_set((gui_text_t *)hg_label_weather_thu, "Thu", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 3,
                         31);
            gui_text_type_set((gui_text_t *)hg_label_weather_thu,
                              "/font/Inter_24pt_Regular_size31_bits2_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_weather_thu, MID_LEFT);
            // Create hg_image_weather_line2 (hg_image)
            hg_image_weather_line2 = gui_img_create_from_fs((gui_obj_t *)hg_window_weather2,
                                                            "hg_image_weather_line2", "/weather/weather_separator.bin", 25, 45, 360, 2);
            // Create hg_label_temperature_low2 (hg_label)
            hg_label_temperature_low2 = gui_text_create((gui_obj_t *)hg_window_weather2,
                                                        "hg_label_temperature_low2", 300, 0, 50, 45);
            gui_text_set((gui_text_t *)hg_label_temperature_low2, "60°", GUI_FONT_SRC_BMP, gui_rgb(142, 185,
                         241), 4, 31);
            gui_text_type_set((gui_text_t *)hg_label_temperature_low2,
                              "/font/Inter_24pt_Regular_size31_bits2_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_temperature_low2, MID_LEFT);
            // Create hg_label_temperature_high2 (hg_label)
            hg_label_temperature_high2 = gui_text_create((gui_obj_t *)hg_window_weather2,
                                                         "hg_label_temperature_high2", 350, 0, 50, 45);
            gui_text_set((gui_text_t *)hg_label_temperature_high2, "75°", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 4, 31);
            gui_text_type_set((gui_text_t *)hg_label_temperature_high2,
                              "/font/Inter_24pt_Regular_size31_bits2_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_temperature_high2, MID_LEFT);
            // Create hg_image_weather_cloudy (hg_image)
            hg_image_weather_cloudy = gui_img_create_from_fs((gui_obj_t *)hg_window_weather2,
                                                             "hg_image_weather_cloudy", "/weather/weather_condition_partly_cloudy.bin", 183, 0, 44, 31);
            // Create hg_window_weather3 (hg_window)
            hg_window_weather3 = gui_win_create((gui_obj_t *)hg_window_weather_forecast, "hg_window_weather3",
                                                0, 152, 410, 76);

            // Create hg_label_weather_fri (hg_label)
            hg_label_weather_fri = gui_text_create((gui_obj_t *)hg_window_weather3, "hg_label_weather_fri", 25,
                                                   0, 150, 45);
            gui_text_set((gui_text_t *)hg_label_weather_fri, "Fri", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 3,
                         31);
            gui_text_type_set((gui_text_t *)hg_label_weather_fri,
                              "/font/Inter_24pt_Regular_size31_bits2_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_weather_fri, MID_LEFT);
            // Create hg_image_weather_sun2 (hg_image)
            hg_image_weather_sun2 = gui_img_create_from_fs((gui_obj_t *)hg_window_weather3,
                                                           "hg_image_weather_sun2", "/weather/weather_condition_sunny.bin", 185, 0, 33, 33);
            // Create hg_image_weather_line3 (hg_image)
            hg_image_weather_line3 = gui_img_create_from_fs((gui_obj_t *)hg_window_weather3,
                                                            "hg_image_weather_line3", "/weather/weather_separator.bin", 25, 45, 360, 2);
            // Create hg_label_temperature_low3 (hg_label)
            hg_label_temperature_low3 = gui_text_create((gui_obj_t *)hg_window_weather3,
                                                        "hg_label_temperature_low3", 300, 0, 50, 45);
            gui_text_set((gui_text_t *)hg_label_temperature_low3, "51°", GUI_FONT_SRC_BMP, gui_rgb(142, 185,
                         241), 4, 31);
            gui_text_type_set((gui_text_t *)hg_label_temperature_low3,
                              "/font/Inter_24pt_Regular_size31_bits2_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_temperature_low3, MID_LEFT);
            // Create hg_label_temperature_high3 (hg_label)
            hg_label_temperature_high3 = gui_text_create((gui_obj_t *)hg_window_weather3,
                                                         "hg_label_temperature_high3", 350, 0, 50, 45);
            gui_text_set((gui_text_t *)hg_label_temperature_high3, "65°", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 4, 31);
            gui_text_type_set((gui_text_t *)hg_label_temperature_high3,
                              "/font/Inter_24pt_Regular_size31_bits2_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_temperature_high3, MID_LEFT);
            // Create hg_window_weather4 (hg_window)
            hg_window_weather4 = gui_win_create((gui_obj_t *)hg_window_weather_forecast, "hg_window_weather4",
                                                0, 228, 410, 76);

            // Create hg_label_weather_sat (hg_label)
            hg_label_weather_sat = gui_text_create((gui_obj_t *)hg_window_weather4, "hg_label_weather_sat", 25,
                                                   0, 150, 45);
            gui_text_set((gui_text_t *)hg_label_weather_sat, "Sat", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 3,
                         31);
            gui_text_type_set((gui_text_t *)hg_label_weather_sat,
                              "/font/Inter_24pt_Regular_size31_bits2_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_weather_sat, MID_LEFT);
            // Create hg_image_weather_sun3 (hg_image)
            hg_image_weather_sun3 = gui_img_create_from_fs((gui_obj_t *)hg_window_weather4,
                                                           "hg_image_weather_sun3", "/weather/weather_condition_sunny.bin", 185, 0, 33, 33);
            // Create hg_image_weather_line4 (hg_image)
            hg_image_weather_line4 = gui_img_create_from_fs((gui_obj_t *)hg_window_weather4,
                                                            "hg_image_weather_line4", "/weather/weather_separator.bin", 25, 45, 360, 2);
            // Create hg_label_temperature_low4 (hg_label)
            hg_label_temperature_low4 = gui_text_create((gui_obj_t *)hg_window_weather4,
                                                        "hg_label_temperature_low4", 300, 0, 50, 45);
            gui_text_set((gui_text_t *)hg_label_temperature_low4, "64°", GUI_FONT_SRC_BMP, gui_rgb(142, 185,
                         241), 4, 31);
            gui_text_type_set((gui_text_t *)hg_label_temperature_low4,
                              "/font/Inter_24pt_Regular_size31_bits2_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_temperature_low4, MID_LEFT);
            // Create hg_label_temperature_high4 (hg_label)
            hg_label_temperature_high4 = gui_text_create((gui_obj_t *)hg_window_weather4,
                                                         "hg_label_temperature_high4", 350, 0, 50, 45);
            gui_text_set((gui_text_t *)hg_label_temperature_high4, "90°", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 4, 31);
            gui_text_type_set((gui_text_t *)hg_label_temperature_high4,
                              "/font/Inter_24pt_Regular_size31_bits2_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_temperature_high4, MID_LEFT);
            // Create hg_window_weather5 (hg_window)
            hg_window_weather5 = gui_win_create((gui_obj_t *)hg_window_weather_forecast, "hg_window_weather5",
                                                0, 304, 410, 76);

            // Create hg_label_weather_sunday (hg_label)
            hg_label_weather_sunday = gui_text_create((gui_obj_t *)hg_window_weather5,
                                                      "hg_label_weather_sunday", 25, 0, 150, 45);
            gui_text_set((gui_text_t *)hg_label_weather_sunday, "Sun", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                         3, 31);
            gui_text_type_set((gui_text_t *)hg_label_weather_sunday,
                              "/font/Inter_24pt_Regular_size31_bits2_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_weather_sunday, MID_LEFT);
            // Create hg_image_weather_sun4 (hg_image)
            hg_image_weather_sun4 = gui_img_create_from_fs((gui_obj_t *)hg_window_weather5,
                                                           "hg_image_weather_sun4", "/weather/weather_condition_sunny.bin", 185, 0, 33, 33);
            // Create hg_image_weather_line5 (hg_image)
            hg_image_weather_line5 = gui_img_create_from_fs((gui_obj_t *)hg_window_weather5,
                                                            "hg_image_weather_line5", "/weather/weather_separator.bin", 25, 45, 360, 2);
            // Create hg_label_temperature_low5 (hg_label)
            hg_label_temperature_low5 = gui_text_create((gui_obj_t *)hg_window_weather5,
                                                        "hg_label_temperature_low5", 300, 0, 50, 45);
            gui_text_set((gui_text_t *)hg_label_temperature_low5, "68°", GUI_FONT_SRC_BMP, gui_rgb(142, 185,
                         241), 4, 31);
            gui_text_type_set((gui_text_t *)hg_label_temperature_low5,
                              "/font/Inter_24pt_Regular_size31_bits2_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_temperature_low5, MID_LEFT);
            // Create hg_label_temperature_high5 (hg_label)
            hg_label_temperature_high5 = gui_text_create((gui_obj_t *)hg_window_weather5,
                                                         "hg_label_temperature_high5", 350, 0, 50, 45);
            gui_text_set((gui_text_t *)hg_label_temperature_high5, "91°", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 4, 31);
            gui_text_type_set((gui_text_t *)hg_label_temperature_high5,
                              "/font/Inter_24pt_Regular_size31_bits2_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_temperature_high5, MID_LEFT);
            // Create hg_window_weather6 (hg_window)
            hg_window_weather6 = gui_win_create((gui_obj_t *)hg_window_weather_forecast, "hg_window_weather6",
                                                0, 380, 410, 76);

            // Create hg_label_weather_mon (hg_label)
            hg_label_weather_mon = gui_text_create((gui_obj_t *)hg_window_weather6, "hg_label_weather_mon", 25,
                                                   0, 150, 45);
            gui_text_set((gui_text_t *)hg_label_weather_mon, "Mon", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 3,
                         31);
            gui_text_type_set((gui_text_t *)hg_label_weather_mon,
                              "/font/Inter_24pt_Regular_size31_bits2_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_weather_mon, MID_LEFT);
            // Create hg_image_weather_line6 (hg_image)
            hg_image_weather_line6 = gui_img_create_from_fs((gui_obj_t *)hg_window_weather6,
                                                            "hg_image_weather_line6", "/weather/weather_separator.bin", 25, 45, 360, 2);
            // Create hg_label_temperature_low6 (hg_label)
            hg_label_temperature_low6 = gui_text_create((gui_obj_t *)hg_window_weather6,
                                                        "hg_label_temperature_low6", 300, 0, 50, 45);
            gui_text_set((gui_text_t *)hg_label_temperature_low6, "55°", GUI_FONT_SRC_BMP, gui_rgb(142, 185,
                         241), 4, 31);
            gui_text_type_set((gui_text_t *)hg_label_temperature_low6,
                              "/font/Inter_24pt_Regular_size31_bits2_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_temperature_low6, MID_LEFT);
            // Create hg_label_temperature_high6 (hg_label)
            hg_label_temperature_high6 = gui_text_create((gui_obj_t *)hg_window_weather6,
                                                         "hg_label_temperature_high6", 350, 0, 50, 45);
            gui_text_set((gui_text_t *)hg_label_temperature_high6, "65°", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 4, 31);
            gui_text_type_set((gui_text_t *)hg_label_temperature_high6,
                              "/font/Inter_24pt_Regular_size31_bits2_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_temperature_high6, MID_LEFT);
            // Create hg_image_weather_windy1 (hg_image)
            hg_image_weather_windy1 = gui_img_create_from_fs((gui_obj_t *)hg_window_weather6,
                                                             "hg_image_weather_windy1", "/weather/weather_condition_windy.bin", 189, 8, 33, 29);
            // Create hg_window_weather7 (hg_window)
            hg_window_weather7 = gui_win_create((gui_obj_t *)hg_window_weather_forecast, "hg_window_weather7",
                                                0, 456, 410, 76);

            // Create hg_label_weather_tue (hg_label)
            hg_label_weather_tue = gui_text_create((gui_obj_t *)hg_window_weather7, "hg_label_weather_tue", 25,
                                                   0, 150, 45);
            gui_text_set((gui_text_t *)hg_label_weather_tue, "Tue", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 3,
                         31);
            gui_text_type_set((gui_text_t *)hg_label_weather_tue,
                              "/font/Inter_24pt_Regular_size31_bits2_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_weather_tue, MID_LEFT);
            // Create hg_image_weather_line7 (hg_image)
            hg_image_weather_line7 = gui_img_create_from_fs((gui_obj_t *)hg_window_weather7,
                                                            "hg_image_weather_line7", "/weather/weather_separator.bin", 25, 45, 360, 2);
            // Create hg_label_temperature_low7 (hg_label)
            hg_label_temperature_low7 = gui_text_create((gui_obj_t *)hg_window_weather7,
                                                        "hg_label_temperature_low7", 300, 0, 50, 45);
            gui_text_set((gui_text_t *)hg_label_temperature_low7, "52°", GUI_FONT_SRC_BMP, gui_rgb(142, 185,
                         241), 4, 31);
            gui_text_type_set((gui_text_t *)hg_label_temperature_low7,
                              "/font/Inter_24pt_Regular_size31_bits2_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_temperature_low7, MID_LEFT);
            // Create hg_label_temperature_high7 (hg_label)
            hg_label_temperature_high7 = gui_text_create((gui_obj_t *)hg_window_weather7,
                                                         "hg_label_temperature_high7", 350, 0, 50, 45);
            gui_text_set((gui_text_t *)hg_label_temperature_high7, "66°", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 4, 31);
            gui_text_type_set((gui_text_t *)hg_label_temperature_high7,
                              "/font/Inter_24pt_Regular_size31_bits2_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_temperature_high7, MID_LEFT);
            // Create hg_image_weather_windy2 (hg_image)
            hg_image_weather_windy2 = gui_img_create_from_fs((gui_obj_t *)hg_window_weather7,
                                                             "hg_image_weather_windy2", "/weather/weather_condition_windy.bin", 189, 8, 33, 29);
            break;
        }
    default:
        break;
    }
}


// Create app_weatherMainView (hg_view)
static void app_weatherMainView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_weatherMainView_switch_in(gui_view_t *view)
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



    // Create hg_list_weather (hg_list)
    hg_list_weather = gui_list_create((gui_obj_t *)view, "hg_list_weather", 0, 0, 410, 502, 502, 0,
                                      VERTICAL, hg_list_weather_note_design, NULL, false);
    gui_list_set_style(hg_list_weather, LIST_FADE);
    gui_list_set_note_num(hg_list_weather, 3);
    gui_list_set_auto_align(hg_list_weather, true);
    gui_list_set_inertia(hg_list_weather, false);

    // Create hg_window_weather_top (hg_window)
    hg_window_weather_top = gui_win_create((gui_obj_t *)view, "hg_window_weather_top", 0, 0, 410, 100);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(hg_time_label_weather_time_str, sizeof(hg_time_label_weather_time_str), "%02d:%02d",
                 t->tm_hour, t->tm_min);
    }


    // Create hg_image_weather_function (hg_image)
    hg_image_weather_function = gui_img_create_from_fs(hg_window_weather_top,
                                                       "hg_image_weather_function", "/weather/weather_left_control.bin", 13, 14, 75, 75);

    // Create hg_image_weather_right (hg_image)
    hg_image_weather_right = gui_img_create_from_fs(hg_window_weather_top, "hg_image_weather_right",
                                                    "/weather/weather_right_control.bin", 322, 14, 75, 75);

    // Create hg_time_label_weather (hg_time_label)
    hg_time_label_weather = gui_text_create(hg_window_weather_top, "hg_time_label_weather", 155, 34,
                                            120, 35);
    gui_text_set((gui_text_t *)hg_time_label_weather, hg_time_label_weather_time_str, GUI_FONT_SRC_BMP,
                 gui_rgb(255, 255, 255), strlen(hg_time_label_weather_time_str), 32);
    gui_text_type_set((gui_text_t *)hg_time_label_weather,
                      "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_time_label_weather, MID_CENTER);

    gui_obj_add_event_cb(GUI_BASE(hg_window_weather_top),
                         (gui_event_cb_t)hg_window_weather_top_key_0_cb, GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)hg_window_weather_top);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(hg_time_label_weather), 30000, true,
                         hg_time_label_weather_time_update_cb);
}
GUI_VIEW_INSTANCE("app_weatherMainView", false, app_weatherMainView_switch_in,
                  app_weatherMainView_switch_out, false);
