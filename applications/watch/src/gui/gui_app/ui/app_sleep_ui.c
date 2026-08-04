/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_sleep UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-06T09:11:40.948Z
 */
#include "app_sleep_ui.h"
#include "../callbacks/app_sleep_callbacks.h"
#include "../user/app_sleep_user.h"
#include <stddef.h>
#include <time.h>

// Component handle definitions
gui_rounded_rect_t *sleep_bg = NULL;
gui_list_t *sleep_list = NULL;
gui_img_t *hg_image_1769657738653_01u9 = NULL;
gui_text_t *sleep_list0_text0 = NULL;
gui_circle_t *sleep_awake_icon = NULL;
gui_circle_t *sleep_REM_icon = NULL;
gui_circle_t *sleep_core_icon = NULL;
gui_circle_t *sleep_deep_icon = NULL;
gui_text_t *sleep_list0_awake_text = NULL;
gui_text_t *sleep_list0_rem_text = NULL;
gui_text_t *sleep_list0_core_text = NULL;
gui_text_t *sleep_list0_deep_text = NULL;
gui_text_t *sleep_list0_awakeTime_text = NULL;
gui_text_t *sleep_list0_remTime_text = NULL;
gui_text_t *sleep_list0_coreTime_text = NULL;
gui_text_t *sleep_list0_deepTime_text = NULL;
gui_text_t *sleep_stages_text = NULL;
gui_rounded_rect_t *sleep_chart_bar2 = NULL;
gui_text_t *sleep_timeSleep_text = NULL;
gui_rounded_rect_t *sleep_chart_bar0 = NULL;
gui_rounded_rect_t *sleep_chart_bar1 = NULL;
gui_rounded_rect_t *sleep_chart_bar4 = NULL;
gui_rounded_rect_t *sleep_chart_bar3 = NULL;
gui_text_t *sleep_chart_time0_text = NULL;
gui_text_t *sleep_chart_time1_text = NULL;
gui_text_t *sleep_chart_time2_text = NULL;
gui_text_t *sleep_chart_time3_text = NULL;
gui_text_t *sleep_chart_time4_text = NULL;
gui_rounded_rect_t *sleep_chart_bar5 = NULL;
gui_rounded_rect_t *sleep_chart_rect1 = NULL;
gui_rounded_rect_t *sleep_chart_rect2 = NULL;
gui_text_t *sleep_list1_sleepTime_text = NULL;
gui_text_t *sleep_list1_sleepTime_text_uite = NULL;
gui_text_t *sleep_list1_sleepTime_text0 = NULL;
gui_text_t *sleep_list1_sleepTime_text1 = NULL;
gui_text_t *sleep_list1_sleepTime_text_uite1 = NULL;
gui_text_t *sleep_list1_sleepTime_text_copy_1769738523896 = NULL;
gui_rounded_rect_t *sleep_chart_rect0 = NULL;
gui_img_t *sleep_char_bg0 = NULL;
gui_img_t *sleep_char_bg1 = NULL;
gui_rounded_rect_t *sleep_char_data0 = NULL;
gui_rounded_rect_t *sleep_char_data1 = NULL;
gui_rounded_rect_t *sleep_char_data2 = NULL;
gui_rounded_rect_t *sleep_char_data3 = NULL;
gui_rounded_rect_t *sleep_char_data4 = NULL;
gui_rounded_rect_t *sleep_char_data5 = NULL;
gui_rounded_rect_t *sleep_char_data6 = NULL;
gui_rounded_rect_t *sleep_char_topLine0 = NULL;
gui_rounded_rect_t *sleep_char_data7 = NULL;
gui_rounded_rect_t *sleep_char_data8 = NULL;
gui_rounded_rect_t *sleep_char_data9 = NULL;
gui_rounded_rect_t *sleep_char_data10 = NULL;
gui_rounded_rect_t *sleep_char_data11 = NULL;
gui_rounded_rect_t *sleep_char_data12 = NULL;
gui_rounded_rect_t *sleep_char_data13 = NULL;
gui_rounded_rect_t *sleep_char_topLine1 = NULL;
gui_text_t *sleep_list2_sleepTime_text0 = NULL;
gui_text_t *sleep_list2_sleepTime_text_uite0 = NULL;
gui_text_t *sleep_list2_sleepTime_text1 = NULL;
gui_text_t *sleep_list2_sleepTime_text_uite1 = NULL;
gui_text_t *sleep_list2_sleepTime_text2 = NULL;
gui_text_t *sleep_list2_sleepTime_text_uite2 = NULL;
gui_text_t *sleep_list2_sleepTime_text3 = NULL;
gui_text_t *sleep_list2_sleepTime_text_uite3 = NULL;
gui_text_t *hg_label_1769741973751_t3b1 = NULL;
gui_text_t *hg_label_1769741973751_t3b1_copy_1769742407966 = NULL;
gui_text_t *sleep_duration_text = NULL;
gui_text_t *sleep_consistent_text = NULL;
gui_text_t *sleep_last_7_days_text = NULL;
gui_text_t *sleep_nextSleep_text = NULL;
gui_text_t *sleep_next_alarm_bedtime_text0 = NULL;
gui_text_t *sleep_next_alarm_bedtime_text = NULL;
gui_text_t *sleep_next_alarm_tomorrow_text = NULL;
gui_img_t *sleep_alarm_working_icon = NULL;
gui_text_t *sleep_next_alarm_wakeup_text0 = NULL;
gui_text_t *sleep_next_alarm_wakeup_text = NULL;
gui_text_t *sleep_next_alarm_tomorrow_text1 = NULL;
gui_img_t *sleep_next_alarm_icon = NULL;
gui_text_t *sleep_nextSchedule_text = NULL;
gui_win_t *sleep_top_window = NULL;
gui_text_t *hg_time_label_1769654821849_4rcr = NULL;
gui_img_t *hg_image_1769654877646_xx8b = NULL;
gui_img_t *hg_image_1769654928940_2nzg = NULL;

// Time string global variables
char hg_time_label_1769654821849_4rcr_time_str[10] = {0};

// List component note_design callback functions
// note_design callback function declaration
static void sleep_list_note_design(gui_obj_t *obj, void *param);

// note_design callback function implementation
static void sleep_list_note_design(gui_obj_t *obj, void *param)
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
            // Create hg_image_1769657738653_01u9 (hg_image)
            hg_image_1769657738653_01u9 = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1769657738653_01u9", "/app_sleep/sleep_Bars_Container.bin", 36, 46, 171, 50);
            gui_img_scale((gui_img_t *)hg_image_1769657738653_01u9, 2.000000f, 2.000000f);
            // Create sleep_list0_text0 (hg_label)
            sleep_list0_text0 = gui_text_create((gui_obj_t *)note, "sleep_list0_text0", 36, 161, 100, 40);
            gui_text_set((gui_text_t *)sleep_list0_text0, "SEP 12", GUI_FONT_SRC_BMP, gui_rgb(120, 120, 120), 6,
                         30);
            gui_text_type_set((gui_text_t *)sleep_list0_text0,
                              "/font/Inter_24pt_Regular_size30_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_list0_text0, LEFT);
            // Create sleep_awake_icon (hg_circle)
            sleep_awake_icon = gui_circle_create((gui_obj_t *)note, "sleep_awake_icon", 54, 214, 18,
                                                 gui_rgb(238, 124, 97));
            // Create sleep_REM_icon (hg_circle)
            sleep_REM_icon = gui_circle_create((gui_obj_t *)note, "sleep_REM_icon", 54, 264, 18, gui_rgb(128,
                                               207, 250));
            // Create sleep_core_icon (hg_circle)
            sleep_core_icon = gui_circle_create((gui_obj_t *)note, "sleep_core_icon", 54, 313, 18, gui_rgb(59,
                                                130, 247));
            // Create sleep_deep_icon (hg_circle)
            sleep_deep_icon = gui_circle_create((gui_obj_t *)note, "sleep_deep_icon", 54, 364, 18, gui_rgb(66,
                                                49, 199));
            // Create sleep_list0_awake_text (hg_label)
            sleep_list0_awake_text = gui_text_create((gui_obj_t *)note, "sleep_list0_awake_text", 94, 200, 100,
                                                     40);
            gui_text_set((gui_text_t *)sleep_list0_awake_text, "Awake", GUI_FONT_SRC_BMP, gui_rgb(240, 240,
                         240), 5, 30);
            gui_text_type_set((gui_text_t *)sleep_list0_awake_text,
                              "/font/Inter_24pt_Regular_size30_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_list0_awake_text, LEFT);
            // Create sleep_list0_rem_text (hg_label)
            sleep_list0_rem_text = gui_text_create((gui_obj_t *)note, "sleep_list0_rem_text", 94, 250, 100, 40);
            gui_text_set((gui_text_t *)sleep_list0_rem_text, "REM", GUI_FONT_SRC_BMP, gui_rgb(240, 240, 240), 3,
                         30);
            gui_text_type_set((gui_text_t *)sleep_list0_rem_text,
                              "/font/Inter_24pt_Regular_size30_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_list0_rem_text, LEFT);
            // Create sleep_list0_core_text (hg_label)
            sleep_list0_core_text = gui_text_create((gui_obj_t *)note, "sleep_list0_core_text", 94, 299, 100,
                                                    40);
            gui_text_set((gui_text_t *)sleep_list0_core_text, "Core", GUI_FONT_SRC_BMP, gui_rgb(240, 240, 240),
                         4, 30);
            gui_text_type_set((gui_text_t *)sleep_list0_core_text,
                              "/font/Inter_24pt_Regular_size30_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_list0_core_text, LEFT);
            // Create sleep_list0_deep_text (hg_label)
            sleep_list0_deep_text = gui_text_create((gui_obj_t *)note, "sleep_list0_deep_text", 94, 350, 100,
                                                    40);
            gui_text_set((gui_text_t *)sleep_list0_deep_text, "Deep", GUI_FONT_SRC_BMP, gui_rgb(240, 240, 240),
                         4, 30);
            gui_text_type_set((gui_text_t *)sleep_list0_deep_text,
                              "/font/Inter_24pt_Regular_size30_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_list0_deep_text, LEFT);
            // Create sleep_list0_awakeTime_text (hg_label)
            sleep_list0_awakeTime_text = gui_text_create((gui_obj_t *)note, "sleep_list0_awakeTime_text", 266,
                                                         200, 124, 40);
            gui_text_set((gui_text_t *)sleep_list0_awakeTime_text, "6m", GUI_FONT_SRC_BMP, gui_rgb(240, 240,
                         240), 2, 30);
            gui_text_type_set((gui_text_t *)sleep_list0_awakeTime_text,
                              "/font/Inter_24pt_Regular_size30_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_list0_awakeTime_text, RIGHT);
            // Create sleep_list0_remTime_text (hg_label)
            sleep_list0_remTime_text = gui_text_create((gui_obj_t *)note, "sleep_list0_remTime_text", 253, 250,
                                                       137, 40);
            gui_text_set((gui_text_t *)sleep_list0_remTime_text, "1h 45m", GUI_FONT_SRC_BMP, gui_rgb(240, 240,
                         240), 6, 30);
            gui_text_type_set((gui_text_t *)sleep_list0_remTime_text,
                              "/font/Inter_24pt_Regular_size30_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_list0_remTime_text, RIGHT);
            // Create sleep_list0_coreTime_text (hg_label)
            sleep_list0_coreTime_text = gui_text_create((gui_obj_t *)note, "sleep_list0_coreTime_text", 226,
                                                        299, 164, 40);
            gui_text_set((gui_text_t *)sleep_list0_coreTime_text, "4h 48m", GUI_FONT_SRC_BMP, gui_rgb(240, 240,
                         240), 6, 30);
            gui_text_type_set((gui_text_t *)sleep_list0_coreTime_text,
                              "/font/Inter_24pt_Regular_size30_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_list0_coreTime_text, RIGHT);
            // Create sleep_list0_deepTime_text (hg_label)
            sleep_list0_deepTime_text = gui_text_create((gui_obj_t *)note, "sleep_list0_deepTime_text", 252,
                                                        350, 138, 40);
            gui_text_set((gui_text_t *)sleep_list0_deepTime_text, "37m", GUI_FONT_SRC_BMP, gui_rgb(240, 240,
                         240), 3, 30);
            gui_text_type_set((gui_text_t *)sleep_list0_deepTime_text,
                              "/font/Inter_24pt_Regular_size30_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_list0_deepTime_text, RIGHT);
            // Create sleep_stages_text (hg_label)
            sleep_stages_text = gui_text_create((gui_obj_t *)note, "sleep_stages_text", 186, 2, 200, 40);
            gui_text_set((gui_text_t *)sleep_stages_text, "Sleep Stages", GUI_FONT_SRC_BMP, gui_rgb(94, 92,
                         223), 12, 32);
            gui_text_type_set((gui_text_t *)sleep_stages_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_stages_text, RIGHT);
            break;
        }
    case 1:
        {
            // Create sleep_chart_bar2 (hg_rect)
            sleep_chart_bar2 = gui_rect_create((gui_obj_t *)note, "sleep_chart_bar2", 173, 137, 3, 140, 0,
                                               gui_rgb(128, 128, 128));
            // Create sleep_timeSleep_text (hg_label)
            sleep_timeSleep_text = gui_text_create((gui_obj_t *)note, "sleep_timeSleep_text", 187, 2, 200, 40);
            gui_text_set((gui_text_t *)sleep_timeSleep_text, "Time Asleep", GUI_FONT_SRC_BMP, gui_rgb(94, 92,
                         223), 11, 32);
            gui_text_type_set((gui_text_t *)sleep_timeSleep_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_timeSleep_text, RIGHT);
            // Create sleep_chart_bar0 (hg_rect)
            sleep_chart_bar0 = gui_rect_create((gui_obj_t *)note, "sleep_chart_bar0", 30, 137, 3, 140, 0,
                                               gui_rgb(128, 128, 128));
            // Create sleep_chart_bar1 (hg_rect)
            sleep_chart_bar1 = gui_rect_create((gui_obj_t *)note, "sleep_chart_bar1", 100, 137, 3, 140, 0,
                                               gui_rgb(128, 128, 128));
            // Create sleep_chart_bar4 (hg_rect)
            sleep_chart_bar4 = gui_rect_create((gui_obj_t *)note, "sleep_chart_bar4", 315, 137, 3, 140, 0,
                                               gui_rgb(128, 128, 128));
            // Create sleep_chart_bar3 (hg_rect)
            sleep_chart_bar3 = gui_rect_create((gui_obj_t *)note, "sleep_chart_bar3", 244, 137, 3, 140, 0,
                                               gui_rgb(128, 128, 128));
            // Create sleep_chart_time0_text (hg_label)
            sleep_chart_time0_text = gui_text_create((gui_obj_t *)note, "sleep_chart_time0_text", 33, 260, 60,
                                                     34);
            gui_text_set((gui_text_t *)sleep_chart_time0_text, "10 PM", GUI_FONT_SRC_BMP, gui_rgb(128, 128,
                         128), 5, 20);
            gui_text_type_set((gui_text_t *)sleep_chart_time0_text,
                              "/font/Inter_24pt_Regular_size20_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_chart_time0_text, LEFT);
            // Create sleep_chart_time1_text (hg_label)
            sleep_chart_time1_text = gui_text_create((gui_obj_t *)note, "sleep_chart_time1_text", 103, 260, 60,
                                                     34);
            gui_text_set((gui_text_t *)sleep_chart_time1_text, "12 PM", GUI_FONT_SRC_BMP, gui_rgb(128, 128,
                         128), 5, 20);
            gui_text_type_set((gui_text_t *)sleep_chart_time1_text,
                              "/font/Inter_24pt_Regular_size20_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_chart_time1_text, LEFT);
            // Create sleep_chart_time2_text (hg_label)
            sleep_chart_time2_text = gui_text_create((gui_obj_t *)note, "sleep_chart_time2_text", 177, 260, 60,
                                                     34);
            gui_text_set((gui_text_t *)sleep_chart_time2_text, "2 AM", GUI_FONT_SRC_BMP, gui_rgb(128, 128, 128),
                         4, 20);
            gui_text_type_set((gui_text_t *)sleep_chart_time2_text,
                              "/font/Inter_24pt_Regular_size20_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_chart_time2_text, LEFT);
            // Create sleep_chart_time3_text (hg_label)
            sleep_chart_time3_text = gui_text_create((gui_obj_t *)note, "sleep_chart_time3_text", 248, 260, 60,
                                                     34);
            gui_text_set((gui_text_t *)sleep_chart_time3_text, "4 AM", GUI_FONT_SRC_BMP, gui_rgb(128, 128, 128),
                         4, 20);
            gui_text_type_set((gui_text_t *)sleep_chart_time3_text,
                              "/font/Inter_24pt_Regular_size20_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_chart_time3_text, LEFT);
            // Create sleep_chart_time4_text (hg_label)
            sleep_chart_time4_text = gui_text_create((gui_obj_t *)note, "sleep_chart_time4_text", 319, 260, 60,
                                                     34);
            gui_text_set((gui_text_t *)sleep_chart_time4_text, "6 AM", GUI_FONT_SRC_BMP, gui_rgb(128, 128, 128),
                         4, 20);
            gui_text_type_set((gui_text_t *)sleep_chart_time4_text,
                              "/font/Inter_24pt_Regular_size20_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_chart_time4_text, LEFT);
            // Create sleep_chart_bar5 (hg_rect)
            sleep_chart_bar5 = gui_rect_create((gui_obj_t *)note, "sleep_chart_bar5", 384, 137, 3, 140, 0,
                                               gui_rgb(128, 128, 128));
            // Create sleep_chart_rect1 (hg_rect)
            sleep_chart_rect1 = gui_rect_create((gui_obj_t *)note, "sleep_chart_rect1", 294, 172, 30, 45, 5,
                                                gui_rgb(94, 92, 223));
            // Create sleep_chart_rect2 (hg_rect)
            sleep_chart_rect2 = gui_rect_create((gui_obj_t *)note, "sleep_chart_rect2", 329, 172, 15, 45, 5,
                                                gui_rgb(94, 92, 223));
            // Create sleep_list1_sleepTime_text (hg_label)
            sleep_list1_sleepTime_text = gui_text_create((gui_obj_t *)note, "sleep_list1_sleepTime_text", 33,
                                                         304, 100, 40);
            gui_text_set((gui_text_t *)sleep_list1_sleepTime_text, "Sep 12", GUI_FONT_SRC_BMP, gui_rgb(94, 92,
                         223), 6, 30);
            gui_text_type_set((gui_text_t *)sleep_list1_sleepTime_text,
                              "/font/Inter_24pt_Regular_size30_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_list1_sleepTime_text, LEFT);
            // Create sleep_list1_sleepTime_text_uite (hg_label)
            sleep_list1_sleepTime_text_uite = gui_text_create((gui_obj_t *)note,
                                                              "sleep_list1_sleepTime_text_uite", 60, 358, 50, 40);
            gui_text_set((gui_text_t *)sleep_list1_sleepTime_text_uite, "HR", GUI_FONT_SRC_BMP, gui_rgb(255,
                         255, 255), 2, 30);
            gui_text_type_set((gui_text_t *)sleep_list1_sleepTime_text_uite,
                              "/font/Inter_24pt_Regular_size30_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_list1_sleepTime_text_uite, LEFT);
            // Create sleep_list1_sleepTime_text0 (hg_label)
            sleep_list1_sleepTime_text0 = gui_text_create((gui_obj_t *)note, "sleep_list1_sleepTime_text0", 33,
                                                          345, 50, 60);
            gui_text_set((gui_text_t *)sleep_list1_sleepTime_text0, "7", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 1, 42);
            gui_text_type_set((gui_text_t *)sleep_list1_sleepTime_text0,
                              "/font/Inter_24pt_Regular_size42_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_list1_sleepTime_text0, LEFT);
            // Create sleep_list1_sleepTime_text1 (hg_label)
            sleep_list1_sleepTime_text1 = gui_text_create((gui_obj_t *)note, "sleep_list1_sleepTime_text1", 108,
                                                          345, 50, 60);
            gui_text_set((gui_text_t *)sleep_list1_sleepTime_text1, "26", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 2, 42);
            gui_text_type_set((gui_text_t *)sleep_list1_sleepTime_text1,
                              "/font/Inter_24pt_Regular_size42_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_list1_sleepTime_text1, LEFT);
            // Create sleep_list1_sleepTime_text_uite1 (hg_label)
            sleep_list1_sleepTime_text_uite1 = gui_text_create((gui_obj_t *)note,
                                                               "sleep_list1_sleepTime_text_uite1", 162, 358, 66, 40);
            gui_text_set((gui_text_t *)sleep_list1_sleepTime_text_uite1, "MIN", GUI_FONT_SRC_BMP, gui_rgb(255,
                         255, 255), 3, 30);
            gui_text_type_set((gui_text_t *)sleep_list1_sleepTime_text_uite1,
                              "/font/Inter_24pt_Regular_size30_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_list1_sleepTime_text_uite1, LEFT);
            // Create sleep_list1_sleepTime_text_copy_1769738523896 (hg_label)
            sleep_list1_sleepTime_text_copy_1769738523896 = gui_text_create((gui_obj_t *)note,
                                                                            "sleep_list1_sleepTime_text_copy_1769738523896", 33, 396, 234, 36);
            gui_text_set((gui_text_t *)sleep_list1_sleepTime_text_copy_1769738523896, "10:55 PM - 6:26 AM",
                         GUI_FONT_SRC_BMP, gui_rgb(200, 200, 200), 18, 26);
            gui_text_type_set((gui_text_t *)sleep_list1_sleepTime_text_copy_1769738523896,
                              "/font/Inter_24pt_Regular_size26_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_list1_sleepTime_text_copy_1769738523896, LEFT);
            // Create sleep_chart_rect0 (hg_rect)
            sleep_chart_rect0 = gui_rect_create((gui_obj_t *)note, "sleep_chart_rect0", 45, 172, 240, 45, 5,
                                                gui_rgb(94, 92, 223));
            break;
        }
    case 2:
        {
            // Create sleep_char_bg0 (hg_image)
            sleep_char_bg0 = gui_img_create_from_fs((gui_obj_t *)note, "sleep_char_bg0",
                                                    "/app_sleep/sleep_chart_bg.bin", 16, 110, 180, 139);
            // Create sleep_char_bg1 (hg_image)
            sleep_char_bg1 = gui_img_create_from_fs((gui_obj_t *)note, "sleep_char_bg1",
                                                    "/app_sleep/sleep_chart_bg.bin", 215, 110, 180, 139);
            // Create sleep_char_data0 (hg_rect)
            sleep_char_data0 = gui_rect_create((gui_obj_t *)note, "sleep_char_data0", 16, 128, 10, 119, 2,
                                               gui_rgb(73, 93, 121));
            // Create sleep_char_data1 (hg_rect)
            sleep_char_data1 = gui_rect_create((gui_obj_t *)note, "sleep_char_data1", 43, 122, 10, 126, 2,
                                               gui_rgb(73, 93, 121));
            // Create sleep_char_data2 (hg_rect)
            sleep_char_data2 = gui_rect_create((gui_obj_t *)note, "sleep_char_data2", 69, 129, 10, 119, 2,
                                               gui_rgb(73, 93, 121));
            // Create sleep_char_data3 (hg_rect)
            sleep_char_data3 = gui_rect_create((gui_obj_t *)note, "sleep_char_data3", 99, 142, 10, 106, 2,
                                               gui_rgb(73, 93, 121));
            // Create sleep_char_data4 (hg_rect)
            sleep_char_data4 = gui_rect_create((gui_obj_t *)note, "sleep_char_data4", 128, 129, 10, 119, 2,
                                               gui_rgb(73, 93, 121));
            // Create sleep_char_data5 (hg_rect)
            sleep_char_data5 = gui_rect_create((gui_obj_t *)note, "sleep_char_data5", 158, 129, 10, 119, 2,
                                               gui_rgb(73, 93, 121));
            // Create sleep_char_data6 (hg_rect)
            sleep_char_data6 = gui_rect_create((gui_obj_t *)note, "sleep_char_data6", 186, 129, 10, 119, 2,
                                               gui_rgb(73, 93, 121));
            // Create sleep_char_topLine0 (hg_rect)
            sleep_char_topLine0 = gui_rect_create((gui_obj_t *)note, "sleep_char_topLine0", 16, 126, 180, 5,
                                                  2.5, gui_rgb(94, 92, 223));
            // Create sleep_char_data7 (hg_rect)
            sleep_char_data7 = gui_rect_create((gui_obj_t *)note, "sleep_char_data7", 215, 129, 10, 119, 2,
                                               gui_rgb(73, 93, 121));
            // Create sleep_char_data8 (hg_rect)
            sleep_char_data8 = gui_rect_create((gui_obj_t *)note, "sleep_char_data8", 245, 129, 10, 119, 2,
                                               gui_rgb(73, 93, 121));
            // Create sleep_char_data9 (hg_rect)
            sleep_char_data9 = gui_rect_create((gui_obj_t *)note, "sleep_char_data9", 274, 129, 10, 119, 2,
                                               gui_rgb(73, 93, 121));
            // Create sleep_char_data10 (hg_rect)
            sleep_char_data10 = gui_rect_create((gui_obj_t *)note, "sleep_char_data10", 300, 129, 10, 119, 2,
                                                gui_rgb(73, 93, 121));
            // Create sleep_char_data11 (hg_rect)
            sleep_char_data11 = gui_rect_create((gui_obj_t *)note, "sleep_char_data11", 327, 129, 10, 119, 2,
                                                gui_rgb(73, 93, 121));
            // Create sleep_char_data12 (hg_rect)
            sleep_char_data12 = gui_rect_create((gui_obj_t *)note, "sleep_char_data12", 356, 129, 10, 119, 2,
                                                gui_rgb(73, 93, 121));
            // Create sleep_char_data13 (hg_rect)
            sleep_char_data13 = gui_rect_create((gui_obj_t *)note, "sleep_char_data13", 385, 129, 10, 119, 2,
                                                gui_rgb(73, 93, 121));
            // Create sleep_char_topLine1 (hg_rect)
            sleep_char_topLine1 = gui_rect_create((gui_obj_t *)note, "sleep_char_topLine1", 215, 126, 180, 5,
                                                  2.5, gui_rgb(94, 92, 223));
            // Create sleep_list2_sleepTime_text0 (hg_label)
            sleep_list2_sleepTime_text0 = gui_text_create((gui_obj_t *)note, "sleep_list2_sleepTime_text0", 16,
                                                          79, 20, 40);
            gui_text_set((gui_text_t *)sleep_list2_sleepTime_text0, "7", GUI_FONT_SRC_BMP, gui_rgb(94, 92, 223),
                         1, 30);
            gui_text_type_set((gui_text_t *)sleep_list2_sleepTime_text0,
                              "/font/Inter_24pt_Regular_size30_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_list2_sleepTime_text0, LEFT);
            // Create sleep_list2_sleepTime_text_uite0 (hg_label)
            sleep_list2_sleepTime_text_uite0 = gui_text_create((gui_obj_t *)note,
                                                               "sleep_list2_sleepTime_text_uite0", 32, 88, 25, 30);
            gui_text_set((gui_text_t *)sleep_list2_sleepTime_text_uite0, "H", GUI_FONT_SRC_BMP, gui_rgb(94, 92,
                         223), 1, 20);
            gui_text_type_set((gui_text_t *)sleep_list2_sleepTime_text_uite0,
                              "/font/Inter_24pt_Regular_size20_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_list2_sleepTime_text_uite0, LEFT);
            // Create sleep_list2_sleepTime_text1 (hg_label)
            sleep_list2_sleepTime_text1 = gui_text_create((gui_obj_t *)note, "sleep_list2_sleepTime_text1", 58,
                                                          79, 31, 40);
            gui_text_set((gui_text_t *)sleep_list2_sleepTime_text1, "5", GUI_FONT_SRC_BMP, gui_rgb(94, 92, 223),
                         1, 30);
            gui_text_type_set((gui_text_t *)sleep_list2_sleepTime_text1,
                              "/font/Inter_24pt_Regular_size30_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_list2_sleepTime_text1, LEFT);
            // Create sleep_list2_sleepTime_text_uite1 (hg_label)
            sleep_list2_sleepTime_text_uite1 = gui_text_create((gui_obj_t *)note,
                                                               "sleep_list2_sleepTime_text_uite1", 76, 88, 30, 30);
            gui_text_set((gui_text_t *)sleep_list2_sleepTime_text_uite1, "M", GUI_FONT_SRC_BMP, gui_rgb(94, 92,
                         223), 1, 20);
            gui_text_type_set((gui_text_t *)sleep_list2_sleepTime_text_uite1,
                              "/font/Inter_24pt_Regular_size20_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_list2_sleepTime_text_uite1, LEFT);
            // Create sleep_list2_sleepTime_text2 (hg_label)
            sleep_list2_sleepTime_text2 = gui_text_create((gui_obj_t *)note, "sleep_list2_sleepTime_text2", 215,
                                                          79, 20, 40);
            gui_text_set((gui_text_t *)sleep_list2_sleepTime_text2, "7", GUI_FONT_SRC_BMP, gui_rgb(94, 92, 223),
                         1, 30);
            gui_text_type_set((gui_text_t *)sleep_list2_sleepTime_text2,
                              "/font/Inter_24pt_Regular_size30_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_list2_sleepTime_text2, LEFT);
            // Create sleep_list2_sleepTime_text_uite2 (hg_label)
            sleep_list2_sleepTime_text_uite2 = gui_text_create((gui_obj_t *)note,
                                                               "sleep_list2_sleepTime_text_uite2", 230, 88, 25, 30);
            gui_text_set((gui_text_t *)sleep_list2_sleepTime_text_uite2, "H", GUI_FONT_SRC_BMP, gui_rgb(94, 92,
                         223), 1, 20);
            gui_text_type_set((gui_text_t *)sleep_list2_sleepTime_text_uite2,
                              "/font/Inter_24pt_Regular_size20_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_list2_sleepTime_text_uite2, LEFT);
            // Create sleep_list2_sleepTime_text3 (hg_label)
            sleep_list2_sleepTime_text3 = gui_text_create((gui_obj_t *)note, "sleep_list2_sleepTime_text3", 254,
                                                          79, 30, 40);
            gui_text_set((gui_text_t *)sleep_list2_sleepTime_text3, "16", GUI_FONT_SRC_BMP, gui_rgb(94, 92,
                         223), 2, 30);
            gui_text_type_set((gui_text_t *)sleep_list2_sleepTime_text3,
                              "/font/Inter_24pt_Regular_size30_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_list2_sleepTime_text3, LEFT);
            // Create sleep_list2_sleepTime_text_uite3 (hg_label)
            sleep_list2_sleepTime_text_uite3 = gui_text_create((gui_obj_t *)note,
                                                               "sleep_list2_sleepTime_text_uite3", 284, 88, 30, 30);
            gui_text_set((gui_text_t *)sleep_list2_sleepTime_text_uite3, "M", GUI_FONT_SRC_BMP, gui_rgb(94, 92,
                         223), 1, 20);
            gui_text_type_set((gui_text_t *)sleep_list2_sleepTime_text_uite3,
                              "/font/Inter_24pt_Regular_size20_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_list2_sleepTime_text_uite3, LEFT);
            // Create hg_label_1769741973751_t3b1 (hg_label)
            hg_label_1769741973751_t3b1 = gui_text_create((gui_obj_t *)note, "hg_label_1769741973751_t3b1", 18,
                                                          262, 170, 38);
            gui_text_set((gui_text_t *)hg_label_1769741973751_t3b1, "AUG 30-SEP 5", GUI_FONT_SRC_BMP,
                         gui_rgb(128, 128, 128), 12, 28);
            gui_text_type_set((gui_text_t *)hg_label_1769741973751_t3b1,
                              "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_1769741973751_t3b1, LEFT);
            // Create hg_label_1769741973751_t3b1_copy_1769742407966 (hg_label)
            hg_label_1769741973751_t3b1_copy_1769742407966 = gui_text_create((gui_obj_t *)note,
                                                                             "hg_label_1769741973751_t3b1_copy_1769742407966", 215, 262, 170, 38);
            gui_text_set((gui_text_t *)hg_label_1769741973751_t3b1_copy_1769742407966, "SEP 6-12",
                         GUI_FONT_SRC_BMP, gui_rgb(128, 128, 128), 8, 28);
            gui_text_type_set((gui_text_t *)hg_label_1769741973751_t3b1_copy_1769742407966,
                              "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_1769741973751_t3b1_copy_1769742407966, LEFT);
            // Create sleep_duration_text (hg_label)
            sleep_duration_text = gui_text_create((gui_obj_t *)note, "sleep_duration_text", 18, 318, 238, 42);
            gui_text_set((gui_text_t *)sleep_duration_text, "Sleep Duration", GUI_FONT_SRC_BMP, gui_rgb(94, 92,
                         223), 14, 32);
            gui_text_type_set((gui_text_t *)sleep_duration_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_duration_text, LEFT);
            // Create sleep_consistent_text (hg_label)
            sleep_consistent_text = gui_text_create((gui_obj_t *)note, "sleep_consistent_text", 18, 358, 200,
                                                    50);
            gui_text_set((gui_text_t *)sleep_consistent_text, "Consistent", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 10, 40);
            gui_text_type_set((gui_text_t *)sleep_consistent_text,
                              "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_consistent_text, LEFT);
            // Create sleep_last_7_days_text (hg_label)
            sleep_last_7_days_text = gui_text_create((gui_obj_t *)note, "sleep_last_7_days_text", 18, 406, 170,
                                                     38);
            gui_text_set((gui_text_t *)sleep_last_7_days_text, "In last 7 days", GUI_FONT_SRC_BMP, gui_rgb(128,
                         128, 128), 14, 28);
            gui_text_type_set((gui_text_t *)sleep_last_7_days_text,
                              "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_last_7_days_text, LEFT);
            // Create sleep_nextSleep_text (hg_label)
            sleep_nextSleep_text = gui_text_create((gui_obj_t *)note, "sleep_nextSleep_text", 185, 2, 200, 40);
            gui_text_set((gui_text_t *)sleep_nextSleep_text, "Last 14 Days", GUI_FONT_SRC_BMP, gui_rgb(94, 92,
                         223), 12, 32);
            gui_text_type_set((gui_text_t *)sleep_nextSleep_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_nextSleep_text, RIGHT);
            break;
        }
    case 3:
        {
            // Create sleep_next_alarm_bedtime_text0 (hg_label)
            sleep_next_alarm_bedtime_text0 = gui_text_create((gui_obj_t *)note,
                                                             "sleep_next_alarm_bedtime_text0", 120, 94, 131, 38);
            gui_text_set((gui_text_t *)sleep_next_alarm_bedtime_text0, "Bedtime", GUI_FONT_SRC_BMP, gui_rgb(94,
                         92, 223), 7, 28);
            gui_text_type_set((gui_text_t *)sleep_next_alarm_bedtime_text0,
                              "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_next_alarm_bedtime_text0, LEFT);
            // Create sleep_next_alarm_bedtime_text (hg_label)
            sleep_next_alarm_bedtime_text = gui_text_create((gui_obj_t *)note, "sleep_next_alarm_bedtime_text",
                                                            29, 141, 200, 50);
            gui_text_set((gui_text_t *)sleep_next_alarm_bedtime_text, "10:30 PM", GUI_FONT_SRC_BMP, gui_rgb(255,
                         255, 255), 8, 40);
            gui_text_type_set((gui_text_t *)sleep_next_alarm_bedtime_text,
                              "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_next_alarm_bedtime_text, LEFT);
            // Create sleep_next_alarm_tomorrow_text (hg_label)
            sleep_next_alarm_tomorrow_text = gui_text_create((gui_obj_t *)note,
                                                             "sleep_next_alarm_tomorrow_text", 29, 192, 170, 38);
            gui_text_set((gui_text_t *)sleep_next_alarm_tomorrow_text, "Tomorrow", GUI_FONT_SRC_BMP,
                         gui_rgb(128, 128, 128), 8, 28);
            gui_text_type_set((gui_text_t *)sleep_next_alarm_tomorrow_text,
                              "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_next_alarm_tomorrow_text, LEFT);
            // Create sleep_alarm_working_icon (hg_image)
            sleep_alarm_working_icon = gui_img_create_from_fs((gui_obj_t *)note, "sleep_alarm_working_icon",
                                                              "/app_sleep/alarm_working_icon.bin", 31, 275, 60, 36);
            // Create sleep_next_alarm_wakeup_text0 (hg_label)
            sleep_next_alarm_wakeup_text0 = gui_text_create((gui_obj_t *)note, "sleep_next_alarm_wakeup_text0",
                                                            114, 281, 146, 38);
            gui_text_set((gui_text_t *)sleep_next_alarm_wakeup_text0, "Wake Up", GUI_FONT_SRC_BMP, gui_rgb(94,
                         92, 223), 7, 28);
            gui_text_type_set((gui_text_t *)sleep_next_alarm_wakeup_text0,
                              "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_next_alarm_wakeup_text0, LEFT);
            // Create sleep_next_alarm_wakeup_text (hg_label)
            sleep_next_alarm_wakeup_text = gui_text_create((gui_obj_t *)note, "sleep_next_alarm_wakeup_text",
                                                           31, 331, 200, 50);
            gui_text_set((gui_text_t *)sleep_next_alarm_wakeup_text, "7:00 PM", GUI_FONT_SRC_BMP, gui_rgb(255,
                         255, 255), 7, 40);
            gui_text_type_set((gui_text_t *)sleep_next_alarm_wakeup_text,
                              "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_next_alarm_wakeup_text, LEFT);
            // Create sleep_next_alarm_tomorrow_text1 (hg_label)
            sleep_next_alarm_tomorrow_text1 = gui_text_create((gui_obj_t *)note,
                                                              "sleep_next_alarm_tomorrow_text1", 31, 371, 170, 38);
            gui_text_set((gui_text_t *)sleep_next_alarm_tomorrow_text1, "Tomorrow", GUI_FONT_SRC_BMP,
                         gui_rgb(128, 128, 128), 8, 28);
            gui_text_type_set((gui_text_t *)sleep_next_alarm_tomorrow_text1,
                              "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_next_alarm_tomorrow_text1, LEFT);
            // Create sleep_next_alarm_icon (hg_image)
            sleep_next_alarm_icon = gui_img_create_from_fs((gui_obj_t *)note, "sleep_next_alarm_icon",
                                                           "/app_sleep/sleep_alarm_bed_icon.bin", 29, 84, 72, 46);
            // Create sleep_nextSchedule_text (hg_label)
            sleep_nextSchedule_text = gui_text_create((gui_obj_t *)note, "sleep_nextSchedule_text", 190, 0, 217,
                                                      40);
            gui_text_set((gui_text_t *)sleep_nextSchedule_text, "Next Schedule", GUI_FONT_SRC_BMP, gui_rgb(94,
                         92, 223), 13, 32);
            gui_text_type_set((gui_text_t *)sleep_nextSchedule_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)sleep_nextSchedule_text, RIGHT);
            break;
        }
    default:
        break;
    }
}


// Create app_sleep_view (hg_view)
static void app_sleep_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_sleep_view_switch_in(gui_view_t *view)
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



    // Create sleep_bg (hg_rect)
    sleep_bg = gui_rect_create((gui_obj_t *)view, "sleep_bg", 0, 0, 410, 502, 0, gui_rgb(25, 59, 117));
    // Set linear gradient
    gui_rect_set_linear_gradient(sleep_bg, RECT_GRADIENT_VERTICAL);
    gui_rect_add_gradient_stop(sleep_bg, 0.0f, gui_rgba(25, 59, 117, 255));
    gui_rect_add_gradient_stop(sleep_bg, 1.0f, gui_rgba(5, 20, 39, 255));

    // Create sleep_list (hg_list)
    sleep_list = gui_list_create((gui_obj_t *)view, "sleep_list", 0, 52, 410, 450, 450, 0, VERTICAL,
                                 sleep_list_note_design, NULL, false);
    gui_list_set_style(sleep_list, LIST_FADE);
    gui_list_set_note_num(sleep_list, 4);
    gui_list_set_auto_align(sleep_list, true);
    gui_list_set_inertia(sleep_list, false);
    gui_list_enable_area_display(sleep_list, true);

    // Create sleep_top_window (hg_window)
    sleep_top_window = gui_win_create((gui_obj_t *)view, "sleep_top_window", 0, 0, 410, 110);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(hg_time_label_1769654821849_4rcr_time_str,
                 sizeof(hg_time_label_1769654821849_4rcr_time_str), "%02d:%02d", t->tm_hour, t->tm_min);
    }


    // Create hg_time_label_1769654821849_4rcr (hg_time_label)
    hg_time_label_1769654821849_4rcr = gui_text_create(sleep_top_window,
                                                       "hg_time_label_1769654821849_4rcr", 306, 20, 80, 34);
    gui_text_set((gui_text_t *)hg_time_label_1769654821849_4rcr,
                 hg_time_label_1769654821849_4rcr_time_str, GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                 strlen(hg_time_label_1769654821849_4rcr_time_str), 32);
    gui_text_type_set((gui_text_t *)hg_time_label_1769654821849_4rcr,
                      "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_time_label_1769654821849_4rcr, CENTER);

    // Create hg_image_1769654877646_xx8b (hg_image)
    hg_image_1769654877646_xx8b = gui_img_create_from_fs(sleep_top_window,
                                                         "hg_image_1769654877646_xx8b", "/app_phone/icon_bg.bin", 24, 20, 72, 72);

    // Create hg_image_1769654928940_2nzg (hg_image)
    hg_image_1769654928940_2nzg = gui_img_create_from_fs(sleep_top_window,
                                                         "hg_image_1769654928940_2nzg", "/app_sleep/sleep_alarm_icon.bin", 42, 38, 36, 36);

    gui_obj_add_event_cb(GUI_BASE(sleep_top_window), (gui_event_cb_t)sleep_top_window_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)sleep_top_window);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(hg_time_label_1769654821849_4rcr), 30000, true,
                         hg_time_label_1769654821849_4rcr_time_update_cb);
}
GUI_VIEW_INSTANCE("app_sleep_view", false, app_sleep_view_switch_in, app_sleep_view_switch_out,
                  false);
