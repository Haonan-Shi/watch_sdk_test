/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_intercom UI Definition (Auto-generated, do not modify manually)
 * Generated at: 2026-04-13T05:54:55.808Z
 */
#ifndef APP_INTERCOM_UI_H
#define APP_INTERCOM_UI_H

#include "guidef.h"
#include "gui_obj.h"
#include "gui_components_init.h"
#include "gui_view.h"
#include "gui_view_instance.h"
#include "gui_win.h"
#include "draw_font.h"
#include "font_types.h"
#include "gui_text.h"
#include "gui_list.h"
#include "gui_rect.h"
#include "gui_img.h"

// Component handle declarations
extern gui_text_t *available_devices_label;
extern gui_list_t *toggle_list;
extern gui_rounded_rect_t *device1_item_bg;
extern gui_text_t *device1_name_label;
extern gui_img_t *device1_status_dot;
extern gui_rounded_rect_t *device2_item_bg;
extern gui_text_t *device2_name_label;
extern gui_img_t *device2_status_dot;
extern gui_rounded_rect_t *device3_item_bg;
extern gui_text_t *device3_name_label;
extern gui_img_t *device3_status_dot;
extern gui_win_t *toggle_window;
extern gui_win_t *intercom_main_back_win;
extern gui_text_t *toggle_time_label;
extern gui_text_t *toggle_page_title;
extern gui_img_t *toggle_back_btn;
extern gui_obj_t *intercom_toggle_btn;
extern gui_win_t *talk_window;
extern gui_win_t *intercom_talk_back_win;
extern gui_text_t *talk_time_label;
extern gui_img_t *connection_dot;
extern gui_text_t *connection_label;
extern gui_text_t *intercom_device_name_label;
extern gui_img_t *waveform_image;
extern gui_text_t *status_text_label;
extern gui_img_t *talk_btn;
extern gui_obj_t *mute_btn;
extern gui_img_t *talk_back_btn;

// Toggle button state management function declarations
extern bool intercom_toggle_btn_get_state(void);
extern void intercom_toggle_btn_set_state(bool state);
extern bool mute_btn_get_state(void);
extern void mute_btn_set_state(bool state);

#endif // APP_INTERCOM_UI_H
