/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_music_player UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-14T03:23:12.179Z
 */
#include "app_music_player_ui.h"
#include "../callbacks/app_music_player_callbacks.h"
#include "../user/app_music_player_user.h"
#include <stddef.h>

// Component handle definitions
gui_scroll_text_t *song_title_label = NULL;
gui_text_t *artist_label = NULL;
gui_arc_t *progress_ring_bg = NULL;
gui_arc_t *progress_ring = NULL;
gui_img_t *album_cover = NULL;
gui_img_t *skip_back_btn = NULL;
gui_img_t *play_pause_btn = NULL;
gui_img_t *skip_forward_btn = NULL;
gui_img_t *volume_btn = NULL;
gui_img_t *list_btn = NULL;
gui_win_t *volume_overlay_window = NULL;
gui_rounded_rect_t *volume_overlay_bg = NULL;
gui_text_t *volume_title_label = NULL;
gui_rounded_rect_t *volume_bar_bg = NULL;
gui_rounded_rect_t *volume_bar_fill = NULL;
gui_text_t *volume_percent_label = NULL;
gui_img_t *vol_minus_btn = NULL;
gui_img_t *vol_plus_btn = NULL;
gui_img_t *vol_close_btn = NULL;
gui_win_t *win_5 = NULL;
gui_img_t *player_back_btn = NULL;
gui_list_t *playlist_list = NULL;
gui_win_t *playlist_window = NULL;
gui_text_t *playlist_title_label = NULL;
gui_win_t *win_6 = NULL;
gui_img_t *playlist_back_btn = NULL;

// List component note_design callback functions
// note_design callback function declaration
static void playlist_list_note_design(gui_obj_t *obj, void *param);

// note_design callback: delegates to user implementation for data-driven rendering
static void playlist_list_note_design(gui_obj_t *obj, void *param)
{
    GUI_UNUSED(param);
    playlist_note_design_impl(obj);
}


// Create app_music_playerPlayerView (hg_view)
static void app_music_playerPlayerView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_music_playerPlayerView_switch_in(gui_view_t *view)
{
    // Set animation step
    gui_view_set_animate_step(view, 50);

    // Set opacity
    gui_view_set_opacity(view, 255);

    // Set background color
    gui_set_bg_color(gui_rgb(41, 40, 38));

    GUI_UNUSED(view);


    // Create song_title_label (hg_label)
    song_title_label = gui_scroll_text_create((gui_obj_t *)view, "song_title_label", 55, 51, 300, 50);
    gui_scroll_text_set((gui_scroll_text_t *)song_title_label, "Let Me Know", GUI_FONT_SRC_BMP,
                        gui_rgb(242,
                                242,
                                242), 11, 36);
    gui_scroll_text_type_set((gui_scroll_text_t *)song_title_label,
                             "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
    // Required: initializes interval_time_ms. Without this, gui_scroll_text_prepare()
    // does `% interval_time_ms` (==0) on the first frame -> divide-by-zero fault/reboot.
    gui_scroll_text_scroll_set((gui_scroll_text_t *)song_title_label, SCROLL_X, 0, 0, 5000, 0);

    // Create artist_label (hg_label)
    artist_label = gui_text_create((gui_obj_t *)view, "artist_label", 55, 89, 300, 38);
    gui_text_set((gui_text_t *)artist_label, "No Wyld", GUI_FONT_SRC_BMP, gui_rgb(102, 102, 102), 7,
                 24);
    gui_text_type_set((gui_text_t *)artist_label, "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)artist_label, MID_CENTER);

    // Create progress_ring_bg (hg_arc)
    progress_ring_bg = gui_arc_create((gui_obj_t *)view, "progress_ring_bg", 205, 227, 96, 0, 360, 6,
                                      gui_rgba(249, 211, 66, 38));

    // Create skip_back_btn (hg_image)
    skip_back_btn = gui_img_create_from_fs((gui_obj_t *)view, "skip_back_btn",
                                           "/app_music_player/skip_back_btn.bin", 97, 348, 48, 48);
    gui_obj_add_event_cb(skip_back_btn, (gui_event_cb_t)skip_back_btn_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create play_pause_btn (hg_image)
    play_pause_btn = gui_img_create_from_fs((gui_obj_t *)view, "play_pause_btn",
                                            "/app_music_player/play_btn.bin", 177, 342, 56, 56);
    gui_obj_add_event_cb(play_pause_btn, (gui_event_cb_t)play_pause_btn_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create skip_forward_btn (hg_image)
    skip_forward_btn = gui_img_create_from_fs((gui_obj_t *)view, "skip_forward_btn",
                                              "/app_music_player/skip_forward_btn.bin", 265, 348, 48, 48);
    gui_obj_add_event_cb(skip_forward_btn, (gui_event_cb_t)skip_forward_btn_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create volume_btn (hg_image)
    volume_btn = gui_img_create_from_fs((gui_obj_t *)view, "volume_btn",
                                        "/app_music_player/volume_btn.bin", 60, 410, 52, 52);
    gui_obj_add_event_cb(volume_btn, (gui_event_cb_t)volume_btn_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create list_btn (hg_image)
    list_btn = gui_img_create_from_fs((gui_obj_t *)view, "list_btn", "/app_music_player/list_btn.bin",
                                      298, 410, 52, 52);
    gui_obj_add_event_cb(list_btn, (gui_event_cb_t)list_btn_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create win_5 (hg_window)
    win_5 = gui_win_create((gui_obj_t *)view, "win_5", 0, 0, 100, 100);


    // Create player_back_btn (hg_image)
    player_back_btn = gui_img_create_from_fs(win_5, "player_back_btn",
                                             "/app_music_player/back_icon.bin", 32, 28, 32, 32);
    gui_img_scale((gui_img_t *)player_back_btn, 0.666667f, 0.666667f);

    gui_obj_add_event_cb(GUI_BASE(win_5), (gui_event_cb_t)win_5_clicked_0_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create progress_ring (hg_arc)
    progress_ring = gui_arc_create((gui_obj_t *)view, "progress_ring", 205, 227, 96, -90, -90, 6,
                                   gui_rgb(249, 211, 66));
    // Bind timer: progress_anim
    gui_obj_create_timer((gui_obj_t *)progress_ring, 28, true, music_progress_timer_cb);
    gui_obj_start_timer((gui_obj_t *)progress_ring);

    // Create album_cover (hg_image)
    album_cover = gui_img_create_from_fs((gui_obj_t *)view, "album_cover",
                                         "/app_music_player/cover_1.bin", 115, 137, 180, 180);

    // Create volume_overlay_window (hg_window)
    volume_overlay_window = gui_win_create((gui_obj_t *)view, "volume_overlay_window", 0, 0, 410, 502);
    gui_obj_hidden((gui_obj_t *)volume_overlay_window, true);


    // Create volume_overlay_bg (hg_rect)
    volume_overlay_bg = gui_rect_create(volume_overlay_window, "volume_overlay_bg", 0, 0, 410, 502, 90,
                                        gui_rgba(20, 19, 17, 234));

    // Create volume_title_label (hg_label)
    volume_title_label = gui_text_create(volume_overlay_window, "volume_title_label", 155, 175, 100,
                                         24);
    gui_text_set((gui_text_t *)volume_title_label, "Volume", GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242),
                 6, 18);
    gui_text_type_set((gui_text_t *)volume_title_label,
                      "/font/Inter_24pt_Regular_size18_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)volume_title_label, MID_CENTER);

    // Create volume_bar_bg (hg_rect)
    volume_bar_bg = gui_rect_create(volume_overlay_window, "volume_bar_bg", 85, 211, 240, 10, 5,
                                    gui_rgb(38, 38, 38));

    // Create volume_percent_label (hg_label)
    volume_percent_label = gui_text_create(volume_overlay_window, "volume_percent_label", 173, 253, 64,
                                           50);
    gui_text_set((gui_text_t *)volume_percent_label, "70%", GUI_FONT_SRC_BMP, gui_rgb(249, 211, 66), 3,
                 40);
    gui_text_type_set((gui_text_t *)volume_percent_label,
                      "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)volume_percent_label, MID_CENTER);

    // Create vol_minus_btn (hg_image)
    vol_minus_btn = gui_img_create_from_fs(volume_overlay_window, "vol_minus_btn",
                                           "/app_music_player/minus_btn.bin", 69, 237, 72, 72);
    gui_obj_add_event_cb(vol_minus_btn, (gui_event_cb_t)vol_minus_btn_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create vol_plus_btn (hg_image)
    vol_plus_btn = gui_img_create_from_fs(volume_overlay_window, "vol_plus_btn",
                                          "/app_music_player/plus_btn.bin", 269, 237, 72, 72);
    gui_obj_add_event_cb(vol_plus_btn, (gui_event_cb_t)vol_plus_btn_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create vol_close_btn (hg_image)
    vol_close_btn = gui_img_create_from_fs(volume_overlay_window, "vol_close_btn",
                                           "/app_music_player/close_btn.bin", 177, 345, 56, 56);
    gui_obj_add_event_cb(vol_close_btn, (gui_event_cb_t)vol_close_btn_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create volume_bar_fill (hg_rect)
    volume_bar_fill = gui_rect_create(volume_overlay_window, "volume_bar_fill", 85, 211, 168, 10, 5,
                                      gui_rgb(249, 211, 66));

    gui_obj_add_event_cb((gui_obj_t *)view, (gui_event_cb_t)app_music_playerPlayerView_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)view);

    // Call user init callback for topic subscriptions and snapshot request
    player_view_init_cb_impl();
}
GUI_VIEW_INSTANCE("app_music_playerPlayerView", false, app_music_playerPlayerView_switch_in,
                  app_music_playerPlayerView_switch_out, false);

// Create app_music_playerPlaylistView (hg_view)
static void app_music_playerPlaylistView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_music_playerPlaylistView_switch_in(gui_view_t *view)
{
    // Set animation step
    gui_view_set_animate_step(view, 50);

    // Set opacity
    gui_view_set_opacity(view, 255);

    // Set background color
    gui_set_bg_color(gui_rgb(41, 40, 38));

    GUI_UNUSED(view);


    // Create playlist_list (hg_list)
    playlist_list = gui_list_create((gui_obj_t *)view, "playlist_list", 0, 80, 410, 422, 84, 4,
                                    VERTICAL, playlist_list_note_design, NULL, false);
    gui_list_set_style(playlist_list, LIST_CLASSIC);
    gui_list_set_note_num(playlist_list, 0);
    gui_list_set_out_scope(playlist_list, 80);
    gui_list_keep_note_alive(playlist_list, true);

    // Create playlist_window (hg_window)
    playlist_window = gui_win_create((gui_obj_t *)view, "playlist_window", 0, 0, 410, 80);
    gui_win_enable_blur((gui_win_t *)playlist_window, true);
    gui_win_set_blur_degree((gui_win_t *)playlist_window, 225);


    // Create playlist_title_label (hg_label)
    playlist_title_label = gui_text_create(playlist_window, "playlist_title_label", 105, 24, 193, 50);
    gui_text_set((gui_text_t *)playlist_title_label, "Playlist", GUI_FONT_SRC_BMP, gui_rgb(242, 242,
                 242), 8, 40);
    gui_text_type_set((gui_text_t *)playlist_title_label,
                      "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)playlist_title_label, MID_CENTER);

    // Create win_6 (hg_window)
    win_6 = gui_win_create(playlist_window, "win_6", 0, 0, 100, 100);


    // Create playlist_back_btn (hg_image)
    playlist_back_btn = gui_img_create_from_fs(win_6, "playlist_back_btn",
                                               "/app_music_player/back_icon.bin", 32, 28, 32, 32);
    gui_img_scale((gui_img_t *)playlist_back_btn, 0.666667f, 0.666667f);
    gui_obj_add_event_cb(playlist_back_btn, (gui_event_cb_t)playlist_back_btn_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    gui_obj_add_event_cb(GUI_BASE(win_6), (gui_event_cb_t)win_6_clicked_0_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    gui_obj_add_event_cb(GUI_BASE(playlist_window), (gui_event_cb_t)playlist_window_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)playlist_window);

    // Call user init callback for topic subscriptions and dynamic note count
    playlist_view_init_cb_impl();
}
GUI_VIEW_INSTANCE("app_music_playerPlaylistView", false, app_music_playerPlaylistView_switch_in,
                  app_music_playerPlaylistView_switch_out, false);
