/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_noise_user.h"
#include <time.h>

/**
 * User custom implementation
 * This file is generated only once and can be freely modified
 */

// Add custom implementation here

/***
 * Template function
 * Distinguish development environments
 */
// void user_defined_func_called_by_event(void *obj, gui_event_t *e)
// {
//     GUI_UNUSED(obj);
//     GUI_UNUSED(e);
// #ifdef _HONEYGUI_SIMULATOR_
//     // TODO
// #else
//     // TODO
// #endif
// }

// void user_defined_func_called_by_msg(gui_obj_t *obj, const char *topic, void *data, uint16_t len)
// {
//     GUI_UNUSED(obj);
//     GUI_UNUSED(topic);
//     GUI_UNUSED(data);
//     GUI_UNUSED(len);
// #ifdef _HONEYGUI_SIMULATOR_
//     // TODO
// #else
//     // TODO
// #endif
// }


// Noise level related variables
static int current_noise_level = 50;  // Current noise level (dB)
static int target_noise_level = 50;   // Target noise level (dB)
static gui_rounded_rect_t *noise_meters[15];   // Noise bar array

// Noise level color definitions
typedef struct
{
    int threshold;
    gui_color_t color;
} noise_color_t;

static const noise_color_t noise_colors[] =
{
    {60, {.color.rgba = {0x71, 0xE1, 0x68, 0xFF}}},   // Green (safe)
    {80, {.color.rgba = {0x00, 0xFF, 0xFF, 0xFF}}},   // Yellow (caution)
    {100, {.color.rgba = {0x00, 0x66, 0xFF, 0xFF}}},  // Orange (warning)
    {120, {.color.rgba = {0x00, 0x00, 0xFF, 0xFF}}}   // Red (danger)
};

/**
 * Initialize the noise display system
 */
void app_noise_init(void)
{
    // Get handles of all noise bars
    noise_meters[0] = (gui_rounded_rect_t *)Nois_Level_Meter0;
    noise_meters[1] = (gui_rounded_rect_t *)Nois_Level_Meter1;
    noise_meters[2] = (gui_rounded_rect_t *)Nois_Level_Meter2;
    noise_meters[3] = (gui_rounded_rect_t *)Nois_Level_Meter3;
    noise_meters[4] = (gui_rounded_rect_t *)Nois_Level_Meter4;
    noise_meters[5] = (gui_rounded_rect_t *)Nois_Level_Meter5;
    noise_meters[6] = (gui_rounded_rect_t *)Nois_Level_Meter6;
    noise_meters[7] = (gui_rounded_rect_t *)Nois_Level_Meter7;
    noise_meters[8] = (gui_rounded_rect_t *)Nois_Level_Meter8;
    noise_meters[9] = (gui_rounded_rect_t *)Nois_Level_Meter9;
    noise_meters[10] = (gui_rounded_rect_t *)Nois_Level_Meter10;
    noise_meters[11] = (gui_rounded_rect_t *)Nois_Level_Meter11;
    noise_meters[12] = (gui_rounded_rect_t *)Nois_Level_Meter12;
    noise_meters[13] = (gui_rounded_rect_t *)Nois_Level_Meter13;
    noise_meters[14] = (gui_rounded_rect_t *)Nois_Level_Meter14;

    // Initialize the random number seed
    srand(time(NULL));

    // Immediately show the initial state (50dB)
    current_noise_level = 50;
    target_noise_level = 50;
    update_noise_meters(50);
    update_noise_status(50);

    // Start the noise simulation timer (updates every 100ms) - use noise_meters[2] to avoid conflict with the init timer
    gui_obj_create_timer((gui_obj_t *)noise_meters[2], 100, -1, noise_simulation_timer_cb);
    gui_obj_start_timer((gui_obj_t *)noise_meters[2]);

    // Start the display update timer (updates every 50ms for smooth animation)
    gui_obj_create_timer((gui_obj_t *)noise_meters[3], 50, -1, noise_display_timer_cb);
    gui_obj_start_timer((gui_obj_t *)noise_meters[3]);
}

/**
 * Noise simulation timer callback - simulates changes in real noise data
 */
void noise_simulation_timer_cb(void *obj)
{
    GUI_UNUSED(obj);  // Avoid unused parameter warning

    // Simulate noise level changes (30-120 dB)
    static int noise_trend = 0;

    // Random change trend
    if (rand() % 10 == 0)
    {
        noise_trend = (rand() % 3) - 1; // -1, 0, 1
    }

    // Apply trend and random fluctuation
    int noise_change = noise_trend + (rand() % 7) - 3; // Random change from -3 to +3
    target_noise_level += noise_change;

    // Clamp the range
    if (target_noise_level < 30) { target_noise_level = 30; }
    if (target_noise_level > 120) { target_noise_level = 120; }

    // Update the displayed decibel value
    static char noise_text[16];
    snprintf(noise_text, sizeof(noise_text), "%d dB", target_noise_level);
    gui_text_content_set((gui_text_t *)app_noise_data_text, noise_text, strlen(noise_text));

    // Update status text and color
    update_noise_status(target_noise_level);
}

/**
 * Noise display timer callback - implements smooth bar chart animation
 */
void noise_display_timer_cb(void *obj)
{
    GUI_UNUSED(obj);  // Avoid unused parameter warning

    // Smoothly transition to the target noise level
    if (current_noise_level < target_noise_level)
    {
        current_noise_level++;
    }
    else if (current_noise_level > target_noise_level)
    {
        current_noise_level--;
    }

    // Update the noise bar display
    update_noise_meters(current_noise_level);
}

/**
 * Update the noise bar display
 */
void update_noise_meters(int noise_db)
{
    // Calculate how many bars should be shown (30dB=0 bars, 120dB=15 bars)
    int active_meters = ((noise_db - 30) * 15) / 90;
    if (active_meters < 0) { active_meters = 0; }
    if (active_meters > 15) { active_meters = 15; }

    // Update the display state of each bar
    for (int i = 0; i < 15; i++)
    {
        if (noise_meters[i] != NULL)
        {
            if (i < active_meters)
            {
                // Show the bar, set color according to level
                gui_color_t color = get_noise_color(noise_db);
                gui_rect_set_color(noise_meters[i], color);
                gui_rect_set_opacity(noise_meters[i], 255);
            }
            else
            {
                // Hide the bar
                gui_rect_set_opacity(noise_meters[i], 0);
            }
        }
    }
}

/**
 * Get the color based on the noise level
 */
gui_color_t get_noise_color(int noise_db)
{
    for (size_t i = 0; i < sizeof(noise_colors) / sizeof(noise_colors[0]); i++)
    {
        if (noise_db <= noise_colors[i].threshold)
        {
            return noise_colors[i].color;
        }
    }
    // Default red
    gui_color_t red = {.color.rgba = {0x00, 0x00, 0xFF, 0xFF}};
    return red;
}

/**
 * Update the noise status text
 */
void update_noise_status(int noise_db)
{
    const char *status_text;
    const char *icon_path;
    gui_color_t status_color;

    if (noise_db <= 70)
    {
        // OK state: <=70dB, green
        status_text = "OK";
        icon_path = "/app_noise/Noise_ok_icon.bin";
        status_color.color.rgba.r = 0x71;
        status_color.color.rgba.g = 0xE1;
        status_color.color.rgba.b = 0x68;
        status_color.color.rgba.a = 0xFF;
    }
    else
    {
        // LOUD state: >70dB, orange
        status_text = "LOUD";
        icon_path = "/app_noise/Noise_warning_icon.bin";
        status_color.color.rgba.r = 0x00;
        status_color.color.rgba.g = 0xAD;
        status_color.color.rgba.b = 0xF4;
        status_color.color.rgba.a = 0xFF;
    }

    // Update the status text
    gui_text_content_set((gui_text_t *)app_noise_ok_text, (char *)status_text, strlen(status_text));
    gui_text_color_set((gui_text_t *)app_noise_ok_text, status_color);

    // Update the status icon
    if (hg_image_1769156756841_h11r != NULL)
    {
        gui_img_t *icon_img = (gui_img_t *)hg_image_1769156756841_h11r;
        // Use the new API instead of the deprecated gui_img_set_attribute
        gui_img_set_src(icon_img, (const uint8_t *)icon_path, IMG_SRC_FILESYS);
        gui_img_refresh_size(icon_img);
    }
}

/**
 * Set the noise level (for external calls)
 */
void app_noise_set_level(int noise_db)
{
    if (noise_db >= 30 && noise_db <= 120)
    {
        target_noise_level = noise_db;
    }
}

/**
 * Get the current noise level
 */
int app_noise_get_level(void)
{
    return current_noise_level;
}
