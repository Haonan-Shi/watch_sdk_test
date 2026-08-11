/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_video_call_user.h"
#include "../ui/app_video_call_ui.h"
#include "gui_img.h"
#include "gui_view.h"
#include "gui_message.h"

#include <stdio.h>
#include <string.h>

// WiFi Camera feature flag
#ifndef CONFIG_WIFI_CAMERA
#define CONFIG_WIFI_CAMERA 0
#endif

#if CONFIG_WIFI_CAMERA
#include "wifi_gui.h"
#endif

#define RING_PULSE_FRAME_COUNT 30

// State variables
static bool mic_muted = false;
static bool speaker_muted = false;
static int ring_pulse_frame_index = 0;

// Video call state
#if CONFIG_WIFI_CAMERA
static bool video_call_active = false;
static bool video_streaming = false;
#endif

extern bool mic_btn_get_state(void) __attribute__((weak));
extern void mic_btn_set_state(bool state) __attribute__((weak));
extern bool speaker_btn_get_state(void) __attribute__((weak));
extern void speaker_btn_set_state(bool state) __attribute__((weak));

static void sync_call_button_images(void)
{
    if (mic_btn_get_state && mic_btn_set_state)
    {
        if (mic_btn_get_state() != mic_muted)
        {
            mic_btn_set_state(mic_muted);
        }
    }
    else if (mic_btn)
    {
        gui_img_set_src((gui_img_t *)mic_btn,
                        (const uint8_t *)(mic_muted ? "/app_video_call/mic_btn_active.bin" :
                                          "/app_video_call/mic_btn_normal.bin"),
                        IMG_SRC_FILESYS);
    }

    if (speaker_btn_get_state && speaker_btn_set_state)
    {
        if (speaker_btn_get_state() != speaker_muted)
        {
            speaker_btn_set_state(speaker_muted);
        }
    }
    else if (speaker_btn)
    {
        gui_img_set_src((gui_img_t *)speaker_btn,
                        (const uint8_t *)(speaker_muted ? "/app_video_call/speaker_btn_active.bin" :
                                          "/app_video_call/speaker_btn_normal.bin"),
                        IMG_SRC_FILESYS);
    }
}

static void update_ring_pulse_frame(void)
{
    char frame_path[72];

    if (!video_call_ring_pulse_img)
    {
        return;
    }

    snprintf(frame_path, sizeof(frame_path), "/app_video_call/ring_pulse/frame_%02d.bin",
             ring_pulse_frame_index);
    gui_img_set_src((gui_img_t *)video_call_ring_pulse_img, (const uint8_t *)frame_path,
                    IMG_SRC_FILESYS);
}

void video_call_calling_view_init_cb_impl(void)
{
    mic_muted = false;
    speaker_muted = false;
    ring_pulse_frame_index = 0;
    sync_call_button_images();
    update_ring_pulse_frame();
    gui_fb_change();
}

void ring_pulse_timer_cb_impl(void)
{
    ring_pulse_frame_index = (ring_pulse_frame_index + 1) % RING_PULSE_FRAME_COUNT;
    update_ring_pulse_frame();
    gui_fb_change();
}

void mic_toggle(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    if (mic_btn_get_state)
    {
        mic_muted = mic_btn_get_state();
    }
    else
    {
        mic_muted = !mic_muted;
        sync_call_button_images();
    }
    gui_fb_change();
}

void speaker_toggle(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    if (speaker_btn_get_state)
    {
        speaker_muted = speaker_btn_get_state();
    }
    else
    {
        speaker_muted = !speaker_muted;
        sync_call_button_images();
    }
    gui_fb_change();
}

void hangup_reset(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    mic_muted = false;
    speaker_muted = false;
    ring_pulse_frame_index = 0;
    sync_call_button_images();
    update_ring_pulse_frame();

#if CONFIG_WIFI_CAMERA
    // Stop WiFi camera if active
    if (video_call_active)
    {
        video_call_end(obj, e);
    }
#endif

    gui_fb_change();
    gui_view_switch_direct(gui_view_get_current(), "app_video_callIdleView", SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
}

/**
 * video_call_start: Start video call by enabling WiFi camera
 * Called when user initiates a video call
 */
void video_call_start(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

#if CONFIG_WIFI_CAMERA
    // Prevent re-entry
    if (video_call_active)
    {
        return;
    }

    video_call_active = true;
    video_streaming = false;

    // Start WiFi camera
    // type=0: enter camera mode, data=NULL (use default IP)
    char *ip_addr = "192.168.39.1";
    wifi_gui_to_app(WIFI_GUI_CAMERA_ENTER, ip_addr);

    gui_fb_change();
#else
    // If WiFi camera not enabled, just show connected status
    if (video_call_calling_status)
    {
        gui_text_content_set((gui_text_t *)video_call_calling_status, "Connected", 9);
    }
    gui_fb_change();
#endif
}

/**
 * video_call_end: Stop video call by disabling WiFi camera
 * Called when user ends the call
 */
void video_call_end(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

#if CONFIG_WIFI_CAMERA
    if (!video_call_active)
    {
        return;
    }

    // Exit WiFi camera
    wifi_gui_to_app(WIFI_GUI_CAMERA_EXIT, NULL);

    video_call_active = false;
    video_streaming = false;
#endif
}

/**
 * video_call_update_stream: Handle video stream update from WiFi camera
 * Called when "video_update" message is received
 * Replaces avatar image with video stream
 */
void video_call_update_stream(gui_obj_t *obj, const char *topic, void *data, uint16_t len)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(topic);
    GUI_UNUSED(data);
    GUI_UNUSED(len);

#if CONFIG_WIFI_CAMERA
    if (!video_call_active)
    {
        return;
    }

    // Update status to show streaming
    if (!video_streaming)
    {
        video_streaming = true;
        if (video_call_calling_status)
        {
            gui_text_content_set((gui_text_t *)video_call_calling_status, "Streaming...", 11);
        }
        if (video_call_ring_pulse_img)
        {
            gui_obj_hidden((gui_obj_t *)video_call_ring_pulse_img, true);
        }
    }

    gui_img_set_src((gui_img_t *)obj, (const uint8_t *)data, IMG_SRC_MEMADDR);
    gui_img_set_pos((gui_img_t *)obj, 0, 0);
    gui_fb_change();
#endif
}