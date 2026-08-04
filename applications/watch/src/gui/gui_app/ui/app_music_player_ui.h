/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_music_player UI Definition (Auto-generated, do not modify manually)
 * Generated at: 2026-04-17T09:29:33.014Z
 */
#ifndef APP_MUSIC_PLAYER_UI_H
#define APP_MUSIC_PLAYER_UI_H

#include "guidef.h"
#include "gui_obj.h"
#include "gui_components_init.h"
#include "gui_view.h"
#include "gui_view_instance.h"
#include "gui_win.h"
#include "draw_font.h"
#include "font_types.h"
#include "gui_text.h"
#include "gui_arc.h"
#include "gui_img.h"
#include "gui_rect.h"
#include "gui_list.h"
#include "gui_scroll_text.h"

// Component handle declarations
extern gui_scroll_text_t *song_title_label;
extern gui_text_t *artist_label;
extern gui_arc_t *progress_ring_bg;
extern gui_arc_t *progress_ring;
extern gui_img_t *album_cover;
extern gui_img_t *skip_back_btn;
extern gui_img_t *play_pause_btn;
extern gui_img_t *skip_forward_btn;
extern gui_img_t *volume_btn;
extern gui_img_t *list_btn;
extern gui_win_t *volume_overlay_window;
extern gui_rounded_rect_t *volume_overlay_bg;
extern gui_text_t *volume_title_label;
extern gui_rounded_rect_t *volume_bar_bg;
extern gui_rounded_rect_t *volume_bar_fill;
extern gui_text_t *volume_percent_label;
extern gui_img_t *vol_minus_btn;
extern gui_img_t *vol_plus_btn;
extern gui_img_t *vol_close_btn;
extern gui_win_t *win_5;
extern gui_img_t *player_back_btn;
extern gui_list_t *playlist_list;
extern gui_win_t *playlist_window;
extern gui_text_t *playlist_title_label;
extern gui_win_t *win_6;
extern gui_img_t *playlist_back_btn;

#endif // APP_MUSIC_PLAYER_UI_H
