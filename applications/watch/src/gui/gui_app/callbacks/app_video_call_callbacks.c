/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_video_call_callbacks.h"
#include "../ui/app_video_call_ui.h"
#include "../user/app_video_call_user.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// Time string global variables (defined in UI file)
extern char video_call_idle_time_label_time_str[10];
extern char video_call_calling_time_label_time_str[10];

// Timer animation counters
uint16_t video_call_ring_pulse_img_timer_cnt = 0;

// Event callback function implementations

void video_call_btn_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_video_callCallingView",
                           SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
    video_call_start(obj, e);
}

void video_call_idle_back_icon_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_menu_view", SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
}

void win_video_call_idle_back_clicked_0_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_menu_view", SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
}

void video_call_hangup_btn_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    hangup_reset(obj, e);
}

void video_call_calling_back_icon_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    hangup_reset(obj, e);
}

void win_video_calling_back_clicked_0_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_video_callIdleView", SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
}

void video_call_calling_avatar_msg_cb_0(gui_obj_t *obj, const char *topic, void *data, uint16_t len)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(topic);
    GUI_UNUSED(data);
    GUI_UNUSED(len);
    video_call_update_stream(obj, topic, data, len);
}

void video_call_idle_time_label_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(video_call_idle_time_label_time_str, sizeof(video_call_idle_time_label_time_str),
             "%02d:%02d", t->tm_hour, t->tm_min);

    gui_text_content_set((gui_text_t *)video_call_idle_time_label, video_call_idle_time_label_time_str,
                         strlen(video_call_idle_time_label_time_str));
}

void video_call_calling_time_label_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(video_call_calling_time_label_time_str, sizeof(video_call_calling_time_label_time_str),
             "%02d:%02d", t->tm_hour, t->tm_min);

    gui_text_content_set((gui_text_t *)video_call_calling_time_label,
                         video_call_calling_time_label_time_str, strlen(video_call_calling_time_label_time_str));
}

// Preset timer callback functions

/**
 * ring_pulse
 * Component: video_call_ring_pulse_img
 */
void ring_pulse_timer_cb(void *obj)
{
    GUI_UNUSED(obj);
    // Call the implementation function in protected area (if exists)
    // Define ring_pulse_timer_cb_impl() in custom_functions protected area for custom logic
#ifdef __cplusplus
    extern "C" {
#endif
    extern void ring_pulse_timer_cb_impl(void) __attribute__((weak));
#ifdef __cplusplus
}
#endif

if (ring_pulse_timer_cb_impl)
{
    ring_pulse_timer_cb_impl();
}
else
{
    // TODO: Implement timer callback logic
    // Or define ring_pulse_timer_cb_impl() in custom_functions protected area
}
}

// Toggle button state callback functions

/* USER CODE BEGIN mic_btn_on_callback */
/**
 * mic_btn ON state callback
 * Called when button switches to ON state
 */
void mic_btn_on_callback(void)
{
    // TODO: Implement ON state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_play();
}
/* USER CODE END mic_btn_on_callback */

/* USER CODE BEGIN mic_btn_off_callback */
/**
 * mic_btn OFF state callback
 * Called when button switches to OFF state
 */
void mic_btn_off_callback(void)
{
    // TODO: Implement OFF state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_pause();
}
/* USER CODE END mic_btn_off_callback */

/* USER CODE BEGIN speaker_btn_on_callback */
/**
 * speaker_btn ON state callback
 * Called when button switches to ON state
 */
void speaker_btn_on_callback(void)
{
    // TODO: Implement ON state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_play();
}
/* USER CODE END speaker_btn_on_callback */

/* USER CODE BEGIN speaker_btn_off_callback */
/**
 * speaker_btn OFF state callback
 * Called when button switches to OFF state
 */
void speaker_btn_off_callback(void)
{
    // TODO: Implement OFF state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_pause();
}
/* USER CODE END speaker_btn_off_callback */

/* @protected start custom_functions */
// Custom functions
/* @protected end custom_functions */
