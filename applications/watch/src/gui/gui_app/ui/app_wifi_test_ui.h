/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_wifi_test UI Definition (Auto-generated, do not modify manually)
 * Generated at: 2026-04-17T04:56:37.068Z
 */
#ifndef APP_WIFI_TEST_UI_H
#define APP_WIFI_TEST_UI_H

#include "guidef.h"
#include "gui_obj.h"
#include "gui_components_init.h"
#include "gui_view.h"
#include "gui_view_instance.h"
#include "gui_win.h"
#include "draw_font.h"
#include "font_types.h"
#include "gui_scroll_text.h"
#include "gui_img.h"
#include "gui_text.h"
#include "gui_list.h"
#include "gui_rect.h"
#include "gui_arc.h"

// Component handle declarations
extern gui_win_t *wifi_home_top_window;
extern gui_win_t *wifi_home_back_touch_win;
extern gui_img_t *wifi_home_back_btn;
extern gui_text_t *wifi_home_time;
extern gui_scroll_text_t *wifi_home_title;
extern gui_list_t *wifi_home_list;
extern gui_rounded_rect_t *wifi_home_status_bg;
extern gui_img_t *wifi_home_wifi_icon;
extern gui_text_t *wifi_home_status_text;
extern gui_scroll_text_t *wifi_home_ip_text;
extern gui_rounded_rect_t *wifi_home_iperf_bg;
extern gui_img_t *wifi_home_iperf_icon;
extern gui_scroll_text_t *wifi_home_iperf_label;
extern gui_rounded_rect_t *wifi_home_file_bg;
extern gui_img_t *wifi_home_file_icon;
extern gui_scroll_text_t *wifi_home_file_label;
extern gui_rounded_rect_t *wifi_home_custom_bg;
extern gui_img_t *wifi_home_custom_icon;
extern gui_scroll_text_t *wifi_home_custom_label;
extern gui_win_t *iperf_menu_top_window;
extern gui_win_t *iperf_menu_back_touch_win;
extern gui_img_t *iperf_menu_back_btn;
extern gui_text_t *iperf_menu_time;
extern gui_scroll_text_t *iperf_menu_title;
extern gui_list_t *iperf_menu_list;
extern gui_rounded_rect_t *iperf_menu_upload_bg;
extern gui_img_t *iperf_menu_upload_icon;
extern gui_scroll_text_t *iperf_menu_upload_label;
extern gui_rounded_rect_t *iperf_menu_download_bg;
extern gui_img_t *iperf_menu_download_icon;
extern gui_scroll_text_t *iperf_menu_download_label;
extern gui_win_t *iperf_upload_top_window;
extern gui_win_t *iperf_upload_back_touch_win;
extern gui_img_t *iperf_upload_back_btn;
extern gui_text_t *iperf_upload_time;
extern gui_scroll_text_t *iperf_upload_title;
extern gui_rounded_rect_t *iperf_ip_display_bg;
extern gui_rounded_rect_t *iperf_ip_seg0_bg;
extern gui_rounded_rect_t *iperf_ip_seg1_bg;
extern gui_rounded_rect_t *iperf_ip_seg2_bg;
extern gui_rounded_rect_t *iperf_ip_seg3_bg;
extern gui_text_t *iperf_ip_seg0;
extern gui_text_t *iperf_ip_dot0;
extern gui_text_t *iperf_ip_seg1;
extern gui_text_t *iperf_ip_dot1;
extern gui_text_t *iperf_ip_seg2;
extern gui_text_t *iperf_ip_dot2;
extern gui_text_t *iperf_ip_seg3;
extern gui_text_t *iperf_ip_confirmed;
extern gui_rounded_rect_t *iperf_key_1_bg;
extern gui_text_t *iperf_key_1;
extern gui_rounded_rect_t *iperf_key_2_bg;
extern gui_text_t *iperf_key_2;
extern gui_rounded_rect_t *iperf_key_3_bg;
extern gui_text_t *iperf_key_3;
extern gui_rounded_rect_t *iperf_key_4_bg;
extern gui_text_t *iperf_key_4;
extern gui_rounded_rect_t *iperf_key_5_bg;
extern gui_text_t *iperf_key_5;
extern gui_rounded_rect_t *iperf_key_6_bg;
extern gui_text_t *iperf_key_6;
extern gui_rounded_rect_t *iperf_key_7_bg;
extern gui_text_t *iperf_key_7;
extern gui_rounded_rect_t *iperf_key_8_bg;
extern gui_text_t *iperf_key_8;
extern gui_rounded_rect_t *iperf_key_9_bg;
extern gui_text_t *iperf_key_9;
extern gui_rounded_rect_t *iperf_key_dot_bg;
extern gui_text_t *iperf_key_dot;
extern gui_rounded_rect_t *iperf_key_0_bg;
extern gui_text_t *iperf_key_0;
extern gui_rounded_rect_t *iperf_key_del_bg;
extern gui_text_t *iperf_key_del;
extern gui_rounded_rect_t *iperf_btn_confirm_bg;
extern gui_text_t *iperf_btn_confirm;
extern gui_rounded_rect_t *iperf_btn_start_bg;
extern gui_text_t *iperf_btn_start;
extern gui_win_t *iperf_upload_run_top_window;
extern gui_win_t *iperf_upload_run_back_touch_win;
extern gui_img_t *iperf_upload_run_back_btn;
extern gui_text_t *iperf_upload_run_time;
extern gui_scroll_text_t *iperf_upload_run_title;
extern gui_arc_t *iperf_upload_ring_bg;
extern gui_arc_t *iperf_upload_ring;
extern gui_text_t *iperf_upload_speed;
extern gui_text_t *iperf_upload_unit;
extern gui_text_t *iperf_upload_target;
extern gui_win_t *iperf_download_top_window;
extern gui_win_t *iperf_download_back_touch_win;
extern gui_img_t *iperf_download_back_btn;
extern gui_text_t *iperf_download_time;
extern gui_scroll_text_t *iperf_download_title;
extern gui_rounded_rect_t *iperf_download_soc_ip_bg;
extern gui_text_t *iperf_download_soc_ip;
extern gui_win_t *iperf_dl_running_win;
extern gui_arc_t *iperf_dl_ring_bg;
extern gui_arc_t *iperf_dl_ring;
extern gui_text_t *iperf_dl_speed;
extern gui_text_t *iperf_dl_unit;
extern gui_win_t *file_menu_top_window;
extern gui_win_t *file_menu_back_touch_win;
extern gui_img_t *file_menu_back_btn;
extern gui_text_t *file_menu_time;
extern gui_scroll_text_t *file_menu_title;
extern gui_list_t *file_menu_list;
extern gui_rounded_rect_t *file_menu_upload_bg;
extern gui_img_t *file_menu_upload_icon;
extern gui_scroll_text_t *file_menu_upload_label;
extern gui_rounded_rect_t *file_menu_download_bg;
extern gui_img_t *file_menu_download_icon;
extern gui_scroll_text_t *file_menu_download_label;
extern gui_win_t *file_upload_top_window;
extern gui_win_t *file_upload_back_touch_win;
extern gui_img_t *file_upload_back_btn;
extern gui_text_t *file_upload_time;
extern gui_scroll_text_t *file_upload_title;
extern gui_rounded_rect_t *file_ip_display_bg;
extern gui_rounded_rect_t *file_ip_seg0_bg;
extern gui_rounded_rect_t *file_ip_seg1_bg;
extern gui_rounded_rect_t *file_ip_seg2_bg;
extern gui_rounded_rect_t *file_ip_seg3_bg;
extern gui_text_t *file_ip_seg0;
extern gui_text_t *file_ip_dot0;
extern gui_text_t *file_ip_seg1;
extern gui_text_t *file_ip_dot1;
extern gui_text_t *file_ip_seg2;
extern gui_text_t *file_ip_dot2;
extern gui_text_t *file_ip_seg3;
extern gui_text_t *file_ip_confirmed;
extern gui_rounded_rect_t *file_key_1_bg;
extern gui_text_t *file_key_1;
extern gui_rounded_rect_t *file_key_2_bg;
extern gui_text_t *file_key_2;
extern gui_rounded_rect_t *file_key_3_bg;
extern gui_text_t *file_key_3;
extern gui_rounded_rect_t *file_key_4_bg;
extern gui_text_t *file_key_4;
extern gui_rounded_rect_t *file_key_5_bg;
extern gui_text_t *file_key_5;
extern gui_rounded_rect_t *file_key_6_bg;
extern gui_text_t *file_key_6;
extern gui_rounded_rect_t *file_key_7_bg;
extern gui_text_t *file_key_7;
extern gui_rounded_rect_t *file_key_8_bg;
extern gui_text_t *file_key_8;
extern gui_rounded_rect_t *file_key_9_bg;
extern gui_text_t *file_key_9;
extern gui_rounded_rect_t *file_key_dot_bg;
extern gui_text_t *file_key_dot;
extern gui_rounded_rect_t *file_key_0_bg;
extern gui_text_t *file_key_0;
extern gui_rounded_rect_t *file_key_del_bg;
extern gui_text_t *file_key_del;
extern gui_rounded_rect_t *file_btn_confirm_bg;
extern gui_text_t *file_btn_confirm;
extern gui_rounded_rect_t *file_btn_start_bg;
extern gui_text_t *file_btn_start;
extern gui_win_t *file_upload_run_top_window;
extern gui_win_t *file_upload_run_back_touch_win;
extern gui_img_t *file_upload_run_back_btn;
extern gui_text_t *file_upload_run_time;
extern gui_scroll_text_t *file_upload_run_title;
extern gui_arc_t *file_upload_ring_bg;
extern gui_arc_t *file_upload_ring;
extern gui_text_t *file_upload_data;
extern gui_text_t *file_upload_unit;
extern gui_text_t *file_upload_target;
extern gui_win_t *file_download_top_window;
extern gui_win_t *file_download_back_touch_win;
extern gui_img_t *file_download_back_btn;
extern gui_text_t *file_download_time;
extern gui_scroll_text_t *file_download_title;
extern gui_rounded_rect_t *file_download_soc_ip_bg;
extern gui_text_t *file_download_soc_ip;
extern gui_win_t *file_dl_running_win;
extern gui_arc_t *file_dl_ring_bg;
extern gui_arc_t *file_dl_ring;
extern gui_text_t *file_dl_data;
extern gui_text_t *file_dl_unit;
extern gui_text_t *file_dl_connect_timer_host;
extern gui_win_t *custom_menu_top_window;
extern gui_win_t *custom_menu_back_touch_win;
extern gui_img_t *custom_menu_back_btn;
extern gui_text_t *custom_menu_time;
extern gui_scroll_text_t *custom_menu_title;
extern gui_list_t *custom_menu_list;
extern gui_rounded_rect_t *custom_menu_test1_bg;
extern gui_img_t *custom_menu_test1_icon;
extern gui_scroll_text_t *custom_menu_test1_label;
extern gui_rounded_rect_t *custom_menu_test2_bg;
extern gui_img_t *custom_menu_test2_icon;
extern gui_scroll_text_t *custom_menu_test2_label;
extern gui_rounded_rect_t *custom_menu_test3_bg;
extern gui_img_t *custom_menu_test3_icon;
extern gui_scroll_text_t *custom_menu_test3_label;
extern gui_win_t *custom1_top_window;
extern gui_win_t *custom1_back_touch_win;
extern gui_img_t *custom1_back_btn;
extern gui_text_t *custom1_time;
extern gui_scroll_text_t *custom1_title;
extern gui_win_t *custom2_top_window;
extern gui_win_t *custom2_back_touch_win;
extern gui_img_t *custom2_back_btn;
extern gui_text_t *custom2_time;
extern gui_scroll_text_t *custom2_title;
extern gui_win_t *custom3_top_window;
extern gui_win_t *custom3_back_touch_win;
extern gui_img_t *custom3_back_btn;
extern gui_text_t *custom3_time;
extern gui_scroll_text_t *custom3_title;

#endif // APP_WIFI_TEST_UI_H
