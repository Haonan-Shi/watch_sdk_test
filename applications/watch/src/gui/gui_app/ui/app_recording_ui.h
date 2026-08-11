/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_recording UI Definition (Auto-generated, do not modify manually)
 * Generated at: 2026-04-09T08:25:25.993Z
 */
#ifndef APP_RECORDING_UI_H
#define APP_RECORDING_UI_H

#include "guidef.h"
#include "gui_obj.h"
#include "gui_components_init.h"
#include "gui_view.h"
#include "gui_view_instance.h"
#include "gui_win.h"
#include "draw_font.h"
#include "font_types.h"
#include "gui_text.h"
#include "gui_img.h"
#include "gui_list.h"
#include "gui_rect.h"
#include "gui_arc.h"

// Component handle declarations
extern gui_text_t *recording_timer_label;
extern gui_img_t *recording_waveform_image;
extern gui_text_t *recording_status_label;
extern gui_obj_t *recording_record_btn;
extern gui_img_t *recording_files_btn;
extern gui_win_t *recording_main_window;
extern gui_text_t *recording_main_time_label;
extern gui_win_t *win_7;
extern gui_img_t *recording_main_back_btn;
extern gui_list_t *recording_files_list;
extern gui_rounded_rect_t *recording_file_bg_0;
extern gui_text_t *recording_file_name_0;
extern gui_text_t *recording_file_duration_0;
extern gui_img_t *recording_file_chevron_0;
extern gui_rounded_rect_t *recording_file_bg_1;
extern gui_text_t *recording_file_name_1;
extern gui_text_t *recording_file_duration_1;
extern gui_img_t *recording_file_chevron_1;
extern gui_rounded_rect_t *recording_file_bg_2;
extern gui_text_t *recording_file_name_2;
extern gui_text_t *recording_file_duration_2;
extern gui_img_t *recording_file_chevron_2;
extern gui_rounded_rect_t *recording_file_bg_3;
extern gui_text_t *recording_file_name_3;
extern gui_text_t *recording_file_duration_3;
extern gui_img_t *recording_file_chevron_3;
extern gui_rounded_rect_t *recording_file_bg_4;
extern gui_text_t *recording_file_name_4;
extern gui_text_t *recording_file_duration_4;
extern gui_img_t *recording_file_chevron_4;
extern gui_text_t *recording_files_empty_label;
extern gui_win_t *recording_files_window;
extern gui_text_t *recording_files_time_label;
extern gui_text_t *recording_files_title_label;
extern gui_win_t *win_8;
extern gui_img_t *recording_files_back_btn;
extern gui_text_t *playback_file_name_label;
extern gui_arc_t *playback_progress_bg;
extern gui_arc_t *playback_progress_fg;
extern gui_text_t *playback_current_time_label;
extern gui_text_t *playback_total_time_label;
extern gui_obj_t *playback_play_btn;
extern gui_win_t *recording_playback_window;
extern gui_text_t *recording_playback_time_label;
extern gui_win_t *win_9;
extern gui_img_t *recording_playback_back_btn;

// Toggle button state management function declarations
extern bool recording_record_btn_get_state(void);
extern void recording_record_btn_set_state(bool state);
extern bool playback_play_btn_get_state(void);
extern void playback_play_btn_set_state(bool state);

#endif // APP_RECORDING_UI_H
