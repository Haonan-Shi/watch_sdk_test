/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_video_call UI Definition (Auto-generated, do not modify manually)
 * Generated at: 2026-04-14T08:17:49.455Z
 */
#ifndef APP_VIDEO_CALL_UI_H
#define APP_VIDEO_CALL_UI_H

#include "guidef.h"
#include "gui_obj.h"
#include "gui_components_init.h"
#include "gui_view.h"
#include "gui_view_instance.h"
#include "gui_win.h"
#include "draw_font.h"
#include "font_types.h"
#include "gui_img.h"
#include "gui_text.h"

// Component handle declarations
extern gui_img_t *video_call_idle_avatar;
extern gui_text_t *video_call_idle_name;
extern gui_text_t *video_call_idle_subtitle;
extern gui_img_t *video_call_btn;
extern gui_win_t *video_call_idle_window;
extern gui_img_t *video_call_idle_back_icon;
extern gui_text_t *video_call_idle_time_label;
extern gui_win_t *win_video_call_idle_back;
extern gui_img_t *video_call_ring_pulse_img;
extern gui_img_t *video_call_calling_avatar;
extern gui_text_t *video_call_calling_name;
extern gui_text_t *video_call_calling_status;
extern gui_obj_t *mic_btn;
extern gui_img_t *video_call_hangup_btn;
extern gui_obj_t *speaker_btn;
extern gui_win_t *video_call_calling_window;
extern gui_img_t *video_call_calling_back_icon;
extern gui_text_t *video_call_calling_time_label;
extern gui_win_t *win_video_calling_back;

// Toggle button state management function declarations
extern bool mic_btn_get_state(void);
extern void mic_btn_set_state(bool state);
extern bool speaker_btn_get_state(void);
extern void speaker_btn_set_state(bool state);

#endif // APP_VIDEO_CALL_UI_H
