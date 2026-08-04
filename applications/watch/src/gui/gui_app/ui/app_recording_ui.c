/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_recording UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-14T03:23:12.236Z
 */
#include "app_recording_ui.h"
#include "../callbacks/app_recording_callbacks.h"
#include "../user/app_recording_user.h"
#include <stddef.h>
#include <time.h>

// Component handle definitions
gui_text_t *recording_timer_label = NULL;
gui_img_t *recording_waveform_image = NULL;
gui_text_t *recording_status_label = NULL;
gui_obj_t *recording_record_btn = NULL;
gui_img_t *recording_files_btn = NULL;
gui_win_t *recording_main_window = NULL;
gui_text_t *recording_main_time_label = NULL;
gui_win_t *win_7 = NULL;
gui_img_t *recording_main_back_btn = NULL;
gui_list_t *recording_files_list = NULL;
gui_rounded_rect_t *recording_file_bg_0 = NULL;
gui_text_t *recording_file_name_0 = NULL;
gui_text_t *recording_file_duration_0 = NULL;
gui_img_t *recording_file_chevron_0 = NULL;
gui_rounded_rect_t *recording_file_bg_1 = NULL;
gui_text_t *recording_file_name_1 = NULL;
gui_text_t *recording_file_duration_1 = NULL;
gui_img_t *recording_file_chevron_1 = NULL;
gui_rounded_rect_t *recording_file_bg_2 = NULL;
gui_text_t *recording_file_name_2 = NULL;
gui_text_t *recording_file_duration_2 = NULL;
gui_img_t *recording_file_chevron_2 = NULL;
gui_rounded_rect_t *recording_file_bg_3 = NULL;
gui_text_t *recording_file_name_3 = NULL;
gui_text_t *recording_file_duration_3 = NULL;
gui_img_t *recording_file_chevron_3 = NULL;
gui_rounded_rect_t *recording_file_bg_4 = NULL;
gui_text_t *recording_file_name_4 = NULL;
gui_text_t *recording_file_duration_4 = NULL;
gui_img_t *recording_file_chevron_4 = NULL;
gui_text_t *recording_files_empty_label = NULL;
gui_win_t *recording_files_window = NULL;
gui_text_t *recording_files_time_label = NULL;
gui_text_t *recording_files_title_label = NULL;
gui_win_t *win_8 = NULL;
gui_img_t *recording_files_back_btn = NULL;
gui_text_t *playback_file_name_label = NULL;
gui_arc_t *playback_progress_bg = NULL;
gui_arc_t *playback_progress_fg = NULL;
gui_text_t *playback_current_time_label = NULL;
gui_text_t *playback_total_time_label = NULL;
gui_obj_t *playback_play_btn = NULL;
gui_win_t *recording_playback_window = NULL;
gui_text_t *recording_playback_time_label = NULL;
gui_win_t *win_9 = NULL;
gui_img_t *recording_playback_back_btn = NULL;

// Time string global variables
char recording_main_time_label_time_str[10] = {0};
char recording_files_time_label_time_str[10] = {0};
char recording_playback_time_label_time_str[10] = {0};

// Toggle button callback functions

// recording_record_btn dual-state button callback
static bool recording_record_btn_state = false;

void recording_record_btn_toggle_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    // Toggle state
    recording_record_btn_state = !recording_record_btn_state;

    // Switch image based on state and call corresponding callback
    if (recording_record_btn_state)
    {
        gui_img_set_src((gui_img_t *)recording_record_btn,
                        (const uint8_t *)"/app_recording/record_btn_recording.bin", IMG_SRC_FILESYS);
        extern void recording_start(void *obj, gui_event_t *e);
        recording_start(obj, e);
    }
    else
    {
        gui_img_set_src((gui_img_t *)recording_record_btn,
                        (const uint8_t *)"/app_recording/record_btn_idle.bin", IMG_SRC_FILESYS);
        extern void recording_start(void *obj, gui_event_t *e);
        recording_stop(obj, e);
    }
    gui_fb_change();
}

// Get current state
bool recording_record_btn_get_state(void)
{
    return recording_record_btn_state;
}

// Set state (external call)
void recording_record_btn_set_state(bool state)
{
    if (recording_record_btn_state != state)
    {
        recording_record_btn_state = state;

        if (state)
        {
            gui_img_set_src((gui_img_t *)recording_record_btn,
                            (const uint8_t *)"/app_recording/record_btn_recording.bin", IMG_SRC_FILESYS);
            extern void recording_start(void *obj, gui_event_t *e);
            recording_start(NULL, NULL);
        }
        else
        {
            gui_img_set_src((gui_img_t *)recording_record_btn,
                            (const uint8_t *)"/app_recording/record_btn_idle.bin", IMG_SRC_FILESYS);
            extern void recording_start(void *obj, gui_event_t *e);
            recording_stop(NULL, NULL);
        }
    }
}

// playback_play_btn dual-state button callback
static bool playback_play_btn_state = false;

void playback_play_btn_toggle_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    // Toggle state
    playback_play_btn_state = !playback_play_btn_state;

    // Switch image based on state and call corresponding callback
    if (playback_play_btn_state)
    {
        gui_img_set_src((gui_img_t *)playback_play_btn, (const uint8_t *)"/app_recording/pause_btn.bin",
                        IMG_SRC_FILESYS);
        extern void playback_play(void *obj, gui_event_t *e);
        playback_play(obj, e);
    }
    else
    {
        gui_img_set_src((gui_img_t *)playback_play_btn, (const uint8_t *)"/app_recording/play_btn.bin",
                        IMG_SRC_FILESYS);
        extern void playback_play(void *obj, gui_event_t *e);
        playback_pause(obj, e);
    }
    gui_fb_change();
}

// Get current state
bool playback_play_btn_get_state(void)
{
    return playback_play_btn_state;
}

// Set state (external call)
void playback_play_btn_set_state(bool state)
{
    if (playback_play_btn_state != state)
    {
        playback_play_btn_state = state;

        if (state)
        {
            gui_img_set_src((gui_img_t *)playback_play_btn, (const uint8_t *)"/app_recording/pause_btn.bin",
                            IMG_SRC_FILESYS);
            extern void playback_play(void *obj, gui_event_t *e);
            playback_play(NULL, NULL);
        }
        else
        {
            gui_img_set_src((gui_img_t *)playback_play_btn, (const uint8_t *)"/app_recording/play_btn.bin",
                            IMG_SRC_FILESYS);
            extern void playback_play(void *obj, gui_event_t *e);
            playback_pause(NULL, NULL);
        }
    }
}
// List component note_design callback functions
// note_design callback function declaration
static void recording_files_list_note_design(gui_obj_t *obj, void *param);

// note_design callback function implementation
static void recording_files_list_note_design(gui_obj_t *obj, void *param)
{
    GUI_UNUSED(param);
    recording_files_note_design_impl(obj);
}


// Create app_recordingMainView (hg_view)
static void app_recordingMainView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_recordingMainView_switch_in(gui_view_t *view)
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



    // Create recording_timer_label (hg_label)
    recording_timer_label = gui_text_create((gui_obj_t *)view, "recording_timer_label", 105, 88, 200,
                                            58);
    gui_text_set((gui_text_t *)recording_timer_label, "00:00", GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242),
                 5, 48);
    gui_text_type_set((gui_text_t *)recording_timer_label,
                      "/font/Inter_24pt_Regular_size48_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)recording_timer_label, MID_CENTER);
    // Bind timer: recording_timer
    gui_obj_create_timer((gui_obj_t *)recording_timer_label, 1000, true, recording_timer_tick);
    gui_obj_start_timer((gui_obj_t *)recording_timer_label);

    // Create recording_waveform_image (hg_image)
    recording_waveform_image = gui_img_create_from_fs((gui_obj_t *)view, "recording_waveform_image",
                                                      "/app_recording/waveform/inactive/frame_00.bin", 136, 152, 137, 50);
    // Bind timer: recording_waveform
    gui_obj_create_timer((gui_obj_t *)recording_waveform_image, 33, true, recording_waveform_timer_cb);
    gui_obj_start_timer((gui_obj_t *)recording_waveform_image);

    // Create recording_status_label (hg_label)
    recording_status_label = gui_text_create((gui_obj_t *)view, "recording_status_label", 55, 210, 304,
                                             50);
    gui_text_set((gui_text_t *)recording_status_label, "", GUI_FONT_SRC_BMP, gui_rgb(48, 209, 88), 0,
                 36);
    gui_text_type_set((gui_text_t *)recording_status_label,
                      "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)recording_status_label, MID_CENTER);

    // Create recording_record_btn (hg_button)
    recording_record_btn = (gui_obj_t *)gui_img_create_from_fs((gui_obj_t *)view,
                                                               "recording_record_btn", "/app_recording/record_btn_idle.bin", 169, 266, 72, 72);
    if (recording_record_btn_state)
    {
        gui_img_set_src((gui_img_t *)recording_record_btn,
                        (const uint8_t *)"/app_recording/record_btn_recording.bin", IMG_SRC_FILESYS);
    }
    gui_obj_add_event_cb((gui_obj_t *)recording_record_btn, recording_record_btn_toggle_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create recording_files_btn (hg_image)
    recording_files_btn = gui_img_create_from_fs((gui_obj_t *)view, "recording_files_btn",
                                                 "/app_recording/files_btn.bin", 179, 362, 52, 52);
    gui_obj_add_event_cb(recording_files_btn, (gui_event_cb_t)recording_files_btn_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create recording_main_window (hg_window)
    recording_main_window = gui_win_create((gui_obj_t *)view, "recording_main_window", 0, 0, 410, 110);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(recording_main_time_label_time_str, sizeof(recording_main_time_label_time_str),
                 "%02d:%02d", t->tm_hour, t->tm_min);
    }


    // Create recording_main_time_label (hg_time_label)
    recording_main_time_label = gui_text_create(recording_main_window, "recording_main_time_label", 300,
                                                20, 80, 32);
    gui_text_set((gui_text_t *)recording_main_time_label, recording_main_time_label_time_str,
                 GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), strlen(recording_main_time_label_time_str), 28);
    gui_text_type_set((gui_text_t *)recording_main_time_label,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)recording_main_time_label, RIGHT);

    // Create win_7 (hg_window)
    win_7 = gui_win_create(recording_main_window, "win_7", -2, -6, 100, 100);


    // Create recording_main_back_btn (hg_image)
    recording_main_back_btn = gui_img_create_from_fs(win_7, "recording_main_back_btn",
                                                     "/app_recording/back_icon.bin", 32, 32, 32, 32);

    gui_obj_add_event_cb(GUI_BASE(win_7), (gui_event_cb_t)win_7_clicked_0_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    gui_obj_add_event_cb(GUI_BASE(recording_main_window),
                         (gui_event_cb_t)recording_main_window_key_0_cb, GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)recording_main_window);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(recording_main_time_label), 30000, true,
                         recording_main_time_label_time_update_cb);

    // Call user init callback for topic subscriptions and initial render
    recording_main_init_cb_impl();
}
GUI_VIEW_INSTANCE("app_recordingMainView", false, app_recordingMainView_switch_in,
                  app_recordingMainView_switch_out, false);

// Create app_recordingFilesView (hg_view)
static void app_recordingFilesView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_recordingFilesView_switch_in(gui_view_t *view)
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



    // Create recording_files_list (hg_list)
    recording_files_list = gui_list_create((gui_obj_t *)view, "recording_files_list", 0, 128, 410, 374,
                                           84, 8, VERTICAL, recording_files_list_note_design, NULL, false);
    gui_list_set_style(recording_files_list, LIST_CLASSIC);
    gui_list_set_note_num(recording_files_list, 0);
    gui_list_set_out_scope(recording_files_list, 80);
    gui_list_keep_note_alive(recording_files_list, true);

    // Create recording_files_empty_label (hg_label)
    recording_files_empty_label = gui_text_create((gui_obj_t *)view, "recording_files_empty_label", 30,
                                                  254, 350, 58);
    gui_text_set((gui_text_t *)recording_files_empty_label, "No recordings yet", GUI_FONT_SRC_BMP,
                 gui_rgb(102, 102, 102), 17, 48);
    gui_text_type_set((gui_text_t *)recording_files_empty_label,
                      "/font/Inter_24pt_Regular_size48_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)recording_files_empty_label, MID_CENTER);
    gui_obj_hidden((gui_obj_t *)recording_files_empty_label, true);

    // Create recording_files_window (hg_window)
    recording_files_window = gui_win_create((gui_obj_t *)view, "recording_files_window", 0, 0, 410,
                                            128);
    gui_win_enable_blur((gui_win_t *)recording_files_window, true);
    gui_win_set_blur_degree((gui_win_t *)recording_files_window, 225);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(recording_files_time_label_time_str, sizeof(recording_files_time_label_time_str),
                 "%02d:%02d", t->tm_hour, t->tm_min);
    }


    // Create recording_files_time_label (hg_time_label)
    recording_files_time_label = gui_text_create(recording_files_window, "recording_files_time_label",
                                                 300, 20, 80, 32);
    gui_text_set((gui_text_t *)recording_files_time_label, recording_files_time_label_time_str,
                 GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), strlen(recording_files_time_label_time_str), 28);
    gui_text_type_set((gui_text_t *)recording_files_time_label,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)recording_files_time_label, RIGHT);

    // Create recording_files_title_label (hg_label)
    recording_files_title_label = gui_text_create(recording_files_window, "recording_files_title_label",
                                                  105, 70, 200, 50);
    gui_text_set((gui_text_t *)recording_files_title_label, "Recordings", GUI_FONT_SRC_BMP, gui_rgb(242,
                 242, 242), 10, 36);
    gui_text_type_set((gui_text_t *)recording_files_title_label,
                      "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)recording_files_title_label, MID_CENTER);

    // Create win_8 (hg_window)
    win_8 = gui_win_create(recording_files_window, "win_8", 0, 0, 100, 100);


    // Create recording_files_back_btn (hg_image)
    recording_files_back_btn = gui_img_create_from_fs(win_8, "recording_files_back_btn",
                                                      "/app_recording/back_icon.bin", 32, 28, 32, 32);

    gui_obj_add_event_cb(GUI_BASE(win_8), (gui_event_cb_t)win_8_clicked_0_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    gui_obj_add_event_cb(GUI_BASE(recording_files_window),
                         (gui_event_cb_t)recording_files_window_key_0_cb, GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)recording_files_window);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(recording_files_time_label), 30000, true,
                         recording_files_time_label_time_update_cb);

    // Call user init callback for topic subscriptions and snapshot request
    recording_files_init_cb_impl();
}
GUI_VIEW_INSTANCE("app_recordingFilesView", false, app_recordingFilesView_switch_in,
                  app_recordingFilesView_switch_out, false);

// Create app_recordingPlaybackView (hg_view)
static void app_recordingPlaybackView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_recordingPlaybackView_switch_in(gui_view_t *view)
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



    // Create playback_file_name_label (hg_label)
    playback_file_name_label = gui_text_create((gui_obj_t *)view, "playback_file_name_label", 55, 84,
                                               300, 50);
    gui_text_set((gui_text_t *)playback_file_name_label, "Recording 001", GUI_FONT_SRC_BMP, gui_rgb(242,
                 242, 242), 13, 36);
    gui_text_type_set((gui_text_t *)playback_file_name_label,
                      "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)playback_file_name_label, MID_CENTER);

    // Create playback_progress_bg (hg_arc)
    playback_progress_bg = gui_arc_create((gui_obj_t *)view, "playback_progress_bg", 205, 222, 85, 0,
                                          360, 6, gui_rgba(48, 209, 88, 38));

    // Create playback_play_btn (hg_button)
    playback_play_btn = (gui_obj_t *)gui_img_create_from_fs((gui_obj_t *)view, "playback_play_btn",
                                                            "/app_recording/play_btn.bin", 173, 342, 64, 64);
    if (playback_play_btn_state)
    {
        gui_img_set_src((gui_img_t *)playback_play_btn, (const uint8_t *)"/app_recording/pause_btn.bin",
                        IMG_SRC_FILESYS);
    }
    gui_obj_add_event_cb((gui_obj_t *)playback_play_btn, playback_play_btn_toggle_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create playback_progress_fg (hg_arc)
    playback_progress_fg = gui_arc_create((gui_obj_t *)view, "playback_progress_fg", 205, 222, 85, -90,
                                          -90, 6, gui_rgb(48, 209, 88));

    // Create playback_current_time_label (hg_label)
    playback_current_time_label = gui_text_create((gui_obj_t *)view, "playback_current_time_label", 145,
                                                  190, 120, 50);
    gui_text_set((gui_text_t *)playback_current_time_label, "00:00", GUI_FONT_SRC_BMP, gui_rgb(242, 242,
                 242), 5, 40);
    gui_text_type_set((gui_text_t *)playback_current_time_label,
                      "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)playback_current_time_label, MID_CENTER);
    // Bind timer: playback_timer
    gui_obj_create_timer((gui_obj_t *)playback_current_time_label, 100, true, playback_timer_tick);
    gui_obj_start_timer((gui_obj_t *)playback_current_time_label);

    // Create playback_total_time_label (hg_label)
    playback_total_time_label = gui_text_create((gui_obj_t *)view, "playback_total_time_label", 145,
                                                232, 120, 46);
    gui_text_set((gui_text_t *)playback_total_time_label, "/ 00:15", GUI_FONT_SRC_BMP, gui_rgb(102, 102,
                 102), 7, 36);
    gui_text_type_set((gui_text_t *)playback_total_time_label,
                      "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)playback_total_time_label, MID_CENTER);

    // Create recording_playback_window (hg_window)
    recording_playback_window = gui_win_create((gui_obj_t *)view, "recording_playback_window", 0, 0,
                                               410, 110);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(recording_playback_time_label_time_str, sizeof(recording_playback_time_label_time_str),
                 "%02d:%02d", t->tm_hour, t->tm_min);
    }


    // Create recording_playback_time_label (hg_time_label)
    recording_playback_time_label = gui_text_create(recording_playback_window,
                                                    "recording_playback_time_label", 300, 20, 80, 32);
    gui_text_set((gui_text_t *)recording_playback_time_label, recording_playback_time_label_time_str,
                 GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), strlen(recording_playback_time_label_time_str), 28);
    gui_text_type_set((gui_text_t *)recording_playback_time_label,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)recording_playback_time_label, RIGHT);

    // Create win_9 (hg_window)
    win_9 = gui_win_create(recording_playback_window, "win_9", 0, 0, 100, 100);


    // Create recording_playback_back_btn (hg_image)
    recording_playback_back_btn = gui_img_create_from_fs(win_9, "recording_playback_back_btn",
                                                         "/app_recording/back_icon.bin", 32, 28, 32, 32);

    gui_obj_add_event_cb(GUI_BASE(win_9), (gui_event_cb_t)win_9_clicked_0_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    gui_obj_add_event_cb(GUI_BASE(recording_playback_window),
                         (gui_event_cb_t)recording_playback_window_key_0_cb, GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)recording_playback_window);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(recording_playback_time_label), 30000, true,
                         recording_playback_time_label_time_update_cb);

    // Call user init callback for topic subscriptions
    recording_playback_init_cb_impl();
}
GUI_VIEW_INSTANCE("app_recordingPlaybackView", false, app_recordingPlaybackView_switch_in,
                  app_recordingPlaybackView_switch_out, false);
