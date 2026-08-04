/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_recording_callbacks.h"
#include "../ui/app_recording_ui.h"
#include "../user/app_recording_user.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// Time string global variables (defined in UI file)
extern char recording_main_time_label_time_str[10];
extern char recording_files_time_label_time_str[10];
extern char recording_playback_time_label_time_str[10];

// Timer animation counters
uint16_t recording_timer_label_timer_cnt = 0;
uint16_t recording_waveform_image_timer_cnt = 0;
uint16_t playback_current_time_label_timer_cnt = 0;

// Event callback function implementations

void recording_files_btn_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_recordingFilesView", SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
}

void recording_main_window_key_0_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    // Check key name
    if (strcmp(e->indev_name, "Home") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "SmartWatchTemplateMainView",
                               SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
    }
    else if (strcmp(e->indev_name, "Menu") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "app_menu_view", SWITCH_OUT_ANIMATION_FADE,
                               SWITCH_IN_ANIMATION_FADE);
    }
}

void win_7_clicked_0_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_menu_view", SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
}

void recording_files_window_key_0_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    // Check key name
    if (strcmp(e->indev_name, "Home") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "SmartWatchTemplateMainView",
                               SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
    }
    else if (strcmp(e->indev_name, "Menu") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "app_menu_view", SWITCH_OUT_ANIMATION_FADE,
                               SWITCH_IN_ANIMATION_FADE);
    }
}

void win_8_clicked_0_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_recordingMainView", SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
}

void recording_playback_window_key_0_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    // Check key name
    if (strcmp(e->indev_name, "Home") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "SmartWatchTemplateMainView",
                               SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
    }
    else if (strcmp(e->indev_name, "Menu") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "app_menu_view", SWITCH_OUT_ANIMATION_FADE,
                               SWITCH_IN_ANIMATION_FADE);
    }
}

void win_9_clicked_0_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_recordingFilesView", SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
}

void recording_main_time_label_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(recording_main_time_label_time_str, sizeof(recording_main_time_label_time_str),
             "%02d:%02d", t->tm_hour, t->tm_min);

    gui_text_content_set((gui_text_t *)recording_main_time_label, recording_main_time_label_time_str,
                         strlen(recording_main_time_label_time_str));
}

void recording_files_time_label_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(recording_files_time_label_time_str, sizeof(recording_files_time_label_time_str),
             "%02d:%02d", t->tm_hour, t->tm_min);

    gui_text_content_set((gui_text_t *)recording_files_time_label, recording_files_time_label_time_str,
                         strlen(recording_files_time_label_time_str));
}

void recording_playback_time_label_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(recording_playback_time_label_time_str, sizeof(recording_playback_time_label_time_str),
             "%02d:%02d", t->tm_hour, t->tm_min);

    gui_text_content_set((gui_text_t *)recording_playback_time_label,
                         recording_playback_time_label_time_str, strlen(recording_playback_time_label_time_str));
}

// Preset timer callback functions

/**
 * recording_timer
 * Component: recording_timer_label
 */
void recording_timer_tick(void *obj)
{
    GUI_UNUSED(obj);
    // Call the implementation function in protected area (if exists)
    // Define recording_timer_tick_impl() in custom_functions protected area for custom logic
#ifdef __cplusplus
    extern "C" {
#endif
    extern void recording_timer_tick_impl(void) __attribute__((weak));
#ifdef __cplusplus
}
#endif

if (recording_timer_tick_impl)
{
    recording_timer_tick_impl();
}
else
{
    // TODO: Implement timer callback logic
    // Or define recording_timer_tick_impl() in custom_functions protected area
}
}

/**
 * recording_waveform
 * Component: recording_waveform_image
 */
void recording_waveform_timer_cb(void *obj)
{
    GUI_UNUSED(obj);
    // Call the implementation function in protected area (if exists)
    // Define recording_waveform_timer_cb_impl() in custom_functions protected area for custom logic
#ifdef __cplusplus
    extern "C" {
#endif
    extern void recording_waveform_timer_cb_impl(void) __attribute__((weak));
#ifdef __cplusplus
}
#endif

if (recording_waveform_timer_cb_impl)
{
    recording_waveform_timer_cb_impl();
}
else
{
    // TODO: Implement timer callback logic
    // Or define recording_waveform_timer_cb_impl() in custom_functions protected area
}
}

/**
 * playback_timer
 * Component: playback_current_time_label
 */
void playback_timer_tick(void *obj)
{
    GUI_UNUSED(obj);
    // Call the implementation function in protected area (if exists)
    // Define playback_timer_tick_impl() in custom_functions protected area for custom logic
#ifdef __cplusplus
    extern "C" {
#endif
    extern void playback_timer_tick_impl(void) __attribute__((weak));
#ifdef __cplusplus
}
#endif

if (playback_timer_tick_impl)
{
    playback_timer_tick_impl();
}
else
{
    // TODO: Implement timer callback logic
    // Or define playback_timer_tick_impl() in custom_functions protected area
}
}

// Toggle button state callback functions

/* USER CODE BEGIN recording_record_btn_on_callback */
/**
 * recording_record_btn ON state callback
 * Called when button switches to ON state
 */
void recording_record_btn_on_callback(void)
{
    // TODO: Implement ON state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_play();
}
/* USER CODE END recording_record_btn_on_callback */

/* USER CODE BEGIN recording_record_btn_off_callback */
/**
 * recording_record_btn OFF state callback
 * Called when button switches to OFF state
 */
void recording_record_btn_off_callback(void)
{
    // TODO: Implement OFF state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_pause();
}
/* USER CODE END recording_record_btn_off_callback */

/* USER CODE BEGIN playback_play_btn_on_callback */
/**
 * playback_play_btn ON state callback
 * Called when button switches to ON state
 */
void playback_play_btn_on_callback(void)
{
    // TODO: Implement ON state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_play();
}
/* USER CODE END playback_play_btn_on_callback */

/* USER CODE BEGIN playback_play_btn_off_callback */
/**
 * playback_play_btn OFF state callback
 * Called when button switches to OFF state
 */
void playback_play_btn_off_callback(void)
{
    // TODO: Implement OFF state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_pause();
}
/* USER CODE END playback_play_btn_off_callback */

/* @protected start custom_functions */
// Custom functions
/* @protected end custom_functions */
