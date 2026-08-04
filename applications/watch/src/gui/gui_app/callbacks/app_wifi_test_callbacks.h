/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_WIFI_TEST_CALLBACKS_H
#define APP_WIFI_TEST_CALLBACKS_H

#include "gui_api.h"
#include "gui_text.h"
#include "gui_obj_focus.h"

// Timer animation counters (defined in callbacks.c)
extern uint16_t iperf_upload_speed_timer_cnt;
extern uint16_t iperf_dl_speed_timer_cnt;
extern uint16_t file_upload_data_timer_cnt;
extern uint16_t file_dl_data_timer_cnt;
extern uint16_t file_dl_connect_timer_host_timer_cnt;

// Event callback function declarations
void app_wifi_testCustom1View_key_0_cb(void *obj, gui_event_t *e);
void app_wifi_testCustom2View_key_0_cb(void *obj, gui_event_t *e);
void app_wifi_testCustom3View_key_0_cb(void *obj, gui_event_t *e);
void app_wifi_testCustomMenuView_key_0_cb(void *obj, gui_event_t *e);
void app_wifi_testFileDownloadView_key_0_cb(void *obj, gui_event_t *e);
void app_wifi_testFileMenuView_key_0_cb(void *obj, gui_event_t *e);
void app_wifi_testFileUploadRunningView_key_0_cb(void *obj, gui_event_t *e);
void app_wifi_testFileUploadView_key_0_cb(void *obj, gui_event_t *e);
void app_wifi_testIperfDownloadView_key_0_cb(void *obj, gui_event_t *e);
void app_wifi_testIperfMenuView_key_0_cb(void *obj, gui_event_t *e);
void app_wifi_testIperfUploadRunningView_key_0_cb(void *obj, gui_event_t *e);
void app_wifi_testIperfUploadView_key_0_cb(void *obj, gui_event_t *e);
void app_wifi_testWifiHomeView_key_0_cb(void *obj, gui_event_t *e);
void custom1_back_touch_win_clicked_0_cb(void *obj, gui_event_t *e);
void custom2_back_touch_win_clicked_0_cb(void *obj, gui_event_t *e);
void custom3_back_touch_win_clicked_0_cb(void *obj, gui_event_t *e);
void custom_menu_back_touch_win_clicked_0_cb(void *obj, gui_event_t *e);
void custom_menu_test1_btn_switch_view_cb(void *obj, gui_event_t *e);
void custom_menu_test2_btn_switch_view_cb(void *obj, gui_event_t *e);
void custom_menu_test3_btn_switch_view_cb(void *obj, gui_event_t *e);
void file_btn_confirm_bg_clicked_cb(void *obj, gui_event_t *e);
void file_btn_confirm_clicked_cb(void *obj, gui_event_t *e);
void file_btn_start_bg_clicked_cb(void *obj, gui_event_t *e);
void file_btn_start_clicked_cb(void *obj, gui_event_t *e);
void file_download_back(void *obj, gui_event_t *e);
void file_key_0_bg_clicked_cb(void *obj, gui_event_t *e);
void file_key_0_clicked_cb(void *obj, gui_event_t *e);
void file_key_1_bg_clicked_cb(void *obj, gui_event_t *e);
void file_key_1_clicked_cb(void *obj, gui_event_t *e);
void file_key_2_bg_clicked_cb(void *obj, gui_event_t *e);
void file_key_2_clicked_cb(void *obj, gui_event_t *e);
void file_key_3_bg_clicked_cb(void *obj, gui_event_t *e);
void file_key_3_clicked_cb(void *obj, gui_event_t *e);
void file_key_4_bg_clicked_cb(void *obj, gui_event_t *e);
void file_key_4_clicked_cb(void *obj, gui_event_t *e);
void file_key_5_bg_clicked_cb(void *obj, gui_event_t *e);
void file_key_5_clicked_cb(void *obj, gui_event_t *e);
void file_key_6_bg_clicked_cb(void *obj, gui_event_t *e);
void file_key_6_clicked_cb(void *obj, gui_event_t *e);
void file_key_7_bg_clicked_cb(void *obj, gui_event_t *e);
void file_key_7_clicked_cb(void *obj, gui_event_t *e);
void file_key_8_bg_clicked_cb(void *obj, gui_event_t *e);
void file_key_8_clicked_cb(void *obj, gui_event_t *e);
void file_key_9_bg_clicked_cb(void *obj, gui_event_t *e);
void file_key_9_clicked_cb(void *obj, gui_event_t *e);
void file_key_del_bg_clicked_cb(void *obj, gui_event_t *e);
void file_key_del_clicked_cb(void *obj, gui_event_t *e);
void file_key_dot_bg_clicked_cb(void *obj, gui_event_t *e);
void file_key_dot_clicked_cb(void *obj, gui_event_t *e);
void file_menu_back_touch_win_clicked_0_cb(void *obj, gui_event_t *e);
void file_menu_download_bg_clicked_cb(void *obj, gui_event_t *e);
void file_menu_upload_btn_switch_view_cb(void *obj, gui_event_t *e);
void file_upload_back(void *obj, gui_event_t *e);
void file_upload_run_back_touch_win_clicked_0_cb(void *obj, gui_event_t *e);
void iperf_btn_confirm_bg_clicked_cb(void *obj, gui_event_t *e);
void iperf_btn_confirm_clicked_cb(void *obj, gui_event_t *e);
void iperf_btn_start_bg_clicked_cb(void *obj, gui_event_t *e);
void iperf_btn_start_clicked_cb(void *obj, gui_event_t *e);
void iperf_download_back(void *obj, gui_event_t *e);
void iperf_key_0_bg_clicked_cb(void *obj, gui_event_t *e);
void iperf_key_0_clicked_cb(void *obj, gui_event_t *e);
void iperf_key_1_bg_clicked_cb(void *obj, gui_event_t *e);
void iperf_key_1_clicked_cb(void *obj, gui_event_t *e);
void iperf_key_2_bg_clicked_cb(void *obj, gui_event_t *e);
void iperf_key_2_clicked_cb(void *obj, gui_event_t *e);
void iperf_key_3_bg_clicked_cb(void *obj, gui_event_t *e);
void iperf_key_3_clicked_cb(void *obj, gui_event_t *e);
void iperf_key_4_bg_clicked_cb(void *obj, gui_event_t *e);
void iperf_key_4_clicked_cb(void *obj, gui_event_t *e);
void iperf_key_5_bg_clicked_cb(void *obj, gui_event_t *e);
void iperf_key_5_clicked_cb(void *obj, gui_event_t *e);
void iperf_key_6_bg_clicked_cb(void *obj, gui_event_t *e);
void iperf_key_6_clicked_cb(void *obj, gui_event_t *e);
void iperf_key_7_bg_clicked_cb(void *obj, gui_event_t *e);
void iperf_key_7_clicked_cb(void *obj, gui_event_t *e);
void iperf_key_8_bg_clicked_cb(void *obj, gui_event_t *e);
void iperf_key_8_clicked_cb(void *obj, gui_event_t *e);
void iperf_key_9_bg_clicked_cb(void *obj, gui_event_t *e);
void iperf_key_9_clicked_cb(void *obj, gui_event_t *e);
void iperf_key_del_bg_clicked_cb(void *obj, gui_event_t *e);
void iperf_key_del_clicked_cb(void *obj, gui_event_t *e);
void iperf_key_dot_bg_clicked_cb(void *obj, gui_event_t *e);
void iperf_key_dot_clicked_cb(void *obj, gui_event_t *e);
void iperf_menu_back_touch_win_clicked_0_cb(void *obj, gui_event_t *e);
void iperf_menu_download_bg_clicked_cb(void *obj, gui_event_t *e);
void iperf_menu_upload_btn_switch_view_cb(void *obj, gui_event_t *e);
void iperf_upload_back(void *obj, gui_event_t *e);
void iperf_upload_run_back_touch_win_clicked_0_cb(void *obj, gui_event_t *e);
void wifi_home_back_touch_win_clicked_0_cb(void *obj, gui_event_t *e);
void wifi_home_custom_btn_switch_view_cb(void *obj, gui_event_t *e);
void wifi_home_file_btn_switch_view_cb(void *obj, gui_event_t *e);
void wifi_home_iperf_btn_switch_view_cb(void *obj, gui_event_t *e);
void wifi_home_time_time_update_cb(void *p);
void iperf_menu_time_time_update_cb(void *p);
void iperf_upload_time_time_update_cb(void *p);
void iperf_upload_run_time_time_update_cb(void *p);
void iperf_download_time_time_update_cb(void *p);
void file_menu_time_time_update_cb(void *p);
void file_upload_time_time_update_cb(void *p);
void file_upload_run_time_time_update_cb(void *p);
void file_download_time_time_update_cb(void *p);
void custom_menu_time_time_update_cb(void *p);
void custom1_time_time_update_cb(void *p);
void custom2_time_time_update_cb(void *p);
void custom3_time_time_update_cb(void *p);

// User-configured timer callback function declarations
void iperf_upload_speed_tick_cb(void *obj);
void iperf_dl_speed_tick_cb(void *obj);
void file_upload_data_tick_cb(void *obj);
void file_dl_data_tick_cb(void *obj);
void file_dl_connect_delay_cb(void *obj);

#endif // APP_WIFI_TEST_CALLBACKS_H
