/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_wifi_test UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-06T09:11:40.994Z
 */
#include "app_wifi_test_ui.h"
#include "../callbacks/app_wifi_test_callbacks.h"
#include "../user/app_wifi_test_user.h"
#include <stddef.h>
#include <time.h>

// Component handle definitions
gui_win_t *wifi_home_top_window = NULL;
gui_win_t *wifi_home_back_touch_win = NULL;
gui_img_t *wifi_home_back_btn = NULL;
gui_text_t *wifi_home_time = NULL;
gui_scroll_text_t *wifi_home_title = NULL;
gui_list_t *wifi_home_list = NULL;
gui_rounded_rect_t *wifi_home_status_bg = NULL;
gui_img_t *wifi_home_wifi_icon = NULL;
gui_text_t *wifi_home_status_text = NULL;
gui_scroll_text_t *wifi_home_ip_text = NULL;
gui_rounded_rect_t *wifi_home_iperf_bg = NULL;
gui_img_t *wifi_home_iperf_icon = NULL;
gui_scroll_text_t *wifi_home_iperf_label = NULL;
gui_rounded_rect_t *wifi_home_file_bg = NULL;
gui_img_t *wifi_home_file_icon = NULL;
gui_scroll_text_t *wifi_home_file_label = NULL;
gui_rounded_rect_t *wifi_home_custom_bg = NULL;
gui_img_t *wifi_home_custom_icon = NULL;
gui_scroll_text_t *wifi_home_custom_label = NULL;
gui_win_t *iperf_menu_top_window = NULL;
gui_win_t *iperf_menu_back_touch_win = NULL;
gui_img_t *iperf_menu_back_btn = NULL;
gui_text_t *iperf_menu_time = NULL;
gui_scroll_text_t *iperf_menu_title = NULL;
gui_list_t *iperf_menu_list = NULL;
gui_rounded_rect_t *iperf_menu_upload_bg = NULL;
gui_img_t *iperf_menu_upload_icon = NULL;
gui_scroll_text_t *iperf_menu_upload_label = NULL;
gui_rounded_rect_t *iperf_menu_download_bg = NULL;
gui_img_t *iperf_menu_download_icon = NULL;
gui_scroll_text_t *iperf_menu_download_label = NULL;
gui_win_t *iperf_upload_top_window = NULL;
gui_win_t *iperf_upload_back_touch_win = NULL;
gui_img_t *iperf_upload_back_btn = NULL;
gui_text_t *iperf_upload_time = NULL;
gui_scroll_text_t *iperf_upload_title = NULL;
gui_rounded_rect_t *iperf_ip_display_bg = NULL;
gui_rounded_rect_t *iperf_ip_seg0_bg = NULL;
gui_rounded_rect_t *iperf_ip_seg1_bg = NULL;
gui_rounded_rect_t *iperf_ip_seg2_bg = NULL;
gui_rounded_rect_t *iperf_ip_seg3_bg = NULL;
gui_text_t *iperf_ip_seg0 = NULL;
gui_text_t *iperf_ip_dot0 = NULL;
gui_text_t *iperf_ip_seg1 = NULL;
gui_text_t *iperf_ip_dot1 = NULL;
gui_text_t *iperf_ip_seg2 = NULL;
gui_text_t *iperf_ip_dot2 = NULL;
gui_text_t *iperf_ip_seg3 = NULL;
gui_text_t *iperf_ip_confirmed = NULL;
gui_rounded_rect_t *iperf_key_1_bg = NULL;
gui_text_t *iperf_key_1 = NULL;
gui_rounded_rect_t *iperf_key_2_bg = NULL;
gui_text_t *iperf_key_2 = NULL;
gui_rounded_rect_t *iperf_key_3_bg = NULL;
gui_text_t *iperf_key_3 = NULL;
gui_rounded_rect_t *iperf_key_4_bg = NULL;
gui_text_t *iperf_key_4 = NULL;
gui_rounded_rect_t *iperf_key_5_bg = NULL;
gui_text_t *iperf_key_5 = NULL;
gui_rounded_rect_t *iperf_key_6_bg = NULL;
gui_text_t *iperf_key_6 = NULL;
gui_rounded_rect_t *iperf_key_7_bg = NULL;
gui_text_t *iperf_key_7 = NULL;
gui_rounded_rect_t *iperf_key_8_bg = NULL;
gui_text_t *iperf_key_8 = NULL;
gui_rounded_rect_t *iperf_key_9_bg = NULL;
gui_text_t *iperf_key_9 = NULL;
gui_rounded_rect_t *iperf_key_dot_bg = NULL;
gui_text_t *iperf_key_dot = NULL;
gui_rounded_rect_t *iperf_key_0_bg = NULL;
gui_text_t *iperf_key_0 = NULL;
gui_rounded_rect_t *iperf_key_del_bg = NULL;
gui_text_t *iperf_key_del = NULL;
gui_rounded_rect_t *iperf_btn_confirm_bg = NULL;
gui_text_t *iperf_btn_confirm = NULL;
gui_rounded_rect_t *iperf_btn_start_bg = NULL;
gui_text_t *iperf_btn_start = NULL;
gui_win_t *iperf_upload_run_top_window = NULL;
gui_win_t *iperf_upload_run_back_touch_win = NULL;
gui_img_t *iperf_upload_run_back_btn = NULL;
gui_text_t *iperf_upload_run_time = NULL;
gui_scroll_text_t *iperf_upload_run_title = NULL;
gui_arc_t *iperf_upload_ring_bg = NULL;
gui_arc_t *iperf_upload_ring = NULL;
gui_text_t *iperf_upload_speed = NULL;
gui_text_t *iperf_upload_unit = NULL;
gui_text_t *iperf_upload_target = NULL;
gui_win_t *iperf_download_top_window = NULL;
gui_win_t *iperf_download_back_touch_win = NULL;
gui_img_t *iperf_download_back_btn = NULL;
gui_text_t *iperf_download_time = NULL;
gui_scroll_text_t *iperf_download_title = NULL;
gui_rounded_rect_t *iperf_download_soc_ip_bg = NULL;
gui_text_t *iperf_download_soc_ip = NULL;
gui_win_t *iperf_dl_running_win = NULL;
gui_arc_t *iperf_dl_ring_bg = NULL;
gui_arc_t *iperf_dl_ring = NULL;
gui_text_t *iperf_dl_speed = NULL;
gui_text_t *iperf_dl_unit = NULL;
gui_win_t *file_menu_top_window = NULL;
gui_win_t *file_menu_back_touch_win = NULL;
gui_img_t *file_menu_back_btn = NULL;
gui_text_t *file_menu_time = NULL;
gui_scroll_text_t *file_menu_title = NULL;
gui_list_t *file_menu_list = NULL;
gui_rounded_rect_t *file_menu_upload_bg = NULL;
gui_img_t *file_menu_upload_icon = NULL;
gui_scroll_text_t *file_menu_upload_label = NULL;
gui_rounded_rect_t *file_menu_download_bg = NULL;
gui_img_t *file_menu_download_icon = NULL;
gui_scroll_text_t *file_menu_download_label = NULL;
gui_win_t *file_upload_top_window = NULL;
gui_win_t *file_upload_back_touch_win = NULL;
gui_img_t *file_upload_back_btn = NULL;
gui_text_t *file_upload_time = NULL;
gui_scroll_text_t *file_upload_title = NULL;
gui_rounded_rect_t *file_ip_display_bg = NULL;
gui_rounded_rect_t *file_ip_seg0_bg = NULL;
gui_rounded_rect_t *file_ip_seg1_bg = NULL;
gui_rounded_rect_t *file_ip_seg2_bg = NULL;
gui_rounded_rect_t *file_ip_seg3_bg = NULL;
gui_text_t *file_ip_seg0 = NULL;
gui_text_t *file_ip_dot0 = NULL;
gui_text_t *file_ip_seg1 = NULL;
gui_text_t *file_ip_dot1 = NULL;
gui_text_t *file_ip_seg2 = NULL;
gui_text_t *file_ip_dot2 = NULL;
gui_text_t *file_ip_seg3 = NULL;
gui_text_t *file_ip_confirmed = NULL;
gui_rounded_rect_t *file_key_1_bg = NULL;
gui_text_t *file_key_1 = NULL;
gui_rounded_rect_t *file_key_2_bg = NULL;
gui_text_t *file_key_2 = NULL;
gui_rounded_rect_t *file_key_3_bg = NULL;
gui_text_t *file_key_3 = NULL;
gui_rounded_rect_t *file_key_4_bg = NULL;
gui_text_t *file_key_4 = NULL;
gui_rounded_rect_t *file_key_5_bg = NULL;
gui_text_t *file_key_5 = NULL;
gui_rounded_rect_t *file_key_6_bg = NULL;
gui_text_t *file_key_6 = NULL;
gui_rounded_rect_t *file_key_7_bg = NULL;
gui_text_t *file_key_7 = NULL;
gui_rounded_rect_t *file_key_8_bg = NULL;
gui_text_t *file_key_8 = NULL;
gui_rounded_rect_t *file_key_9_bg = NULL;
gui_text_t *file_key_9 = NULL;
gui_rounded_rect_t *file_key_dot_bg = NULL;
gui_text_t *file_key_dot = NULL;
gui_rounded_rect_t *file_key_0_bg = NULL;
gui_text_t *file_key_0 = NULL;
gui_rounded_rect_t *file_key_del_bg = NULL;
gui_text_t *file_key_del = NULL;
gui_rounded_rect_t *file_btn_confirm_bg = NULL;
gui_text_t *file_btn_confirm = NULL;
gui_rounded_rect_t *file_btn_start_bg = NULL;
gui_text_t *file_btn_start = NULL;
gui_win_t *file_upload_run_top_window = NULL;
gui_win_t *file_upload_run_back_touch_win = NULL;
gui_img_t *file_upload_run_back_btn = NULL;
gui_text_t *file_upload_run_time = NULL;
gui_scroll_text_t *file_upload_run_title = NULL;
gui_arc_t *file_upload_ring_bg = NULL;
gui_arc_t *file_upload_ring = NULL;
gui_text_t *file_upload_data = NULL;
gui_text_t *file_upload_unit = NULL;
gui_text_t *file_upload_target = NULL;
gui_win_t *file_download_top_window = NULL;
gui_win_t *file_download_back_touch_win = NULL;
gui_img_t *file_download_back_btn = NULL;
gui_text_t *file_download_time = NULL;
gui_scroll_text_t *file_download_title = NULL;
gui_rounded_rect_t *file_download_soc_ip_bg = NULL;
gui_text_t *file_download_soc_ip = NULL;
gui_win_t *file_dl_running_win = NULL;
gui_arc_t *file_dl_ring_bg = NULL;
gui_arc_t *file_dl_ring = NULL;
gui_text_t *file_dl_data = NULL;
gui_text_t *file_dl_unit = NULL;
gui_text_t *file_dl_connect_timer_host = NULL;
gui_win_t *custom_menu_top_window = NULL;
gui_win_t *custom_menu_back_touch_win = NULL;
gui_img_t *custom_menu_back_btn = NULL;
gui_text_t *custom_menu_time = NULL;
gui_scroll_text_t *custom_menu_title = NULL;
gui_list_t *custom_menu_list = NULL;
gui_rounded_rect_t *custom_menu_test1_bg = NULL;
gui_img_t *custom_menu_test1_icon = NULL;
gui_scroll_text_t *custom_menu_test1_label = NULL;
gui_rounded_rect_t *custom_menu_test2_bg = NULL;
gui_img_t *custom_menu_test2_icon = NULL;
gui_scroll_text_t *custom_menu_test2_label = NULL;
gui_rounded_rect_t *custom_menu_test3_bg = NULL;
gui_img_t *custom_menu_test3_icon = NULL;
gui_scroll_text_t *custom_menu_test3_label = NULL;
gui_win_t *custom1_top_window = NULL;
gui_win_t *custom1_back_touch_win = NULL;
gui_img_t *custom1_back_btn = NULL;
gui_text_t *custom1_time = NULL;
gui_scroll_text_t *custom1_title = NULL;
gui_win_t *custom2_top_window = NULL;
gui_win_t *custom2_back_touch_win = NULL;
gui_img_t *custom2_back_btn = NULL;
gui_text_t *custom2_time = NULL;
gui_scroll_text_t *custom2_title = NULL;
gui_win_t *custom3_top_window = NULL;
gui_win_t *custom3_back_touch_win = NULL;
gui_img_t *custom3_back_btn = NULL;
gui_text_t *custom3_time = NULL;
gui_scroll_text_t *custom3_title = NULL;

// Time string global variables
char wifi_home_time_time_str[10] = {0};
char iperf_menu_time_time_str[10] = {0};
char iperf_upload_time_time_str[10] = {0};
char iperf_upload_run_time_time_str[10] = {0};
char iperf_download_time_time_str[10] = {0};
char file_menu_time_time_str[10] = {0};
char file_upload_time_time_str[10] = {0};
char file_upload_run_time_time_str[10] = {0};
char file_download_time_time_str[10] = {0};
char custom_menu_time_time_str[10] = {0};
char custom1_time_time_str[10] = {0};
char custom2_time_time_str[10] = {0};
char custom3_time_time_str[10] = {0};

// List component note_design callback functions
// note_design callback function declaration
static void wifi_home_list_note_design(gui_obj_t *obj, void *param);

// note_design callback function implementation
static void wifi_home_list_note_design(gui_obj_t *obj, void *param)
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
            // Create wifi_home_status_bg (hg_rect)
            wifi_home_status_bg = gui_rect_create((gui_obj_t *)note, "wifi_home_status_bg", 24, 0, 362, 84, 16,
                                                  gui_rgba(255, 255, 255, 20));
            // Create wifi_home_wifi_icon (hg_image)
            wifi_home_wifi_icon = gui_img_create_from_fs((gui_obj_t *)note, "wifi_home_wifi_icon",
                                                         "/app_wifi_test/icon_wifi_connected.bin", 40, 6, 72, 72);
            // Create wifi_home_status_text (hg_label)
            wifi_home_status_text = gui_text_create((gui_obj_t *)note, "wifi_home_status_text", 128, 12, 220,
                                                    24);
            gui_text_set((gui_text_t *)wifi_home_status_text, "Connected", GUI_FONT_SRC_BMP, gui_rgb(153, 153,
                         153), 9, 16);
            gui_text_type_set((gui_text_t *)wifi_home_status_text,
                              "/font/Inter_24pt_Regular_size16_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)wifi_home_status_text, MID_LEFT);
            // Create wifi_home_ip_text (hg_label)
            wifi_home_ip_text = gui_scroll_text_create((gui_obj_t *)note, "wifi_home_ip_text", 128, 38, 220,
                                                       50);
            gui_scroll_text_set((gui_scroll_text_t *)wifi_home_ip_text, "IP: 192.168.1.100", GUI_FONT_SRC_BMP,
                                gui_rgb(242, 242, 242), 17, 40);
            gui_scroll_text_type_set((gui_scroll_text_t *)wifi_home_ip_text,
                                     "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_scroll_text_scroll_set((gui_scroll_text_t *)wifi_home_ip_text, SCROLL_X_MID, 0, 0, 3000, 0);
            break;
        }
    case 1:
        {
            // Create wifi_home_iperf_bg (hg_rect)
            wifi_home_iperf_bg = gui_rect_create((gui_obj_t *)note, "wifi_home_iperf_bg", 24, 0, 362, 84, 16,
                                                 gui_rgba(255, 255, 255, 20));
            // Create wifi_home_iperf_icon (hg_image)
            wifi_home_iperf_icon = gui_img_create_from_fs((gui_obj_t *)note, "wifi_home_iperf_icon",
                                                          "/app_wifi_test/icon_iperf.bin", 40, 6, 72, 72);
            // Create wifi_home_iperf_label (hg_label)
            wifi_home_iperf_label = gui_scroll_text_create((gui_obj_t *)note, "wifi_home_iperf_label", 128, 22,
                                                           220, 50);
            gui_scroll_text_set((gui_scroll_text_t *)wifi_home_iperf_label, "iPerf Test", GUI_FONT_SRC_BMP,
                                gui_rgb(242, 242, 242), 10, 40);
            gui_scroll_text_type_set((gui_scroll_text_t *)wifi_home_iperf_label,
                                     "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_scroll_text_scroll_set((gui_scroll_text_t *)wifi_home_iperf_label, SCROLL_X_MID, 0, 0, 3000, 0);
            gui_obj_add_event_cb(obj, wifi_home_iperf_btn_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 2:
        {
            // Create wifi_home_file_bg (hg_rect)
            wifi_home_file_bg = gui_rect_create((gui_obj_t *)note, "wifi_home_file_bg", 24, 0, 362, 84, 16,
                                                gui_rgba(255, 255, 255, 20));
            // Create wifi_home_file_icon (hg_image)
            wifi_home_file_icon = gui_img_create_from_fs((gui_obj_t *)note, "wifi_home_file_icon",
                                                         "/app_wifi_test/icon_file_transfer.bin", 40, 6, 72, 72);
            // Create wifi_home_file_label (hg_label)
            wifi_home_file_label = gui_scroll_text_create((gui_obj_t *)note, "wifi_home_file_label", 128, 22,
                                                          220, 50);
            gui_scroll_text_set((gui_scroll_text_t *)wifi_home_file_label, "File Transfer", GUI_FONT_SRC_BMP,
                                gui_rgb(242, 242, 242), 13, 40);
            gui_scroll_text_type_set((gui_scroll_text_t *)wifi_home_file_label,
                                     "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_scroll_text_scroll_set((gui_scroll_text_t *)wifi_home_file_label, SCROLL_X_MID, 0, 0, 3000, 0);
            gui_obj_add_event_cb(obj, wifi_home_file_btn_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 3:
        {
            // Create wifi_home_custom_bg (hg_rect)
            wifi_home_custom_bg = gui_rect_create((gui_obj_t *)note, "wifi_home_custom_bg", 24, 0, 362, 84, 16,
                                                  gui_rgba(255, 255, 255, 20));
            // Create wifi_home_custom_icon (hg_image)
            wifi_home_custom_icon = gui_img_create_from_fs((gui_obj_t *)note, "wifi_home_custom_icon",
                                                           "/app_wifi_test/icon_custom_tests.bin", 40, 6, 72, 72);
            // Create wifi_home_custom_label (hg_label)
            wifi_home_custom_label = gui_scroll_text_create((gui_obj_t *)note, "wifi_home_custom_label", 128,
                                                            22, 220, 50);
            gui_scroll_text_set((gui_scroll_text_t *)wifi_home_custom_label, "Custom Tests", GUI_FONT_SRC_BMP,
                                gui_rgb(242, 242, 242), 12, 40);
            gui_scroll_text_type_set((gui_scroll_text_t *)wifi_home_custom_label,
                                     "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_scroll_text_scroll_set((gui_scroll_text_t *)wifi_home_custom_label, SCROLL_X_MID, 0, 0, 3000,
                                       0);
            gui_obj_add_event_cb(obj, wifi_home_custom_btn_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    default:
        break;
    }
}

// note_design callback function declaration
static void iperf_menu_list_note_design(gui_obj_t *obj, void *param);

// note_design callback function implementation
static void iperf_menu_list_note_design(gui_obj_t *obj, void *param)
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
            // Create iperf_menu_upload_bg (hg_rect)
            iperf_menu_upload_bg = gui_rect_create((gui_obj_t *)note, "iperf_menu_upload_bg", 24, 0, 362, 84,
                                                   16, gui_rgba(255, 255, 255, 20));
            // Create iperf_menu_upload_icon (hg_image)
            iperf_menu_upload_icon = gui_img_create_from_fs((gui_obj_t *)note, "iperf_menu_upload_icon",
                                                            "/app_wifi_test/icon_upload.bin", 40, 6, 72, 72);
            // Create iperf_menu_upload_label (hg_label)
            iperf_menu_upload_label = gui_scroll_text_create((gui_obj_t *)note, "iperf_menu_upload_label", 128,
                                                             22, 220, 50);
            gui_scroll_text_set((gui_scroll_text_t *)iperf_menu_upload_label, "Upload Test", GUI_FONT_SRC_BMP,
                                gui_rgb(242, 242, 242), 11, 40);
            gui_scroll_text_type_set((gui_scroll_text_t *)iperf_menu_upload_label,
                                     "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_scroll_text_scroll_set((gui_scroll_text_t *)iperf_menu_upload_label, SCROLL_X_MID, 0, 0, 3000,
                                       0);
            gui_obj_add_event_cb(obj, iperf_menu_upload_btn_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 1:
        {
            // Create iperf_menu_download_bg (hg_rect)
            iperf_menu_download_bg = gui_rect_create((gui_obj_t *)note, "iperf_menu_download_bg", 24, 0, 362,
                                                     84, 16, gui_rgba(255, 255, 255, 20));
            gui_obj_add_event_cb(iperf_menu_download_bg, (gui_event_cb_t)iperf_menu_download_bg_clicked_cb,
                                 GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create iperf_menu_download_icon (hg_image)
            iperf_menu_download_icon = gui_img_create_from_fs((gui_obj_t *)note, "iperf_menu_download_icon",
                                                              "/app_wifi_test/icon_download_green.bin", 40, 6, 72, 72);
            // Create iperf_menu_download_label (hg_label)
            iperf_menu_download_label = gui_scroll_text_create((gui_obj_t *)note, "iperf_menu_download_label",
                                                               128, 22, 220, 50);
            gui_scroll_text_set((gui_scroll_text_t *)iperf_menu_download_label, "Download Test",
                                GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 13, 40);
            gui_scroll_text_type_set((gui_scroll_text_t *)iperf_menu_download_label,
                                     "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_scroll_text_scroll_set((gui_scroll_text_t *)iperf_menu_download_label, SCROLL_X_MID, 0, 0, 3000,
                                       0);
            break;
        }
    default:
        break;
    }
}

// note_design callback function declaration
static void file_menu_list_note_design(gui_obj_t *obj, void *param);

// note_design callback function implementation
static void file_menu_list_note_design(gui_obj_t *obj, void *param)
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
            // Create file_menu_upload_bg (hg_rect)
            file_menu_upload_bg = gui_rect_create((gui_obj_t *)note, "file_menu_upload_bg", 24, 0, 362, 84, 16,
                                                  gui_rgba(255, 255, 255, 20));
            // Create file_menu_upload_icon (hg_image)
            file_menu_upload_icon = gui_img_create_from_fs((gui_obj_t *)note, "file_menu_upload_icon",
                                                           "/app_wifi_test/icon_file_upload.bin", 40, 6, 72, 72);
            // Create file_menu_upload_label (hg_label)
            file_menu_upload_label = gui_scroll_text_create((gui_obj_t *)note, "file_menu_upload_label", 128,
                                                            22, 220, 50);
            gui_scroll_text_set((gui_scroll_text_t *)file_menu_upload_label, "Upload Test", GUI_FONT_SRC_BMP,
                                gui_rgb(242, 242, 242), 11, 40);
            gui_scroll_text_type_set((gui_scroll_text_t *)file_menu_upload_label,
                                     "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_scroll_text_scroll_set((gui_scroll_text_t *)file_menu_upload_label, SCROLL_X_MID, 0, 0, 3000,
                                       0);
            gui_obj_add_event_cb(obj, file_menu_upload_btn_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 1:
        {
            // Create file_menu_download_bg (hg_rect)
            file_menu_download_bg = gui_rect_create((gui_obj_t *)note, "file_menu_download_bg", 24, 0, 362, 84,
                                                    16, gui_rgba(255, 255, 255, 20));
            gui_obj_add_event_cb(file_menu_download_bg, (gui_event_cb_t)file_menu_download_bg_clicked_cb,
                                 GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create file_menu_download_icon (hg_image)
            file_menu_download_icon = gui_img_create_from_fs((gui_obj_t *)note, "file_menu_download_icon",
                                                             "/app_wifi_test/icon_file_download.bin", 40, 6, 72, 72);
            // Create file_menu_download_label (hg_label)
            file_menu_download_label = gui_scroll_text_create((gui_obj_t *)note, "file_menu_download_label",
                                                              128, 22, 220, 50);
            gui_scroll_text_set((gui_scroll_text_t *)file_menu_download_label, "Download Test",
                                GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 13, 40);
            gui_scroll_text_type_set((gui_scroll_text_t *)file_menu_download_label,
                                     "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_scroll_text_scroll_set((gui_scroll_text_t *)file_menu_download_label, SCROLL_X_MID, 0, 0, 3000,
                                       0);
            break;
        }
    default:
        break;
    }
}

// note_design callback function declaration
static void custom_menu_list_note_design(gui_obj_t *obj, void *param);

// note_design callback function implementation
static void custom_menu_list_note_design(gui_obj_t *obj, void *param)
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
            // Create custom_menu_test1_bg (hg_rect)
            custom_menu_test1_bg = gui_rect_create((gui_obj_t *)note, "custom_menu_test1_bg", 24, 0, 362, 84,
                                                   16, gui_rgba(255, 255, 255, 20));
            // Create custom_menu_test1_icon (hg_image)
            custom_menu_test1_icon = gui_img_create_from_fs((gui_obj_t *)note, "custom_menu_test1_icon",
                                                            "/app_wifi_test/icon_custom_1.bin", 40, 6, 72, 72);
            // Create custom_menu_test1_label (hg_label)
            custom_menu_test1_label = gui_scroll_text_create((gui_obj_t *)note, "custom_menu_test1_label", 128,
                                                             22, 220, 50);
            gui_scroll_text_set((gui_scroll_text_t *)custom_menu_test1_label, "Custom Test 1", GUI_FONT_SRC_BMP,
                                gui_rgb(242, 242, 242), 13, 40);
            gui_scroll_text_type_set((gui_scroll_text_t *)custom_menu_test1_label,
                                     "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_scroll_text_scroll_set((gui_scroll_text_t *)custom_menu_test1_label, SCROLL_X_MID, 0, 0, 3000,
                                       0);
            gui_obj_add_event_cb(obj, custom_menu_test1_btn_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 1:
        {
            // Create custom_menu_test2_bg (hg_rect)
            custom_menu_test2_bg = gui_rect_create((gui_obj_t *)note, "custom_menu_test2_bg", 24, 0, 362, 84,
                                                   16, gui_rgba(255, 255, 255, 20));
            // Create custom_menu_test2_icon (hg_image)
            custom_menu_test2_icon = gui_img_create_from_fs((gui_obj_t *)note, "custom_menu_test2_icon",
                                                            "/app_wifi_test/icon_custom_2.bin", 40, 6, 72, 72);
            // Create custom_menu_test2_label (hg_label)
            custom_menu_test2_label = gui_scroll_text_create((gui_obj_t *)note, "custom_menu_test2_label", 128,
                                                             22, 220, 50);
            gui_scroll_text_set((gui_scroll_text_t *)custom_menu_test2_label, "Custom Test 2", GUI_FONT_SRC_BMP,
                                gui_rgb(242, 242, 242), 13, 40);
            gui_scroll_text_type_set((gui_scroll_text_t *)custom_menu_test2_label,
                                     "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_scroll_text_scroll_set((gui_scroll_text_t *)custom_menu_test2_label, SCROLL_X_MID, 0, 0, 3000,
                                       0);
            gui_obj_add_event_cb(obj, custom_menu_test2_btn_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 2:
        {
            // Create custom_menu_test3_bg (hg_rect)
            custom_menu_test3_bg = gui_rect_create((gui_obj_t *)note, "custom_menu_test3_bg", 24, 0, 362, 84,
                                                   16, gui_rgba(255, 255, 255, 20));
            // Create custom_menu_test3_icon (hg_image)
            custom_menu_test3_icon = gui_img_create_from_fs((gui_obj_t *)note, "custom_menu_test3_icon",
                                                            "/app_wifi_test/icon_custom_3.bin", 40, 6, 72, 72);
            // Create custom_menu_test3_label (hg_label)
            custom_menu_test3_label = gui_scroll_text_create((gui_obj_t *)note, "custom_menu_test3_label", 128,
                                                             22, 220, 50);
            gui_scroll_text_set((gui_scroll_text_t *)custom_menu_test3_label, "Custom Test 3", GUI_FONT_SRC_BMP,
                                gui_rgb(242, 242, 242), 13, 40);
            gui_scroll_text_type_set((gui_scroll_text_t *)custom_menu_test3_label,
                                     "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_scroll_text_scroll_set((gui_scroll_text_t *)custom_menu_test3_label, SCROLL_X_MID, 0, 0, 3000,
                                       0);
            gui_obj_add_event_cb(obj, custom_menu_test3_btn_switch_view_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    default:
        break;
    }
}


// Create app_wifi_testWifiHomeView (hg_view)
static void app_wifi_testWifiHomeView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_wifi_testWifiHomeView_switch_in(gui_view_t *view)
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



    // Create wifi_home_title (hg_label)
    wifi_home_title = gui_scroll_text_create((gui_obj_t *)view, "wifi_home_title", 24, 73, 362, 50);
    gui_scroll_text_set((gui_scroll_text_t *)wifi_home_title, "WiFi Test", GUI_FONT_SRC_BMP,
                        gui_rgb(242, 242, 242), 9, 40);
    gui_scroll_text_type_set((gui_scroll_text_t *)wifi_home_title,
                             "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_scroll_text_scroll_set((gui_scroll_text_t *)wifi_home_title, SCROLL_X_MID, 0, 0, 3000, 0);

    // Create wifi_home_list (hg_list)
    wifi_home_list = gui_list_create((gui_obj_t *)view, "wifi_home_list", 0, 123, 410, 379, 84, 10,
                                     VERTICAL, wifi_home_list_note_design, NULL, false);
    gui_list_set_style(wifi_home_list, LIST_CLASSIC);
    gui_list_set_note_num(wifi_home_list, 4);
    gui_list_set_out_scope(wifi_home_list, 80);
    gui_list_keep_note_alive(wifi_home_list, true);

    // Create wifi_home_top_window (hg_window)
    wifi_home_top_window = gui_win_create((gui_obj_t *)view, "wifi_home_top_window", 0, 0, 410, 80);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(wifi_home_time_time_str, sizeof(wifi_home_time_time_str), "%02d:%02d", t->tm_hour,
                 t->tm_min);
    }


    // Create wifi_home_back_touch_win (hg_window)
    wifi_home_back_touch_win = gui_win_create(wifi_home_top_window, "wifi_home_back_touch_win", 0, 0,
                                              100, 100);


    // Create wifi_home_back_btn (hg_image)
    wifi_home_back_btn = gui_img_create_from_fs(wifi_home_back_touch_win, "wifi_home_back_btn",
                                                "/app_wifi_test/back_icon.bin", 34, 30, 28, 28);

    gui_obj_add_event_cb(GUI_BASE(wifi_home_back_touch_win),
                         (gui_event_cb_t)wifi_home_back_touch_win_clicked_0_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create wifi_home_time (hg_time_label)
    wifi_home_time = gui_text_create(wifi_home_top_window, "wifi_home_time", 300, 20, 80, 32);
    gui_text_set((gui_text_t *)wifi_home_time, wifi_home_time_time_str, GUI_FONT_SRC_BMP, gui_rgb(255,
                 255, 255), strlen(wifi_home_time_time_str), 32);
    gui_text_type_set((gui_text_t *)wifi_home_time, "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)wifi_home_time, RIGHT);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(wifi_home_time), 30000, true, wifi_home_time_time_update_cb);

    gui_obj_add_event_cb((gui_obj_t *)view, (gui_event_cb_t)app_wifi_testWifiHomeView_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)view);
}
GUI_VIEW_INSTANCE("app_wifi_testWifiHomeView", false, app_wifi_testWifiHomeView_switch_in,
                  app_wifi_testWifiHomeView_switch_out, false);

// Create app_wifi_testIperfMenuView (hg_view)
static void app_wifi_testIperfMenuView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_wifi_testIperfMenuView_switch_in(gui_view_t *view)
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



    // Create iperf_menu_title (hg_label)
    iperf_menu_title = gui_scroll_text_create((gui_obj_t *)view, "iperf_menu_title", 24, 137, 362, 50);
    gui_scroll_text_set((gui_scroll_text_t *)iperf_menu_title, "iPerf Test", GUI_FONT_SRC_BMP,
                        gui_rgb(242, 242, 242), 10, 40);
    gui_scroll_text_type_set((gui_scroll_text_t *)iperf_menu_title,
                             "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_scroll_text_scroll_set((gui_scroll_text_t *)iperf_menu_title, SCROLL_X_MID, 0, 0, 3000, 0);

    // Create iperf_menu_list (hg_list)
    iperf_menu_list = gui_list_create((gui_obj_t *)view, "iperf_menu_list", 0, 187, 410, 220, 84, 10,
                                      VERTICAL, iperf_menu_list_note_design, NULL, false);
    gui_list_set_style(iperf_menu_list, LIST_CLASSIC);
    gui_list_set_note_num(iperf_menu_list, 2);
    gui_list_set_out_scope(iperf_menu_list, 80);
    gui_list_keep_note_alive(iperf_menu_list, true);

    // Create iperf_menu_top_window (hg_window)
    iperf_menu_top_window = gui_win_create((gui_obj_t *)view, "iperf_menu_top_window", 0, 0, 410, 100);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(iperf_menu_time_time_str, sizeof(iperf_menu_time_time_str), "%02d:%02d", t->tm_hour,
                 t->tm_min);
    }


    // Create iperf_menu_back_touch_win (hg_window)
    iperf_menu_back_touch_win = gui_win_create(iperf_menu_top_window, "iperf_menu_back_touch_win", 0, 0,
                                               100, 100);


    // Create iperf_menu_back_btn (hg_image)
    iperf_menu_back_btn = gui_img_create_from_fs(iperf_menu_back_touch_win, "iperf_menu_back_btn",
                                                 "/app_wifi_test/back_icon.bin", 34, 30, 28, 28);

    gui_obj_add_event_cb(GUI_BASE(iperf_menu_back_touch_win),
                         (gui_event_cb_t)iperf_menu_back_touch_win_clicked_0_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create iperf_menu_time (hg_time_label)
    iperf_menu_time = gui_text_create(iperf_menu_top_window, "iperf_menu_time", 300, 20, 80, 32);
    gui_text_set((gui_text_t *)iperf_menu_time, iperf_menu_time_time_str, GUI_FONT_SRC_BMP, gui_rgb(255,
                 255, 255), strlen(iperf_menu_time_time_str), 32);
    gui_text_type_set((gui_text_t *)iperf_menu_time, "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)iperf_menu_time, RIGHT);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(iperf_menu_time), 30000, true, iperf_menu_time_time_update_cb);

    gui_obj_add_event_cb((gui_obj_t *)view, (gui_event_cb_t)app_wifi_testIperfMenuView_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)view);
}
GUI_VIEW_INSTANCE("app_wifi_testIperfMenuView", false, app_wifi_testIperfMenuView_switch_in,
                  app_wifi_testIperfMenuView_switch_out, false);

// Create app_wifi_testIperfUploadView (hg_view)
static void app_wifi_testIperfUploadView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_wifi_testIperfUploadView_switch_in(gui_view_t *view)
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



    // Create iperf_ip_display_bg (hg_rect)
    iperf_ip_display_bg = gui_rect_create((gui_obj_t *)view, "iperf_ip_display_bg", 24, 132, 362, 56,
                                          14, gui_rgba(255, 255, 255, 15));

    // Create iperf_ip_seg0_bg (hg_rect)
    iperf_ip_seg0_bg = gui_rect_create((gui_obj_t *)view, "iperf_ip_seg0_bg", 30, 140, 76, 40, 8,
                                       gui_rgba(90, 200, 250, 38));

    // Create iperf_ip_seg1_bg (hg_rect)
    iperf_ip_seg1_bg = gui_rect_create((gui_obj_t *)view, "iperf_ip_seg1_bg", 122, 140, 76, 40, 8,
                                       gui_rgba(255, 255, 255, 12));

    // Create iperf_ip_seg2_bg (hg_rect)
    iperf_ip_seg2_bg = gui_rect_create((gui_obj_t *)view, "iperf_ip_seg2_bg", 214, 140, 76, 40, 8,
                                       gui_rgba(255, 255, 255, 12));

    // Create iperf_ip_seg3_bg (hg_rect)
    iperf_ip_seg3_bg = gui_rect_create((gui_obj_t *)view, "iperf_ip_seg3_bg", 306, 140, 76, 40, 8,
                                       gui_rgba(255, 255, 255, 12));

    // Create iperf_key_1_bg (hg_rect)
    iperf_key_1_bg = gui_rect_create((gui_obj_t *)view, "iperf_key_1_bg", 24, 210, 86, 52, 12,
                                     gui_rgba(255, 255, 255, 31));
    gui_obj_add_event_cb(iperf_key_1_bg, (gui_event_cb_t)iperf_key_1_bg_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create iperf_key_2_bg (hg_rect)
    iperf_key_2_bg = gui_rect_create((gui_obj_t *)view, "iperf_key_2_bg", 115, 210, 86, 52, 12,
                                     gui_rgba(255, 255, 255, 31));
    gui_obj_add_event_cb(iperf_key_2_bg, (gui_event_cb_t)iperf_key_2_bg_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create iperf_key_3_bg (hg_rect)
    iperf_key_3_bg = gui_rect_create((gui_obj_t *)view, "iperf_key_3_bg", 206, 210, 86, 52, 12,
                                     gui_rgba(255, 255, 255, 31));
    gui_obj_add_event_cb(iperf_key_3_bg, (gui_event_cb_t)iperf_key_3_bg_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create iperf_key_4_bg (hg_rect)
    iperf_key_4_bg = gui_rect_create((gui_obj_t *)view, "iperf_key_4_bg", 297, 210, 86, 52, 12,
                                     gui_rgba(255, 255, 255, 31));
    gui_obj_add_event_cb(iperf_key_4_bg, (gui_event_cb_t)iperf_key_4_bg_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create iperf_key_5_bg (hg_rect)
    iperf_key_5_bg = gui_rect_create((gui_obj_t *)view, "iperf_key_5_bg", 24, 267, 86, 52, 12,
                                     gui_rgba(255, 255, 255, 31));
    gui_obj_add_event_cb(iperf_key_5_bg, (gui_event_cb_t)iperf_key_5_bg_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create iperf_key_6_bg (hg_rect)
    iperf_key_6_bg = gui_rect_create((gui_obj_t *)view, "iperf_key_6_bg", 115, 267, 86, 52, 12,
                                     gui_rgba(255, 255, 255, 31));
    gui_obj_add_event_cb(iperf_key_6_bg, (gui_event_cb_t)iperf_key_6_bg_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create iperf_key_7_bg (hg_rect)
    iperf_key_7_bg = gui_rect_create((gui_obj_t *)view, "iperf_key_7_bg", 206, 267, 86, 52, 12,
                                     gui_rgba(255, 255, 255, 31));
    gui_obj_add_event_cb(iperf_key_7_bg, (gui_event_cb_t)iperf_key_7_bg_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create iperf_key_8_bg (hg_rect)
    iperf_key_8_bg = gui_rect_create((gui_obj_t *)view, "iperf_key_8_bg", 297, 267, 86, 52, 12,
                                     gui_rgba(255, 255, 255, 31));
    gui_obj_add_event_cb(iperf_key_8_bg, (gui_event_cb_t)iperf_key_8_bg_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create iperf_key_9_bg (hg_rect)
    iperf_key_9_bg = gui_rect_create((gui_obj_t *)view, "iperf_key_9_bg", 24, 324, 86, 52, 12,
                                     gui_rgba(255, 255, 255, 31));
    gui_obj_add_event_cb(iperf_key_9_bg, (gui_event_cb_t)iperf_key_9_bg_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create iperf_key_dot_bg (hg_rect)
    iperf_key_dot_bg = gui_rect_create((gui_obj_t *)view, "iperf_key_dot_bg", 115, 324, 86, 52, 12,
                                       gui_rgba(255, 255, 255, 31));
    gui_obj_add_event_cb(iperf_key_dot_bg, (gui_event_cb_t)iperf_key_dot_bg_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create iperf_key_0_bg (hg_rect)
    iperf_key_0_bg = gui_rect_create((gui_obj_t *)view, "iperf_key_0_bg", 206, 324, 86, 52, 12,
                                     gui_rgba(255, 255, 255, 31));
    gui_obj_add_event_cb(iperf_key_0_bg, (gui_event_cb_t)iperf_key_0_bg_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create iperf_key_del_bg (hg_rect)
    iperf_key_del_bg = gui_rect_create((gui_obj_t *)view, "iperf_key_del_bg", 297, 324, 86, 52, 12,
                                       gui_rgba(255, 255, 255, 31));
    gui_obj_add_event_cb(iperf_key_del_bg, (gui_event_cb_t)iperf_key_del_bg_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create iperf_btn_confirm_bg (hg_rect)
    iperf_btn_confirm_bg = gui_rect_create((gui_obj_t *)view, "iperf_btn_confirm_bg", 24, 386, 178, 52,
                                           12, gui_rgb(90, 200, 250));
    gui_obj_add_event_cb(iperf_btn_confirm_bg, (gui_event_cb_t)iperf_btn_confirm_bg_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create iperf_btn_start_bg (hg_rect)
    iperf_btn_start_bg = gui_rect_create((gui_obj_t *)view, "iperf_btn_start_bg", 208, 386, 178, 52, 12,
                                         gui_rgba(255, 255, 255, 15));
    gui_obj_add_event_cb(iperf_btn_start_bg, (gui_event_cb_t)iperf_btn_start_bg_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create iperf_upload_title (hg_label)
    iperf_upload_title = gui_scroll_text_create((gui_obj_t *)view, "iperf_upload_title", 80, 80, 250,
                                                50);
    gui_scroll_text_set((gui_scroll_text_t *)iperf_upload_title, "iPerf Upload", GUI_FONT_SRC_BMP,
                        gui_rgb(242, 242, 242), 12, 40);
    gui_scroll_text_type_set((gui_scroll_text_t *)iperf_upload_title,
                             "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_scroll_text_scroll_set((gui_scroll_text_t *)iperf_upload_title, SCROLL_X_MID, 0, 0, 3000, 0);

    // Create iperf_ip_seg0 (hg_label)
    iperf_ip_seg0 = gui_text_create((gui_obj_t *)view, "iperf_ip_seg0", 30, 140, 76, 40);
    gui_text_set((gui_text_t *)iperf_ip_seg0, "---", GUI_FONT_SRC_BMP, gui_rgb(90, 200, 250), 3, 22);
    gui_text_type_set((gui_text_t *)iperf_ip_seg0, "/font/Inter_24pt_Regular_size22_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)iperf_ip_seg0, MID_CENTER);

    // Create iperf_ip_dot0 (hg_label)
    iperf_ip_dot0 = gui_text_create((gui_obj_t *)view, "iperf_ip_dot0", 106, 140, 16, 40);
    gui_text_set((gui_text_t *)iperf_ip_dot0, ".", GUI_FONT_SRC_BMP, gui_rgb(85, 85, 85), 1, 24);
    gui_text_type_set((gui_text_t *)iperf_ip_dot0, "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)iperf_ip_dot0, MID_CENTER);

    // Create iperf_ip_seg1 (hg_label)
    iperf_ip_seg1 = gui_text_create((gui_obj_t *)view, "iperf_ip_seg1", 122, 140, 76, 40);
    gui_text_set((gui_text_t *)iperf_ip_seg1, "---", GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 3, 22);
    gui_text_type_set((gui_text_t *)iperf_ip_seg1, "/font/Inter_24pt_Regular_size22_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)iperf_ip_seg1, MID_CENTER);

    // Create iperf_ip_dot1 (hg_label)
    iperf_ip_dot1 = gui_text_create((gui_obj_t *)view, "iperf_ip_dot1", 198, 140, 16, 40);
    gui_text_set((gui_text_t *)iperf_ip_dot1, ".", GUI_FONT_SRC_BMP, gui_rgb(85, 85, 85), 1, 24);
    gui_text_type_set((gui_text_t *)iperf_ip_dot1, "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)iperf_ip_dot1, MID_CENTER);

    // Create iperf_ip_seg2 (hg_label)
    iperf_ip_seg2 = gui_text_create((gui_obj_t *)view, "iperf_ip_seg2", 214, 140, 76, 40);
    gui_text_set((gui_text_t *)iperf_ip_seg2, "---", GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 3, 22);
    gui_text_type_set((gui_text_t *)iperf_ip_seg2, "/font/Inter_24pt_Regular_size22_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)iperf_ip_seg2, MID_CENTER);

    // Create iperf_ip_dot2 (hg_label)
    iperf_ip_dot2 = gui_text_create((gui_obj_t *)view, "iperf_ip_dot2", 290, 140, 16, 40);
    gui_text_set((gui_text_t *)iperf_ip_dot2, ".", GUI_FONT_SRC_BMP, gui_rgb(85, 85, 85), 1, 24);
    gui_text_type_set((gui_text_t *)iperf_ip_dot2, "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)iperf_ip_dot2, MID_CENTER);

    // Create iperf_ip_seg3 (hg_label)
    iperf_ip_seg3 = gui_text_create((gui_obj_t *)view, "iperf_ip_seg3", 306, 140, 76, 40);
    gui_text_set((gui_text_t *)iperf_ip_seg3, "---", GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 3, 22);
    gui_text_type_set((gui_text_t *)iperf_ip_seg3, "/font/Inter_24pt_Regular_size22_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)iperf_ip_seg3, MID_CENTER);

    // Create iperf_ip_confirmed (hg_label)
    iperf_ip_confirmed = gui_text_create((gui_obj_t *)view, "iperf_ip_confirmed", 130, 186, 150, 20);
    gui_text_set((gui_text_t *)iperf_ip_confirmed, "IP Confirmed", GUI_FONT_SRC_BMP, gui_rgb(76, 217,
                 100), 12, 14);
    gui_text_type_set((gui_text_t *)iperf_ip_confirmed,
                      "/font/Inter_24pt_Regular_size14_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)iperf_ip_confirmed, MID_CENTER);
    gui_obj_hidden((gui_obj_t *)iperf_ip_confirmed, true);

    // Create iperf_key_1 (hg_label)
    iperf_key_1 = gui_text_create((gui_obj_t *)view, "iperf_key_1", 24, 210, 86, 52);
    gui_text_set((gui_text_t *)iperf_key_1, "1", GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 1, 24);
    gui_text_type_set((gui_text_t *)iperf_key_1, "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)iperf_key_1, MID_CENTER);
    gui_obj_add_event_cb(iperf_key_1, (gui_event_cb_t)iperf_key_1_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create iperf_key_2 (hg_label)
    iperf_key_2 = gui_text_create((gui_obj_t *)view, "iperf_key_2", 115, 210, 86, 52);
    gui_text_set((gui_text_t *)iperf_key_2, "2", GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 1, 24);
    gui_text_type_set((gui_text_t *)iperf_key_2, "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)iperf_key_2, MID_CENTER);
    gui_obj_add_event_cb(iperf_key_2, (gui_event_cb_t)iperf_key_2_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create iperf_key_3 (hg_label)
    iperf_key_3 = gui_text_create((gui_obj_t *)view, "iperf_key_3", 206, 210, 86, 52);
    gui_text_set((gui_text_t *)iperf_key_3, "3", GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 1, 24);
    gui_text_type_set((gui_text_t *)iperf_key_3, "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)iperf_key_3, MID_CENTER);
    gui_obj_add_event_cb(iperf_key_3, (gui_event_cb_t)iperf_key_3_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create iperf_key_4 (hg_label)
    iperf_key_4 = gui_text_create((gui_obj_t *)view, "iperf_key_4", 297, 210, 86, 52);
    gui_text_set((gui_text_t *)iperf_key_4, "4", GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 1, 24);
    gui_text_type_set((gui_text_t *)iperf_key_4, "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)iperf_key_4, MID_CENTER);
    gui_obj_add_event_cb(iperf_key_4, (gui_event_cb_t)iperf_key_4_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create iperf_key_5 (hg_label)
    iperf_key_5 = gui_text_create((gui_obj_t *)view, "iperf_key_5", 24, 267, 86, 52);
    gui_text_set((gui_text_t *)iperf_key_5, "5", GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 1, 24);
    gui_text_type_set((gui_text_t *)iperf_key_5, "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)iperf_key_5, MID_CENTER);
    gui_obj_add_event_cb(iperf_key_5, (gui_event_cb_t)iperf_key_5_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create iperf_key_6 (hg_label)
    iperf_key_6 = gui_text_create((gui_obj_t *)view, "iperf_key_6", 115, 267, 86, 52);
    gui_text_set((gui_text_t *)iperf_key_6, "6", GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 1, 24);
    gui_text_type_set((gui_text_t *)iperf_key_6, "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)iperf_key_6, MID_CENTER);
    gui_obj_add_event_cb(iperf_key_6, (gui_event_cb_t)iperf_key_6_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create iperf_key_7 (hg_label)
    iperf_key_7 = gui_text_create((gui_obj_t *)view, "iperf_key_7", 206, 267, 86, 52);
    gui_text_set((gui_text_t *)iperf_key_7, "7", GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 1, 24);
    gui_text_type_set((gui_text_t *)iperf_key_7, "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)iperf_key_7, MID_CENTER);
    gui_obj_add_event_cb(iperf_key_7, (gui_event_cb_t)iperf_key_7_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create iperf_key_8 (hg_label)
    iperf_key_8 = gui_text_create((gui_obj_t *)view, "iperf_key_8", 297, 267, 86, 52);
    gui_text_set((gui_text_t *)iperf_key_8, "8", GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 1, 24);
    gui_text_type_set((gui_text_t *)iperf_key_8, "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)iperf_key_8, MID_CENTER);
    gui_obj_add_event_cb(iperf_key_8, (gui_event_cb_t)iperf_key_8_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create iperf_key_9 (hg_label)
    iperf_key_9 = gui_text_create((gui_obj_t *)view, "iperf_key_9", 24, 324, 86, 52);
    gui_text_set((gui_text_t *)iperf_key_9, "9", GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 1, 24);
    gui_text_type_set((gui_text_t *)iperf_key_9, "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)iperf_key_9, MID_CENTER);
    gui_obj_add_event_cb(iperf_key_9, (gui_event_cb_t)iperf_key_9_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create iperf_key_dot (hg_label)
    iperf_key_dot = gui_text_create((gui_obj_t *)view, "iperf_key_dot", 115, 324, 86, 52);
    gui_text_set((gui_text_t *)iperf_key_dot, ".", GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 1, 28);
    gui_text_type_set((gui_text_t *)iperf_key_dot, "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)iperf_key_dot, MID_CENTER);
    gui_obj_add_event_cb(iperf_key_dot, (gui_event_cb_t)iperf_key_dot_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create iperf_key_0 (hg_label)
    iperf_key_0 = gui_text_create((gui_obj_t *)view, "iperf_key_0", 206, 324, 86, 52);
    gui_text_set((gui_text_t *)iperf_key_0, "0", GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 1, 24);
    gui_text_type_set((gui_text_t *)iperf_key_0, "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)iperf_key_0, MID_CENTER);
    gui_obj_add_event_cb(iperf_key_0, (gui_event_cb_t)iperf_key_0_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create iperf_key_del (hg_label)
    iperf_key_del = gui_text_create((gui_obj_t *)view, "iperf_key_del", 297, 324, 86, 52);
    gui_text_set((gui_text_t *)iperf_key_del, "DEL", GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 3, 16);
    gui_text_type_set((gui_text_t *)iperf_key_del, "/font/Inter_24pt_Regular_size16_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)iperf_key_del, MID_CENTER);
    gui_obj_add_event_cb(iperf_key_del, (gui_event_cb_t)iperf_key_del_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create iperf_btn_confirm (hg_label)
    iperf_btn_confirm = gui_text_create((gui_obj_t *)view, "iperf_btn_confirm", 24, 386, 178, 52);
    gui_text_set((gui_text_t *)iperf_btn_confirm, "Confirm", GUI_FONT_SRC_BMP, gui_rgb(0, 0, 0), 7, 20);
    gui_text_type_set((gui_text_t *)iperf_btn_confirm,
                      "/font/Inter_24pt_Regular_size20_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)iperf_btn_confirm, MID_CENTER);
    gui_obj_add_event_cb(iperf_btn_confirm, (gui_event_cb_t)iperf_btn_confirm_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create iperf_btn_start (hg_label)
    iperf_btn_start = gui_text_create((gui_obj_t *)view, "iperf_btn_start", 208, 386, 178, 52);
    gui_text_set((gui_text_t *)iperf_btn_start, "Start", GUI_FONT_SRC_BMP, gui_rgb(85, 85, 85), 5, 20);
    gui_text_type_set((gui_text_t *)iperf_btn_start, "/font/Inter_24pt_Regular_size20_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)iperf_btn_start, MID_CENTER);
    gui_obj_add_event_cb(iperf_btn_start, (gui_event_cb_t)iperf_btn_start_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create iperf_upload_top_window (hg_window)
    iperf_upload_top_window = gui_win_create((gui_obj_t *)view, "iperf_upload_top_window", 0, 0, 410,
                                             100);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(iperf_upload_time_time_str, sizeof(iperf_upload_time_time_str), "%02d:%02d", t->tm_hour,
                 t->tm_min);
    }


    // Create iperf_upload_back_touch_win (hg_window)
    iperf_upload_back_touch_win = gui_win_create(iperf_upload_top_window, "iperf_upload_back_touch_win",
                                                 0, 0, 100, 100);


    // Create iperf_upload_back_btn (hg_image)
    iperf_upload_back_btn = gui_img_create_from_fs(iperf_upload_back_touch_win, "iperf_upload_back_btn",
                                                   "/app_wifi_test/back_icon.bin", 34, 30, 28, 28);

    gui_obj_add_event_cb(GUI_BASE(iperf_upload_back_touch_win), iperf_upload_back,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create iperf_upload_time (hg_time_label)
    iperf_upload_time = gui_text_create(iperf_upload_top_window, "iperf_upload_time", 300, 20, 80, 32);
    gui_text_set((gui_text_t *)iperf_upload_time, iperf_upload_time_time_str, GUI_FONT_SRC_BMP,
                 gui_rgb(255, 255, 255), strlen(iperf_upload_time_time_str), 32);
    gui_text_type_set((gui_text_t *)iperf_upload_time,
                      "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)iperf_upload_time, RIGHT);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(iperf_upload_time), 30000, true, iperf_upload_time_time_update_cb);

    gui_obj_add_event_cb((gui_obj_t *)view, (gui_event_cb_t)app_wifi_testIperfUploadView_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)view);
}
GUI_VIEW_INSTANCE("app_wifi_testIperfUploadView", false, app_wifi_testIperfUploadView_switch_in,
                  app_wifi_testIperfUploadView_switch_out, false);

// Create app_wifi_testIperfUploadRunningView (hg_view)
static void app_wifi_testIperfUploadRunningView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_wifi_testIperfUploadRunningView_switch_in(gui_view_t *view)
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



    // Create iperf_upload_ring_bg (hg_arc)
    iperf_upload_ring_bg = gui_arc_create((gui_obj_t *)view, "iperf_upload_ring_bg", 205, 250, 96, 0,
                                          360, 4, gui_rgba(90, 200, 250, 40));

    // Create iperf_upload_run_title (hg_label)
    iperf_upload_run_title = gui_scroll_text_create((gui_obj_t *)view, "iperf_upload_run_title", 0, 80,
                                                    410, 50);
    gui_scroll_text_set((gui_scroll_text_t *)iperf_upload_run_title, "iPerf Upload Running",
                        GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 20, 40);
    gui_scroll_text_type_set((gui_scroll_text_t *)iperf_upload_run_title,
                             "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_scroll_text_scroll_set((gui_scroll_text_t *)iperf_upload_run_title, SCROLL_X_MID, 0, 0, 3000,
                               0);

    // Create iperf_upload_ring (hg_arc)
    iperf_upload_ring = gui_arc_create((gui_obj_t *)view, "iperf_upload_ring", 205, 250, 96, 0, 360, 4,
                                       gui_rgb(90, 200, 250));

    // Create iperf_upload_run_top_window (hg_window)
    iperf_upload_run_top_window = gui_win_create((gui_obj_t *)view, "iperf_upload_run_top_window", 0, 0,
                                                 410, 100);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(iperf_upload_run_time_time_str, sizeof(iperf_upload_run_time_time_str), "%02d:%02d",
                 t->tm_hour, t->tm_min);
    }


    // Create iperf_upload_run_back_touch_win (hg_window)
    iperf_upload_run_back_touch_win = gui_win_create(iperf_upload_run_top_window,
                                                     "iperf_upload_run_back_touch_win", 0, 0, 100, 100);


    // Create iperf_upload_run_back_btn (hg_image)
    iperf_upload_run_back_btn = gui_img_create_from_fs(iperf_upload_run_back_touch_win,
                                                       "iperf_upload_run_back_btn", "/app_wifi_test/back_icon.bin", 34, 30, 28, 28);

    gui_obj_add_event_cb(GUI_BASE(iperf_upload_run_back_touch_win),
                         (gui_event_cb_t)iperf_upload_run_back_touch_win_clicked_0_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create iperf_upload_run_time (hg_time_label)
    iperf_upload_run_time = gui_text_create(iperf_upload_run_top_window, "iperf_upload_run_time", 300,
                                            20, 80, 32);
    gui_text_set((gui_text_t *)iperf_upload_run_time, iperf_upload_run_time_time_str, GUI_FONT_SRC_BMP,
                 gui_rgb(255, 255, 255), strlen(iperf_upload_run_time_time_str), 32);
    gui_text_type_set((gui_text_t *)iperf_upload_run_time,
                      "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)iperf_upload_run_time, RIGHT);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(iperf_upload_run_time), 30000, true,
                         iperf_upload_run_time_time_update_cb);

    // Create iperf_upload_speed (hg_label)
    iperf_upload_speed = gui_text_create((gui_obj_t *)view, "iperf_upload_speed", 130, 215, 150, 50);
    gui_text_set((gui_text_t *)iperf_upload_speed, "0", GUI_FONT_SRC_BMP, gui_rgb(90, 200, 250), 1, 40);
    gui_text_type_set((gui_text_t *)iperf_upload_speed,
                      "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)iperf_upload_speed, MID_CENTER);
    // Bind timer: speed_tick
    gui_obj_create_timer((gui_obj_t *)iperf_upload_speed, 500, true, iperf_upload_speed_tick_cb);
    gui_obj_start_timer((gui_obj_t *)iperf_upload_speed);

    // Create iperf_upload_unit (hg_label)
    iperf_upload_unit = gui_text_create((gui_obj_t *)view, "iperf_upload_unit", 130, 262, 150, 24);
    gui_text_set((gui_text_t *)iperf_upload_unit, "KByte/s", GUI_FONT_SRC_BMP, gui_rgb(153, 153, 153),
                 7, 18);
    gui_text_type_set((gui_text_t *)iperf_upload_unit,
                      "/font/Inter_24pt_Regular_size18_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)iperf_upload_unit, MID_CENTER);

    // Create iperf_upload_target (hg_label)
    iperf_upload_target = gui_text_create((gui_obj_t *)view, "iperf_upload_target", 80, 370, 250, 24);
    gui_text_set((gui_text_t *)iperf_upload_target, "Target: 0.0.0.0", GUI_FONT_SRC_BMP, gui_rgb(102,
                 102, 102), 15, 16);
    gui_text_type_set((gui_text_t *)iperf_upload_target,
                      "/font/Inter_24pt_Regular_size16_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)iperf_upload_target, MID_CENTER);

    gui_obj_add_event_cb((gui_obj_t *)view,
                         (gui_event_cb_t)app_wifi_testIperfUploadRunningView_key_0_cb, GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)view);
}
GUI_VIEW_INSTANCE("app_wifi_testIperfUploadRunningView", false,
                  app_wifi_testIperfUploadRunningView_switch_in, app_wifi_testIperfUploadRunningView_switch_out,
                  false);

// Create app_wifi_testIperfDownloadView (hg_view)
static void app_wifi_testIperfDownloadView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_wifi_testIperfDownloadView_switch_in(gui_view_t *view)
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



    // Create iperf_download_soc_ip_bg (hg_rect)
    iperf_download_soc_ip_bg = gui_rect_create((gui_obj_t *)view, "iperf_download_soc_ip_bg", 24, 140,
                                               362, 84, 16, gui_rgba(255, 255, 255, 20));

    // Create iperf_download_title (hg_label)
    iperf_download_title = gui_scroll_text_create((gui_obj_t *)view, "iperf_download_title", 0, 80, 410,
                                                  50);
    gui_scroll_text_set((gui_scroll_text_t *)iperf_download_title, "iPerf Download", GUI_FONT_SRC_BMP,
                        gui_rgb(242, 242, 242), 14, 40);
    gui_scroll_text_type_set((gui_scroll_text_t *)iperf_download_title,
                             "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_scroll_text_scroll_set((gui_scroll_text_t *)iperf_download_title, SCROLL_X_MID, 0, 0, 3000, 0);

    // Create iperf_download_soc_ip (hg_label)
    iperf_download_soc_ip = gui_text_create((gui_obj_t *)view, "iperf_download_soc_ip", 24, 140, 362,
                                            84);
    gui_text_set((gui_text_t *)iperf_download_soc_ip, "SoC IP: 192.168.1.100", GUI_FONT_SRC_BMP,
                 gui_rgb(242, 242, 242), 21, 40);
    gui_text_type_set((gui_text_t *)iperf_download_soc_ip,
                      "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)iperf_download_soc_ip, MID_CENTER);

    // Create iperf_dl_running_win (hg_window)
    iperf_dl_running_win = gui_win_create((gui_obj_t *)view, "iperf_dl_running_win", 0, 230, 410, 230);


    // Create iperf_dl_ring_bg (hg_arc)
    iperf_dl_ring_bg = gui_arc_create(iperf_dl_running_win, "iperf_dl_ring_bg", 205, 95, 86, 0, 360, 4,
                                      gui_rgba(76, 217, 100, 40));

    // Create iperf_dl_ring (hg_arc)
    iperf_dl_ring = gui_arc_create(iperf_dl_running_win, "iperf_dl_ring", 205, 95, 86, 0, 360, 4,
                                   gui_rgb(76, 217, 100));

    // Create iperf_dl_speed (hg_label)
    iperf_dl_speed = gui_text_create(iperf_dl_running_win, "iperf_dl_speed", 130, 55, 150, 50);
    gui_text_set((gui_text_t *)iperf_dl_speed, "0", GUI_FONT_SRC_BMP, gui_rgb(76, 217, 100), 1, 36);
    gui_text_type_set((gui_text_t *)iperf_dl_speed, "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)iperf_dl_speed, MID_CENTER);
    // Bind timer: speed_tick
    gui_obj_create_timer((gui_obj_t *)iperf_dl_speed, 500, true, iperf_dl_speed_tick_cb);
    gui_obj_start_timer((gui_obj_t *)iperf_dl_speed);

    // Create iperf_dl_unit (hg_label)
    iperf_dl_unit = gui_text_create(iperf_dl_running_win, "iperf_dl_unit", 130, 100, 150, 24);
    gui_text_set((gui_text_t *)iperf_dl_unit, "KByte/s", GUI_FONT_SRC_BMP, gui_rgb(153, 153, 153), 7,
                 16);
    gui_text_type_set((gui_text_t *)iperf_dl_unit, "/font/Inter_24pt_Regular_size16_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)iperf_dl_unit, MID_CENTER);

    // Create iperf_download_top_window (hg_window)
    iperf_download_top_window = gui_win_create((gui_obj_t *)view, "iperf_download_top_window", 0, 0,
                                               410, 100);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(iperf_download_time_time_str, sizeof(iperf_download_time_time_str), "%02d:%02d",
                 t->tm_hour, t->tm_min);
    }


    // Create iperf_download_back_touch_win (hg_window)
    iperf_download_back_touch_win = gui_win_create(iperf_download_top_window,
                                                   "iperf_download_back_touch_win", 0, 0, 100, 100);


    // Create iperf_download_back_btn (hg_image)
    iperf_download_back_btn = gui_img_create_from_fs(iperf_download_back_touch_win,
                                                     "iperf_download_back_btn", "/app_wifi_test/back_icon.bin", 34, 30, 28, 28);

    gui_obj_add_event_cb(GUI_BASE(iperf_download_back_touch_win), iperf_download_back,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create iperf_download_time (hg_time_label)
    iperf_download_time = gui_text_create(iperf_download_top_window, "iperf_download_time", 300, 20, 80,
                                          32);
    gui_text_set((gui_text_t *)iperf_download_time, iperf_download_time_time_str, GUI_FONT_SRC_BMP,
                 gui_rgb(255, 255, 255), strlen(iperf_download_time_time_str), 32);
    gui_text_type_set((gui_text_t *)iperf_download_time,
                      "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)iperf_download_time, RIGHT);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(iperf_download_time), 30000, true,
                         iperf_download_time_time_update_cb);

    gui_obj_add_event_cb((gui_obj_t *)view, (gui_event_cb_t)app_wifi_testIperfDownloadView_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)view);
}
GUI_VIEW_INSTANCE("app_wifi_testIperfDownloadView", false, app_wifi_testIperfDownloadView_switch_in,
                  app_wifi_testIperfDownloadView_switch_out, false);

// Create app_wifi_testFileMenuView (hg_view)
static void app_wifi_testFileMenuView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_wifi_testFileMenuView_switch_in(gui_view_t *view)
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



    // Create file_menu_title (hg_label)
    file_menu_title = gui_scroll_text_create((gui_obj_t *)view, "file_menu_title", 24, 137, 362, 50);
    gui_scroll_text_set((gui_scroll_text_t *)file_menu_title, "File Transfer", GUI_FONT_SRC_BMP,
                        gui_rgb(242, 242, 242), 13, 40);
    gui_scroll_text_type_set((gui_scroll_text_t *)file_menu_title,
                             "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_scroll_text_scroll_set((gui_scroll_text_t *)file_menu_title, SCROLL_X_MID, 0, 0, 3000, 0);

    // Create file_menu_list (hg_list)
    file_menu_list = gui_list_create((gui_obj_t *)view, "file_menu_list", 0, 187, 410, 220, 84, 10,
                                     VERTICAL, file_menu_list_note_design, NULL, false);
    gui_list_set_style(file_menu_list, LIST_CLASSIC);
    gui_list_set_note_num(file_menu_list, 2);
    gui_list_set_out_scope(file_menu_list, 80);
    gui_list_keep_note_alive(file_menu_list, true);

    // Create file_menu_top_window (hg_window)
    file_menu_top_window = gui_win_create((gui_obj_t *)view, "file_menu_top_window", 0, 0, 410, 100);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(file_menu_time_time_str, sizeof(file_menu_time_time_str), "%02d:%02d", t->tm_hour,
                 t->tm_min);
    }


    // Create file_menu_back_touch_win (hg_window)
    file_menu_back_touch_win = gui_win_create(file_menu_top_window, "file_menu_back_touch_win", 0, 0,
                                              100, 100);


    // Create file_menu_back_btn (hg_image)
    file_menu_back_btn = gui_img_create_from_fs(file_menu_back_touch_win, "file_menu_back_btn",
                                                "/app_wifi_test/back_icon.bin", 34, 30, 28, 28);

    gui_obj_add_event_cb(GUI_BASE(file_menu_back_touch_win),
                         (gui_event_cb_t)file_menu_back_touch_win_clicked_0_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create file_menu_time (hg_time_label)
    file_menu_time = gui_text_create(file_menu_top_window, "file_menu_time", 300, 20, 80, 32);
    gui_text_set((gui_text_t *)file_menu_time, file_menu_time_time_str, GUI_FONT_SRC_BMP, gui_rgb(255,
                 255, 255), strlen(file_menu_time_time_str), 32);
    gui_text_type_set((gui_text_t *)file_menu_time, "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)file_menu_time, RIGHT);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(file_menu_time), 30000, true, file_menu_time_time_update_cb);

    gui_obj_add_event_cb((gui_obj_t *)view, (gui_event_cb_t)app_wifi_testFileMenuView_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)view);
}
GUI_VIEW_INSTANCE("app_wifi_testFileMenuView", false, app_wifi_testFileMenuView_switch_in,
                  app_wifi_testFileMenuView_switch_out, false);

// Create app_wifi_testFileUploadView (hg_view)
static void app_wifi_testFileUploadView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_wifi_testFileUploadView_switch_in(gui_view_t *view)
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



    // Create file_ip_display_bg (hg_rect)
    file_ip_display_bg = gui_rect_create((gui_obj_t *)view, "file_ip_display_bg", 24, 132, 362, 56, 14,
                                         gui_rgba(255, 255, 255, 15));

    // Create file_ip_seg0_bg (hg_rect)
    file_ip_seg0_bg = gui_rect_create((gui_obj_t *)view, "file_ip_seg0_bg", 30, 140, 76, 40, 8,
                                      gui_rgba(255, 149, 0, 38));

    // Create file_ip_seg1_bg (hg_rect)
    file_ip_seg1_bg = gui_rect_create((gui_obj_t *)view, "file_ip_seg1_bg", 122, 140, 76, 40, 8,
                                      gui_rgba(255, 255, 255, 12));

    // Create file_ip_seg2_bg (hg_rect)
    file_ip_seg2_bg = gui_rect_create((gui_obj_t *)view, "file_ip_seg2_bg", 214, 140, 76, 40, 8,
                                      gui_rgba(255, 255, 255, 12));

    // Create file_ip_seg3_bg (hg_rect)
    file_ip_seg3_bg = gui_rect_create((gui_obj_t *)view, "file_ip_seg3_bg", 306, 140, 76, 40, 8,
                                      gui_rgba(255, 255, 255, 12));

    // Create file_key_1_bg (hg_rect)
    file_key_1_bg = gui_rect_create((gui_obj_t *)view, "file_key_1_bg", 24, 210, 86, 52, 12,
                                    gui_rgba(255, 255, 255, 31));
    gui_obj_add_event_cb(file_key_1_bg, (gui_event_cb_t)file_key_1_bg_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create file_key_2_bg (hg_rect)
    file_key_2_bg = gui_rect_create((gui_obj_t *)view, "file_key_2_bg", 115, 210, 86, 52, 12,
                                    gui_rgba(255, 255, 255, 31));
    gui_obj_add_event_cb(file_key_2_bg, (gui_event_cb_t)file_key_2_bg_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create file_key_3_bg (hg_rect)
    file_key_3_bg = gui_rect_create((gui_obj_t *)view, "file_key_3_bg", 206, 210, 86, 52, 12,
                                    gui_rgba(255, 255, 255, 31));
    gui_obj_add_event_cb(file_key_3_bg, (gui_event_cb_t)file_key_3_bg_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create file_key_4_bg (hg_rect)
    file_key_4_bg = gui_rect_create((gui_obj_t *)view, "file_key_4_bg", 297, 210, 86, 52, 12,
                                    gui_rgba(255, 255, 255, 31));
    gui_obj_add_event_cb(file_key_4_bg, (gui_event_cb_t)file_key_4_bg_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create file_key_5_bg (hg_rect)
    file_key_5_bg = gui_rect_create((gui_obj_t *)view, "file_key_5_bg", 24, 267, 86, 52, 12,
                                    gui_rgba(255, 255, 255, 31));
    gui_obj_add_event_cb(file_key_5_bg, (gui_event_cb_t)file_key_5_bg_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create file_key_6_bg (hg_rect)
    file_key_6_bg = gui_rect_create((gui_obj_t *)view, "file_key_6_bg", 115, 267, 86, 52, 12,
                                    gui_rgba(255, 255, 255, 31));
    gui_obj_add_event_cb(file_key_6_bg, (gui_event_cb_t)file_key_6_bg_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create file_key_7_bg (hg_rect)
    file_key_7_bg = gui_rect_create((gui_obj_t *)view, "file_key_7_bg", 206, 267, 86, 52, 12,
                                    gui_rgba(255, 255, 255, 31));
    gui_obj_add_event_cb(file_key_7_bg, (gui_event_cb_t)file_key_7_bg_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create file_key_8_bg (hg_rect)
    file_key_8_bg = gui_rect_create((gui_obj_t *)view, "file_key_8_bg", 297, 267, 86, 52, 12,
                                    gui_rgba(255, 255, 255, 31));
    gui_obj_add_event_cb(file_key_8_bg, (gui_event_cb_t)file_key_8_bg_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create file_key_9_bg (hg_rect)
    file_key_9_bg = gui_rect_create((gui_obj_t *)view, "file_key_9_bg", 24, 324, 86, 52, 12,
                                    gui_rgba(255, 255, 255, 31));
    gui_obj_add_event_cb(file_key_9_bg, (gui_event_cb_t)file_key_9_bg_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create file_key_dot_bg (hg_rect)
    file_key_dot_bg = gui_rect_create((gui_obj_t *)view, "file_key_dot_bg", 115, 324, 86, 52, 12,
                                      gui_rgba(255, 255, 255, 31));
    gui_obj_add_event_cb(file_key_dot_bg, (gui_event_cb_t)file_key_dot_bg_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create file_key_0_bg (hg_rect)
    file_key_0_bg = gui_rect_create((gui_obj_t *)view, "file_key_0_bg", 206, 324, 86, 52, 12,
                                    gui_rgba(255, 255, 255, 31));
    gui_obj_add_event_cb(file_key_0_bg, (gui_event_cb_t)file_key_0_bg_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create file_key_del_bg (hg_rect)
    file_key_del_bg = gui_rect_create((gui_obj_t *)view, "file_key_del_bg", 297, 324, 86, 52, 12,
                                      gui_rgba(255, 255, 255, 31));
    gui_obj_add_event_cb(file_key_del_bg, (gui_event_cb_t)file_key_del_bg_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create file_btn_confirm_bg (hg_rect)
    file_btn_confirm_bg = gui_rect_create((gui_obj_t *)view, "file_btn_confirm_bg", 24, 386, 178, 52,
                                          12, gui_rgb(90, 200, 250));
    gui_obj_add_event_cb(file_btn_confirm_bg, (gui_event_cb_t)file_btn_confirm_bg_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create file_btn_start_bg (hg_rect)
    file_btn_start_bg = gui_rect_create((gui_obj_t *)view, "file_btn_start_bg", 208, 386, 178, 52, 12,
                                        gui_rgba(255, 255, 255, 15));
    gui_obj_add_event_cb(file_btn_start_bg, (gui_event_cb_t)file_btn_start_bg_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create file_upload_title (hg_label)
    file_upload_title = gui_scroll_text_create((gui_obj_t *)view, "file_upload_title", 80, 80, 250, 50);
    gui_scroll_text_set((gui_scroll_text_t *)file_upload_title, "File Upload", GUI_FONT_SRC_BMP,
                        gui_rgb(242, 242, 242), 11, 40);
    gui_scroll_text_type_set((gui_scroll_text_t *)file_upload_title,
                             "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_scroll_text_scroll_set((gui_scroll_text_t *)file_upload_title, SCROLL_X_MID, 0, 0, 3000, 0);

    // Create file_ip_seg0 (hg_label)
    file_ip_seg0 = gui_text_create((gui_obj_t *)view, "file_ip_seg0", 30, 140, 76, 40);
    gui_text_set((gui_text_t *)file_ip_seg0, "---", GUI_FONT_SRC_BMP, gui_rgb(255, 149, 0), 3, 22);
    gui_text_type_set((gui_text_t *)file_ip_seg0, "/font/Inter_24pt_Regular_size22_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)file_ip_seg0, MID_CENTER);

    // Create file_ip_dot0 (hg_label)
    file_ip_dot0 = gui_text_create((gui_obj_t *)view, "file_ip_dot0", 106, 140, 16, 40);
    gui_text_set((gui_text_t *)file_ip_dot0, ".", GUI_FONT_SRC_BMP, gui_rgb(85, 85, 85), 1, 24);
    gui_text_type_set((gui_text_t *)file_ip_dot0, "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)file_ip_dot0, MID_CENTER);

    // Create file_ip_seg1 (hg_label)
    file_ip_seg1 = gui_text_create((gui_obj_t *)view, "file_ip_seg1", 122, 140, 76, 40);
    gui_text_set((gui_text_t *)file_ip_seg1, "---", GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 3, 22);
    gui_text_type_set((gui_text_t *)file_ip_seg1, "/font/Inter_24pt_Regular_size22_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)file_ip_seg1, MID_CENTER);

    // Create file_ip_dot1 (hg_label)
    file_ip_dot1 = gui_text_create((gui_obj_t *)view, "file_ip_dot1", 198, 140, 16, 40);
    gui_text_set((gui_text_t *)file_ip_dot1, ".", GUI_FONT_SRC_BMP, gui_rgb(85, 85, 85), 1, 24);
    gui_text_type_set((gui_text_t *)file_ip_dot1, "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)file_ip_dot1, MID_CENTER);

    // Create file_ip_seg2 (hg_label)
    file_ip_seg2 = gui_text_create((gui_obj_t *)view, "file_ip_seg2", 214, 140, 76, 40);
    gui_text_set((gui_text_t *)file_ip_seg2, "---", GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 3, 22);
    gui_text_type_set((gui_text_t *)file_ip_seg2, "/font/Inter_24pt_Regular_size22_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)file_ip_seg2, MID_CENTER);

    // Create file_ip_dot2 (hg_label)
    file_ip_dot2 = gui_text_create((gui_obj_t *)view, "file_ip_dot2", 290, 140, 16, 40);
    gui_text_set((gui_text_t *)file_ip_dot2, ".", GUI_FONT_SRC_BMP, gui_rgb(85, 85, 85), 1, 24);
    gui_text_type_set((gui_text_t *)file_ip_dot2, "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)file_ip_dot2, MID_CENTER);

    // Create file_ip_seg3 (hg_label)
    file_ip_seg3 = gui_text_create((gui_obj_t *)view, "file_ip_seg3", 306, 140, 76, 40);
    gui_text_set((gui_text_t *)file_ip_seg3, "---", GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 3, 22);
    gui_text_type_set((gui_text_t *)file_ip_seg3, "/font/Inter_24pt_Regular_size22_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)file_ip_seg3, MID_CENTER);

    // Create file_ip_confirmed (hg_label)
    file_ip_confirmed = gui_text_create((gui_obj_t *)view, "file_ip_confirmed", 130, 186, 150, 20);
    gui_text_set((gui_text_t *)file_ip_confirmed, "IP Confirmed", GUI_FONT_SRC_BMP, gui_rgb(76, 217,
                 100), 12, 14);
    gui_text_type_set((gui_text_t *)file_ip_confirmed,
                      "/font/Inter_24pt_Regular_size14_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)file_ip_confirmed, MID_CENTER);
    gui_obj_hidden((gui_obj_t *)file_ip_confirmed, true);

    // Create file_key_1 (hg_label)
    file_key_1 = gui_text_create((gui_obj_t *)view, "file_key_1", 24, 210, 86, 52);
    gui_text_set((gui_text_t *)file_key_1, "1", GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 1, 4);
    gui_text_type_set((gui_text_t *)file_key_1, "/font/Inter_24pt_Regular_size4_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)file_key_1, MID_CENTER);
    gui_obj_add_event_cb(file_key_1, (gui_event_cb_t)file_key_1_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create file_key_2 (hg_label)
    file_key_2 = gui_text_create((gui_obj_t *)view, "file_key_2", 115, 210, 86, 52);
    gui_text_set((gui_text_t *)file_key_2, "2", GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 1, 24);
    gui_text_type_set((gui_text_t *)file_key_2, "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)file_key_2, MID_CENTER);
    gui_obj_add_event_cb(file_key_2, (gui_event_cb_t)file_key_2_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create file_key_3 (hg_label)
    file_key_3 = gui_text_create((gui_obj_t *)view, "file_key_3", 206, 210, 86, 52);
    gui_text_set((gui_text_t *)file_key_3, "3", GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 1, 24);
    gui_text_type_set((gui_text_t *)file_key_3, "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)file_key_3, MID_CENTER);
    gui_obj_add_event_cb(file_key_3, (gui_event_cb_t)file_key_3_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create file_key_4 (hg_label)
    file_key_4 = gui_text_create((gui_obj_t *)view, "file_key_4", 297, 210, 86, 52);
    gui_text_set((gui_text_t *)file_key_4, "4", GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 1, 24);
    gui_text_type_set((gui_text_t *)file_key_4, "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)file_key_4, MID_CENTER);
    gui_obj_add_event_cb(file_key_4, (gui_event_cb_t)file_key_4_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create file_key_5 (hg_label)
    file_key_5 = gui_text_create((gui_obj_t *)view, "file_key_5", 24, 267, 86, 52);
    gui_text_set((gui_text_t *)file_key_5, "5", GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 1, 24);
    gui_text_type_set((gui_text_t *)file_key_5, "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)file_key_5, MID_CENTER);
    gui_obj_add_event_cb(file_key_5, (gui_event_cb_t)file_key_5_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create file_key_6 (hg_label)
    file_key_6 = gui_text_create((gui_obj_t *)view, "file_key_6", 115, 267, 86, 52);
    gui_text_set((gui_text_t *)file_key_6, "6", GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 1, 24);
    gui_text_type_set((gui_text_t *)file_key_6, "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)file_key_6, MID_CENTER);
    gui_obj_add_event_cb(file_key_6, (gui_event_cb_t)file_key_6_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create file_key_7 (hg_label)
    file_key_7 = gui_text_create((gui_obj_t *)view, "file_key_7", 206, 267, 86, 52);
    gui_text_set((gui_text_t *)file_key_7, "7", GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 1, 24);
    gui_text_type_set((gui_text_t *)file_key_7, "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)file_key_7, MID_CENTER);
    gui_obj_add_event_cb(file_key_7, (gui_event_cb_t)file_key_7_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create file_key_8 (hg_label)
    file_key_8 = gui_text_create((gui_obj_t *)view, "file_key_8", 297, 267, 86, 52);
    gui_text_set((gui_text_t *)file_key_8, "8", GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 1, 24);
    gui_text_type_set((gui_text_t *)file_key_8, "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)file_key_8, MID_CENTER);
    gui_obj_add_event_cb(file_key_8, (gui_event_cb_t)file_key_8_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create file_key_9 (hg_label)
    file_key_9 = gui_text_create((gui_obj_t *)view, "file_key_9", 24, 324, 86, 52);
    gui_text_set((gui_text_t *)file_key_9, "9", GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 1, 24);
    gui_text_type_set((gui_text_t *)file_key_9, "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)file_key_9, MID_CENTER);
    gui_obj_add_event_cb(file_key_9, (gui_event_cb_t)file_key_9_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create file_key_dot (hg_label)
    file_key_dot = gui_text_create((gui_obj_t *)view, "file_key_dot", 115, 324, 86, 52);
    gui_text_set((gui_text_t *)file_key_dot, ".", GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 1, 28);
    gui_text_type_set((gui_text_t *)file_key_dot, "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)file_key_dot, MID_CENTER);
    gui_obj_add_event_cb(file_key_dot, (gui_event_cb_t)file_key_dot_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create file_key_0 (hg_label)
    file_key_0 = gui_text_create((gui_obj_t *)view, "file_key_0", 206, 324, 86, 52);
    gui_text_set((gui_text_t *)file_key_0, "0", GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 1, 24);
    gui_text_type_set((gui_text_t *)file_key_0, "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)file_key_0, MID_CENTER);
    gui_obj_add_event_cb(file_key_0, (gui_event_cb_t)file_key_0_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create file_key_del (hg_label)
    file_key_del = gui_text_create((gui_obj_t *)view, "file_key_del", 297, 324, 86, 52);
    gui_text_set((gui_text_t *)file_key_del, "DEL", GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 3, 16);
    gui_text_type_set((gui_text_t *)file_key_del, "/font/Inter_24pt_Regular_size16_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)file_key_del, MID_CENTER);
    gui_obj_add_event_cb(file_key_del, (gui_event_cb_t)file_key_del_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create file_btn_confirm (hg_label)
    file_btn_confirm = gui_text_create((gui_obj_t *)view, "file_btn_confirm", 24, 386, 178, 52);
    gui_text_set((gui_text_t *)file_btn_confirm, "Confirm", GUI_FONT_SRC_BMP, gui_rgb(0, 0, 0), 7, 20);
    gui_text_type_set((gui_text_t *)file_btn_confirm,
                      "/font/Inter_24pt_Regular_size20_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)file_btn_confirm, MID_CENTER);
    gui_obj_add_event_cb(file_btn_confirm, (gui_event_cb_t)file_btn_confirm_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create file_btn_start (hg_label)
    file_btn_start = gui_text_create((gui_obj_t *)view, "file_btn_start", 208, 386, 178, 52);
    gui_text_set((gui_text_t *)file_btn_start, "Start", GUI_FONT_SRC_BMP, gui_rgb(85, 85, 85), 5, 20);
    gui_text_type_set((gui_text_t *)file_btn_start, "/font/Inter_24pt_Regular_size20_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)file_btn_start, MID_CENTER);
    gui_obj_add_event_cb(file_btn_start, (gui_event_cb_t)file_btn_start_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create file_upload_top_window (hg_window)
    file_upload_top_window = gui_win_create((gui_obj_t *)view, "file_upload_top_window", 0, 0, 410,
                                            100);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(file_upload_time_time_str, sizeof(file_upload_time_time_str), "%02d:%02d", t->tm_hour,
                 t->tm_min);
    }


    // Create file_upload_back_touch_win (hg_window)
    file_upload_back_touch_win = gui_win_create(file_upload_top_window, "file_upload_back_touch_win", 0,
                                                0, 100, 100);


    // Create file_upload_back_btn (hg_image)
    file_upload_back_btn = gui_img_create_from_fs(file_upload_back_touch_win, "file_upload_back_btn",
                                                  "/app_wifi_test/back_icon.bin", 34, 30, 28, 28);

    gui_obj_add_event_cb(GUI_BASE(file_upload_back_touch_win), file_upload_back,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create file_upload_time (hg_time_label)
    file_upload_time = gui_text_create(file_upload_top_window, "file_upload_time", 300, 20, 80, 32);
    gui_text_set((gui_text_t *)file_upload_time, file_upload_time_time_str, GUI_FONT_SRC_BMP,
                 gui_rgb(255, 255, 255), strlen(file_upload_time_time_str), 32);
    gui_text_type_set((gui_text_t *)file_upload_time,
                      "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)file_upload_time, RIGHT);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(file_upload_time), 30000, true, file_upload_time_time_update_cb);

    gui_obj_add_event_cb((gui_obj_t *)view, (gui_event_cb_t)app_wifi_testFileUploadView_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)view);
}
GUI_VIEW_INSTANCE("app_wifi_testFileUploadView", false, app_wifi_testFileUploadView_switch_in,
                  app_wifi_testFileUploadView_switch_out, false);

// Create app_wifi_testFileUploadRunningView (hg_view)
static void app_wifi_testFileUploadRunningView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_wifi_testFileUploadRunningView_switch_in(gui_view_t *view)
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



    // Create file_upload_ring_bg (hg_arc)
    file_upload_ring_bg = gui_arc_create((gui_obj_t *)view, "file_upload_ring_bg", 205, 250, 96, 0, 360,
                                         4, gui_rgba(255, 149, 0, 40));

    // Create file_upload_run_title (hg_label)
    file_upload_run_title = gui_scroll_text_create((gui_obj_t *)view, "file_upload_run_title", 0, 80,
                                                   410, 50);
    gui_scroll_text_set((gui_scroll_text_t *)file_upload_run_title, "File Upload Running",
                        GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 19, 40);
    gui_scroll_text_type_set((gui_scroll_text_t *)file_upload_run_title,
                             "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_scroll_text_scroll_set((gui_scroll_text_t *)file_upload_run_title, SCROLL_X_MID, 0, 0, 3000, 0);

    // Create file_upload_ring (hg_arc)
    file_upload_ring = gui_arc_create((gui_obj_t *)view, "file_upload_ring", 205, 250, 96, 0, 360, 4,
                                      gui_rgb(255, 149, 0));

    // Create file_upload_run_top_window (hg_window)
    file_upload_run_top_window = gui_win_create((gui_obj_t *)view, "file_upload_run_top_window", 0, 0,
                                                410, 100);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(file_upload_run_time_time_str, sizeof(file_upload_run_time_time_str), "%02d:%02d",
                 t->tm_hour, t->tm_min);
    }


    // Create file_upload_run_back_touch_win (hg_window)
    file_upload_run_back_touch_win = gui_win_create(file_upload_run_top_window,
                                                    "file_upload_run_back_touch_win", 0, 0, 100, 100);


    // Create file_upload_run_back_btn (hg_image)
    file_upload_run_back_btn = gui_img_create_from_fs(file_upload_run_back_touch_win,
                                                      "file_upload_run_back_btn", "/app_wifi_test/back_icon.bin", 34, 30, 28, 28);

    gui_obj_add_event_cb(GUI_BASE(file_upload_run_back_touch_win),
                         (gui_event_cb_t)file_upload_run_back_touch_win_clicked_0_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create file_upload_run_time (hg_time_label)
    file_upload_run_time = gui_text_create(file_upload_run_top_window, "file_upload_run_time", 300, 20,
                                           80, 32);
    gui_text_set((gui_text_t *)file_upload_run_time, file_upload_run_time_time_str, GUI_FONT_SRC_BMP,
                 gui_rgb(255, 255, 255), strlen(file_upload_run_time_time_str), 32);
    gui_text_type_set((gui_text_t *)file_upload_run_time,
                      "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)file_upload_run_time, RIGHT);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(file_upload_run_time), 30000, true,
                         file_upload_run_time_time_update_cb);

    // Create file_upload_data (hg_label)
    file_upload_data = gui_text_create((gui_obj_t *)view, "file_upload_data", 130, 215, 150, 50);
    gui_text_set((gui_text_t *)file_upload_data, "0", GUI_FONT_SRC_BMP, gui_rgb(255, 149, 0), 1, 40);
    gui_text_type_set((gui_text_t *)file_upload_data,
                      "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)file_upload_data, MID_CENTER);
    // Bind timer: data_tick
    gui_obj_create_timer((gui_obj_t *)file_upload_data, 500, true, file_upload_data_tick_cb);
    gui_obj_start_timer((gui_obj_t *)file_upload_data);

    // Create file_upload_unit (hg_label)
    file_upload_unit = gui_text_create((gui_obj_t *)view, "file_upload_unit", 130, 262, 150, 24);
    gui_text_set((gui_text_t *)file_upload_unit, "KByte", GUI_FONT_SRC_BMP, gui_rgb(153, 153, 153), 5,
                 18);
    gui_text_type_set((gui_text_t *)file_upload_unit,
                      "/font/Inter_24pt_Regular_size18_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)file_upload_unit, MID_CENTER);

    // Create file_upload_target (hg_label)
    file_upload_target = gui_text_create((gui_obj_t *)view, "file_upload_target", 80, 370, 250, 24);
    gui_text_set((gui_text_t *)file_upload_target, "Target: 0.0.0.0", GUI_FONT_SRC_BMP, gui_rgb(102,
                 102, 102), 15, 16);
    gui_text_type_set((gui_text_t *)file_upload_target,
                      "/font/Inter_24pt_Regular_size16_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)file_upload_target, MID_CENTER);

    gui_obj_add_event_cb((gui_obj_t *)view, (gui_event_cb_t)app_wifi_testFileUploadRunningView_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)view);
}
GUI_VIEW_INSTANCE("app_wifi_testFileUploadRunningView", false,
                  app_wifi_testFileUploadRunningView_switch_in, app_wifi_testFileUploadRunningView_switch_out, false);

// Create app_wifi_testFileDownloadView (hg_view)
static void app_wifi_testFileDownloadView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_wifi_testFileDownloadView_switch_in(gui_view_t *view)
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



    // Create file_download_soc_ip_bg (hg_rect)
    file_download_soc_ip_bg = gui_rect_create((gui_obj_t *)view, "file_download_soc_ip_bg", 24, 140,
                                              362, 84, 16, gui_rgba(255, 255, 255, 20));

    // Create file_dl_connect_timer_host (hg_label)
    file_dl_connect_timer_host = gui_text_create((gui_obj_t *)view, "file_dl_connect_timer_host", 0, 0,
                                                 1, 11);
    gui_text_set((gui_text_t *)file_dl_connect_timer_host, "", GUI_FONT_SRC_BMP, gui_rgb(0, 0, 0), 0,
                 1);
    gui_text_type_set((gui_text_t *)file_dl_connect_timer_host,
                      "/font/Inter_24pt_Regular_size1_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)file_dl_connect_timer_host, LEFT);
    gui_obj_hidden((gui_obj_t *)file_dl_connect_timer_host, true);

    // Create file_download_title (hg_label)
    file_download_title = gui_scroll_text_create((gui_obj_t *)view, "file_download_title", 0, 80, 410,
                                                 50);
    gui_scroll_text_set((gui_scroll_text_t *)file_download_title, "File Download", GUI_FONT_SRC_BMP,
                        gui_rgb(242, 242, 242), 13, 40);
    gui_scroll_text_type_set((gui_scroll_text_t *)file_download_title,
                             "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_scroll_text_scroll_set((gui_scroll_text_t *)file_download_title, SCROLL_X_MID, 0, 0, 3000, 0);

    // Create file_download_soc_ip (hg_label)
    file_download_soc_ip = gui_text_create((gui_obj_t *)view, "file_download_soc_ip", 24, 140, 362, 84);
    gui_text_set((gui_text_t *)file_download_soc_ip, "SoC IP: 192.168.1.100", GUI_FONT_SRC_BMP,
                 gui_rgb(242, 242, 242), 21, 40);
    gui_text_type_set((gui_text_t *)file_download_soc_ip,
                      "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)file_download_soc_ip, MID_CENTER);

    // Create file_dl_running_win (hg_window)
    file_dl_running_win = gui_win_create((gui_obj_t *)view, "file_dl_running_win", 0, 230, 410, 230);


    // Create file_dl_ring_bg (hg_arc)
    file_dl_ring_bg = gui_arc_create(file_dl_running_win, "file_dl_ring_bg", 205, 95, 86, 0, 360, 4,
                                     gui_rgba(88, 86, 214, 40));

    // Create file_dl_ring (hg_arc)
    file_dl_ring = gui_arc_create(file_dl_running_win, "file_dl_ring", 205, 95, 86, 0, 360, 4,
                                  gui_rgb(88, 86, 214));

    // Create file_dl_data (hg_label)
    file_dl_data = gui_text_create(file_dl_running_win, "file_dl_data", 130, 55, 150, 50);
    gui_text_set((gui_text_t *)file_dl_data, "0", GUI_FONT_SRC_BMP, gui_rgb(88, 86, 214), 1, 36);
    gui_text_type_set((gui_text_t *)file_dl_data, "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)file_dl_data, MID_CENTER);
    // Bind timer: data_tick
    gui_obj_create_timer((gui_obj_t *)file_dl_data, 500, true, file_dl_data_tick_cb);
    gui_obj_start_timer((gui_obj_t *)file_dl_data);

    // Create file_dl_unit (hg_label)
    file_dl_unit = gui_text_create(file_dl_running_win, "file_dl_unit", 130, 100, 150, 24);
    gui_text_set((gui_text_t *)file_dl_unit, "KByte", GUI_FONT_SRC_BMP, gui_rgb(153, 153, 153), 5, 16);
    gui_text_type_set((gui_text_t *)file_dl_unit, "/font/Inter_24pt_Regular_size16_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)file_dl_unit, MID_CENTER);

    // Create file_download_top_window (hg_window)
    file_download_top_window = gui_win_create((gui_obj_t *)view, "file_download_top_window", 0, 0, 410,
                                              100);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(file_download_time_time_str, sizeof(file_download_time_time_str), "%02d:%02d", t->tm_hour,
                 t->tm_min);
    }


    // Create file_download_back_touch_win (hg_window)
    file_download_back_touch_win = gui_win_create(file_download_top_window,
                                                  "file_download_back_touch_win", 0, 0, 100, 100);


    // Create file_download_back_btn (hg_image)
    file_download_back_btn = gui_img_create_from_fs(file_download_back_touch_win,
                                                    "file_download_back_btn", "/app_wifi_test/back_icon.bin", 34, 30, 28, 28);

    gui_obj_add_event_cb(GUI_BASE(file_download_back_touch_win), file_download_back,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create file_download_time (hg_time_label)
    file_download_time = gui_text_create(file_download_top_window, "file_download_time", 300, 20, 80,
                                         32);
    gui_text_set((gui_text_t *)file_download_time, file_download_time_time_str, GUI_FONT_SRC_BMP,
                 gui_rgb(255, 255, 255), strlen(file_download_time_time_str), 32);
    gui_text_type_set((gui_text_t *)file_download_time,
                      "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)file_download_time, RIGHT);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(file_download_time), 30000, true, file_download_time_time_update_cb);

    gui_obj_add_event_cb((gui_obj_t *)view, (gui_event_cb_t)app_wifi_testFileDownloadView_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)view);
}
GUI_VIEW_INSTANCE("app_wifi_testFileDownloadView", false, app_wifi_testFileDownloadView_switch_in,
                  app_wifi_testFileDownloadView_switch_out, false);

// Create app_wifi_testCustomMenuView (hg_view)
static void app_wifi_testCustomMenuView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_wifi_testCustomMenuView_switch_in(gui_view_t *view)
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



    // Create custom_menu_title (hg_label)
    custom_menu_title = gui_scroll_text_create((gui_obj_t *)view, "custom_menu_title", 24, 90, 362, 50);
    gui_scroll_text_set((gui_scroll_text_t *)custom_menu_title, "Custom Tests", GUI_FONT_SRC_BMP,
                        gui_rgb(242, 242, 242), 12, 40);
    gui_scroll_text_type_set((gui_scroll_text_t *)custom_menu_title,
                             "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_scroll_text_scroll_set((gui_scroll_text_t *)custom_menu_title, SCROLL_X_MID, 0, 0, 3000, 0);

    // Create custom_menu_list (hg_list)
    custom_menu_list = gui_list_create((gui_obj_t *)view, "custom_menu_list", 0, 140, 410, 282, 84, 10,
                                       VERTICAL, custom_menu_list_note_design, NULL, false);
    gui_list_set_style(custom_menu_list, LIST_CLASSIC);
    gui_list_set_note_num(custom_menu_list, 3);
    gui_list_set_out_scope(custom_menu_list, 80);
    gui_list_keep_note_alive(custom_menu_list, true);

    // Create custom_menu_top_window (hg_window)
    custom_menu_top_window = gui_win_create((gui_obj_t *)view, "custom_menu_top_window", 0, 0, 410,
                                            100);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(custom_menu_time_time_str, sizeof(custom_menu_time_time_str), "%02d:%02d", t->tm_hour,
                 t->tm_min);
    }


    // Create custom_menu_back_touch_win (hg_window)
    custom_menu_back_touch_win = gui_win_create(custom_menu_top_window, "custom_menu_back_touch_win", 0,
                                                0, 100, 100);


    // Create custom_menu_back_btn (hg_image)
    custom_menu_back_btn = gui_img_create_from_fs(custom_menu_back_touch_win, "custom_menu_back_btn",
                                                  "/app_wifi_test/back_icon.bin", 34, 30, 28, 28);

    gui_obj_add_event_cb(GUI_BASE(custom_menu_back_touch_win),
                         (gui_event_cb_t)custom_menu_back_touch_win_clicked_0_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create custom_menu_time (hg_time_label)
    custom_menu_time = gui_text_create(custom_menu_top_window, "custom_menu_time", 300, 20, 80, 32);
    gui_text_set((gui_text_t *)custom_menu_time, custom_menu_time_time_str, GUI_FONT_SRC_BMP,
                 gui_rgb(255, 255, 255), strlen(custom_menu_time_time_str), 32);
    gui_text_type_set((gui_text_t *)custom_menu_time,
                      "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)custom_menu_time, RIGHT);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(custom_menu_time), 30000, true, custom_menu_time_time_update_cb);

    gui_obj_add_event_cb((gui_obj_t *)view, (gui_event_cb_t)app_wifi_testCustomMenuView_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)view);
}
GUI_VIEW_INSTANCE("app_wifi_testCustomMenuView", false, app_wifi_testCustomMenuView_switch_in,
                  app_wifi_testCustomMenuView_switch_out, false);

// Create app_wifi_testCustom1View (hg_view)
static void app_wifi_testCustom1View_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_wifi_testCustom1View_switch_in(gui_view_t *view)
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



    // Create custom1_title (hg_label)
    custom1_title = gui_scroll_text_create((gui_obj_t *)view, "custom1_title", 24, 220, 362, 50);
    gui_scroll_text_set((gui_scroll_text_t *)custom1_title, "Custom Test Page 1", GUI_FONT_SRC_BMP,
                        gui_rgb(242, 242, 242), 18, 40);
    gui_scroll_text_type_set((gui_scroll_text_t *)custom1_title,
                             "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_scroll_text_scroll_set((gui_scroll_text_t *)custom1_title, SCROLL_X_MID, 0, 0, 3000, 0);

    // Create custom1_top_window (hg_window)
    custom1_top_window = gui_win_create((gui_obj_t *)view, "custom1_top_window", 0, 0, 410, 100);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(custom1_time_time_str, sizeof(custom1_time_time_str), "%02d:%02d", t->tm_hour, t->tm_min);
    }


    // Create custom1_back_touch_win (hg_window)
    custom1_back_touch_win = gui_win_create(custom1_top_window, "custom1_back_touch_win", 0, 0, 100,
                                            100);


    // Create custom1_back_btn (hg_image)
    custom1_back_btn = gui_img_create_from_fs(custom1_back_touch_win, "custom1_back_btn",
                                              "/app_wifi_test/back_icon.bin", 34, 30, 28, 28);

    gui_obj_add_event_cb(GUI_BASE(custom1_back_touch_win),
                         (gui_event_cb_t)custom1_back_touch_win_clicked_0_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create custom1_time (hg_time_label)
    custom1_time = gui_text_create(custom1_top_window, "custom1_time", 300, 20, 80, 32);
    gui_text_set((gui_text_t *)custom1_time, custom1_time_time_str, GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                 255), strlen(custom1_time_time_str), 32);
    gui_text_type_set((gui_text_t *)custom1_time, "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)custom1_time, RIGHT);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(custom1_time), 30000, true, custom1_time_time_update_cb);

    gui_obj_add_event_cb((gui_obj_t *)view, (gui_event_cb_t)app_wifi_testCustom1View_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)view);
}
GUI_VIEW_INSTANCE("app_wifi_testCustom1View", false, app_wifi_testCustom1View_switch_in,
                  app_wifi_testCustom1View_switch_out, false);

// Create app_wifi_testCustom2View (hg_view)
static void app_wifi_testCustom2View_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_wifi_testCustom2View_switch_in(gui_view_t *view)
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



    // Create custom2_title (hg_label)
    custom2_title = gui_scroll_text_create((gui_obj_t *)view, "custom2_title", 24, 220, 362, 50);
    gui_scroll_text_set((gui_scroll_text_t *)custom2_title, "Custom Test Page 2", GUI_FONT_SRC_BMP,
                        gui_rgb(242, 242, 242), 18, 40);
    gui_scroll_text_type_set((gui_scroll_text_t *)custom2_title,
                             "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_scroll_text_scroll_set((gui_scroll_text_t *)custom2_title, SCROLL_X_MID, 0, 0, 3000, 0);

    // Create custom2_top_window (hg_window)
    custom2_top_window = gui_win_create((gui_obj_t *)view, "custom2_top_window", 0, 0, 410, 100);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(custom2_time_time_str, sizeof(custom2_time_time_str), "%02d:%02d", t->tm_hour, t->tm_min);
    }


    // Create custom2_back_touch_win (hg_window)
    custom2_back_touch_win = gui_win_create(custom2_top_window, "custom2_back_touch_win", 0, 0, 100,
                                            100);


    // Create custom2_back_btn (hg_image)
    custom2_back_btn = gui_img_create_from_fs(custom2_back_touch_win, "custom2_back_btn",
                                              "/app_wifi_test/back_icon.bin", 34, 30, 28, 28);

    gui_obj_add_event_cb(GUI_BASE(custom2_back_touch_win),
                         (gui_event_cb_t)custom2_back_touch_win_clicked_0_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create custom2_time (hg_time_label)
    custom2_time = gui_text_create(custom2_top_window, "custom2_time", 300, 20, 80, 32);
    gui_text_set((gui_text_t *)custom2_time, custom2_time_time_str, GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                 255), strlen(custom2_time_time_str), 32);
    gui_text_type_set((gui_text_t *)custom2_time, "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)custom2_time, RIGHT);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(custom2_time), 30000, true, custom2_time_time_update_cb);

    gui_obj_add_event_cb((gui_obj_t *)view, (gui_event_cb_t)app_wifi_testCustom2View_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)view);
}
GUI_VIEW_INSTANCE("app_wifi_testCustom2View", false, app_wifi_testCustom2View_switch_in,
                  app_wifi_testCustom2View_switch_out, false);

// Create app_wifi_testCustom3View (hg_view)
static void app_wifi_testCustom3View_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_wifi_testCustom3View_switch_in(gui_view_t *view)
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



    // Create custom3_title (hg_label)
    custom3_title = gui_scroll_text_create((gui_obj_t *)view, "custom3_title", 24, 220, 362, 50);
    gui_scroll_text_set((gui_scroll_text_t *)custom3_title, "Custom Test Page 3", GUI_FONT_SRC_BMP,
                        gui_rgb(242, 242, 242), 18, 40);
    gui_scroll_text_type_set((gui_scroll_text_t *)custom3_title,
                             "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_scroll_text_scroll_set((gui_scroll_text_t *)custom3_title, SCROLL_X_MID, 0, 0, 3000, 0);

    // Create custom3_top_window (hg_window)
    custom3_top_window = gui_win_create((gui_obj_t *)view, "custom3_top_window", 0, 0, 410, 100);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(custom3_time_time_str, sizeof(custom3_time_time_str), "%02d:%02d", t->tm_hour, t->tm_min);
    }


    // Create custom3_back_touch_win (hg_window)
    custom3_back_touch_win = gui_win_create(custom3_top_window, "custom3_back_touch_win", 0, 0, 100,
                                            100);


    // Create custom3_back_btn (hg_image)
    custom3_back_btn = gui_img_create_from_fs(custom3_back_touch_win, "custom3_back_btn",
                                              "/app_wifi_test/back_icon.bin", 34, 30, 28, 28);

    gui_obj_add_event_cb(GUI_BASE(custom3_back_touch_win),
                         (gui_event_cb_t)custom3_back_touch_win_clicked_0_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create custom3_time (hg_time_label)
    custom3_time = gui_text_create(custom3_top_window, "custom3_time", 300, 20, 80, 32);
    gui_text_set((gui_text_t *)custom3_time, custom3_time_time_str, GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                 255), strlen(custom3_time_time_str), 32);
    gui_text_type_set((gui_text_t *)custom3_time, "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)custom3_time, RIGHT);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(custom3_time), 30000, true, custom3_time_time_update_cb);

    gui_obj_add_event_cb((gui_obj_t *)view, (gui_event_cb_t)app_wifi_testCustom3View_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)view);
}
GUI_VIEW_INSTANCE("app_wifi_testCustom3View", false, app_wifi_testCustom3View_switch_in,
                  app_wifi_testCustom3View_switch_out, false);
