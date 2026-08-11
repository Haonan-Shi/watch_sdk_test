/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_WIFI_TEST_USER_H
#define APP_WIFI_TEST_USER_H

#include "../callbacks/app_wifi_test_callbacks.h"
#include "../ui/app_wifi_test_ui.h"

void wifi_home_view_init(void *obj);
void iperf_upload_view_init(void *obj);
void iperf_upload_running_view_init(void *obj);
void iperf_download_view_init(void *obj);
void file_upload_view_init(void *obj);
void file_upload_running_view_init(void *obj);
void file_download_view_init(void *obj);

void iperf_key_num(void *obj, gui_event_t *e);
void iperf_key_dot_press(void *obj, gui_event_t *e);
void iperf_key_delete(void *obj, gui_event_t *e);
void iperf_key_confirm(void *obj, gui_event_t *e);
void iperf_upload_start(void *obj, gui_event_t *e);
void iperf_upload_back(void *obj, gui_event_t *e);
void iperf_download_start(void *obj, gui_event_t *e);
void iperf_download_back(void *obj, gui_event_t *e);

void file_key_num(void *obj, gui_event_t *e);
void file_key_dot_press(void *obj, gui_event_t *e);
void file_key_delete(void *obj, gui_event_t *e);
void file_key_confirm(void *obj, gui_event_t *e);
void file_upload_start(void *obj, gui_event_t *e);
void file_upload_back(void *obj, gui_event_t *e);
void file_download_start(void *obj, gui_event_t *e);
void file_download_back(void *obj, gui_event_t *e);

void iperf_upload_speed_tick_cb_impl(void);
void iperf_dl_speed_tick_cb_impl(void);
void iperf_dl_connect_delay_cb_impl(void);
void file_upload_data_tick_cb_impl(void);
void file_dl_data_tick_cb_impl(void);
void file_dl_connect_delay_cb_impl(void);

#endif // APP_WIFI_TEST_USER_H