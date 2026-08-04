/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_intercom_user.h"
#include "../ui/app_intercom_ui.h"
#include "gui_list.h"
#include "app_intercom_callbacks.h"

#ifndef CONFIG_WALKIE_TALKIE
#define CONFIG_WALKIE_TALKIE  0
#endif

#if CONFIG_WALKIE_TALKIE
#include "bridge_intercom.h"
#include "walkie_talkie_app.h"
#endif

/* Intercom state flags */
static bool intercom_toggle_state = false;
static bool intercom_muted = false;
static bool intercom_talking = false;
static bool intercom_receiving = false;
static int waveform_frame_index = 0;

/* Intercom dev info */
#if CONFIG_WALKIE_TALKIE
static T_WALKIE_TALKIE_DEV *intercom_dev_info;
#endif

/*
 * Helper: show/hide list notes by index range.
 * Iterates children of the given list, casts each to gui_list_note_t,
 * and sets visibility for notes whose index is within [start_idx, end_idx].
 */
static void set_list_items_visible(gui_obj_t *list, int start_idx, int end_idx, bool visible)
{
    gui_node_list_t *node;
    gui_list_for_each(node, &list->child_list)
    {
        gui_list_note_t *note = (gui_list_note_t *)gui_list_entry(node, gui_obj_t, brother_list);
        if (note->index >= start_idx && note->index <= end_idx)
        {
            gui_obj_hidden((gui_obj_t *)note, !visible);
        }
    }
}

/**
 * intercom_toggle_on: Show device list (toggle_list items index 1~4)
 * Called by hg_button onCallback when toggled ON.
 */
void intercom_toggle_on(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    intercom_toggle_state = true;

    gui_obj_hidden((gui_obj_t *)available_devices_label, false);

#if CONFIG_WALKIE_TALKIE
    intercom_gui_to_app(EVENT_BUS_TOPIC_INTERCOM_GUI_ON, NULL, 0);
#endif

    gui_fb_change();
}

/**
 * intercom_toggle_off: Hide device list (toggle_list items index 1~4)
 * Called by hg_button offCallback when toggled OFF.
 */
void intercom_toggle_off(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    intercom_toggle_state = false;

    gui_obj_hidden((gui_obj_t *)toggle_list, true);

    gui_obj_hidden((gui_obj_t *)available_devices_label, true);

    gui_fb_change();

#if CONFIG_WALKIE_TALKIE
    intercom_gui_to_app(EVENT_BUS_TOPIC_INTERCOM_GUI_OFF, NULL, 0);
#endif
}

void intercom_update_scan_result(gui_obj_t *obj, const char *topic, void *data, uint16_t len)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(topic);
    GUI_UNUSED(data);
    GUI_UNUSED(len);
#if CONFIG_WALKIE_TALKIE
    intercom_dev_info = (T_WALKIE_TALKIE_DEV *)data;

    /* Expand scroll range to show all 5 items */
    gui_list_set_note_num(toggle_list, intercom_dev_info->dev_cnt);

    /* Show items 0~2 */
    set_list_items_visible((gui_obj_t *)toggle_list, 0, intercom_dev_info->dev_cnt - 1, true);

    gui_obj_hidden((gui_obj_t *)toggle_list, false);
#endif
}

void walkie_talkie_list_note_design(gui_obj_t *obj, void *param)
{
    GUI_UNUSED(param);
    // Cast obj to gui_list_note_t * type
    gui_list_note_t *note = (gui_list_note_t *)obj;
    uint16_t index = note->index;
    GUI_UNUSED(index);

    // Create different list_item content based on index
    switch (index)
    {
    case 0:
        {
            // Create device1_item_bg (hg_rect)
            device1_item_bg = gui_rect_create((gui_obj_t *)note, "device1_item_bg", 15, 1, 380, 84, 16,
                                              gui_rgb(30, 30, 30));
            // Create device1_name_label (hg_label)
            device1_name_label = gui_text_create((gui_obj_t *)note, "device1_name_label", 78, 27, 200, 50);
#if CONFIG_WALKIE_TALKIE
            gui_text_set((gui_text_t *)device1_name_label, (char *)intercom_dev_info->dev_info[0].dev_name,
                         GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 12, 36);
#else
            gui_text_set((gui_text_t *)device1_name_label, "Alex's Watch", GUI_FONT_SRC_BMP, gui_rgb(242, 242,
                         242), 12, 36);
#endif
            gui_text_type_set((gui_text_t *)device1_name_label,
                              "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)device1_name_label, MID_LEFT);
            // Create device1_status_dot (hg_image)
            device1_status_dot = gui_img_create_from_fs((gui_obj_t *)note, "device1_status_dot",
                                                        "/app_intercom/status_dot.bin", 58, 38, 10, 10);
            gui_img_scale((gui_img_t *)device1_status_dot, 1.250000f, 1.250000f);
            gui_obj_add_event_cb(obj, device1_item_bg_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 1:
        {
            // Create device2_item_bg (hg_rect)
            device2_item_bg = gui_rect_create((gui_obj_t *)note, "device2_item_bg", 15, 0, 380, 84, 16,
                                              gui_rgb(30, 30, 30));
            // Create device2_name_label (hg_label)
            device2_name_label = gui_text_create((gui_obj_t *)note, "device2_name_label", 78, 25, 300, 50);
#if CONFIG_WALKIE_TALKIE
            gui_text_set((gui_text_t *)device2_name_label, (char *)intercom_dev_info->dev_info[1].dev_name,
                         GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 14, 36);
#else
            gui_text_set((gui_text_t *)device2_name_label, "Jordan's Watch", GUI_FONT_SRC_BMP, gui_rgb(242, 242,
                         242), 14, 36);
#endif
            gui_text_type_set((gui_text_t *)device2_name_label,
                              "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)device2_name_label, MID_LEFT);
            // Create device2_status_dot (hg_image)
            device2_status_dot = gui_img_create_from_fs((gui_obj_t *)note, "device2_status_dot",
                                                        "/app_intercom/status_dot.bin", 59, 42, 10, 10);
            gui_img_scale((gui_img_t *)device2_status_dot, 1.250000f, 1.250000f);
            gui_obj_add_event_cb(obj, device2_item_bg_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 2:
        {
            // Create device3_item_bg (hg_rect)
            device3_item_bg = gui_rect_create((gui_obj_t *)note, "device3_item_bg", 15, 0, 380, 84, 16,
                                              gui_rgb(30, 30, 30));
            // Create device3_name_label (hg_label)
            device3_name_label = gui_text_create((gui_obj_t *)note, "device3_name_label", 78, 25, 200, 50);
#if CONFIG_WALKIE_TALKIE
            gui_text_set((gui_text_t *)device3_name_label, (char *)intercom_dev_info->dev_info[2].dev_name,
                         GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 11, 36);
#else
            gui_text_set((gui_text_t *)device3_name_label, "Sam's Watch", GUI_FONT_SRC_BMP, gui_rgb(242, 242,
                         242), 11, 36);
#endif
            gui_text_type_set((gui_text_t *)device3_name_label,
                              "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)device3_name_label, MID_LEFT);
            // Create device3_status_dot (hg_image)
            device3_status_dot = gui_img_create_from_fs((gui_obj_t *)note, "device3_status_dot",
                                                        "/app_intercom/status_dot.bin", 56, 40, 10, 10);
            gui_img_scale((gui_img_t *)device3_status_dot, 1.250000f, 1.250000f);
            gui_obj_add_event_cb(obj, device3_item_bg_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    default:
        break;
    }
}

/**
 * intercom_connect_dev: Connect to selected device (index 0~2 → Dev1~Dev3).
 * Calls walkie_talkie_app_connect_dev with the corresponding device index.
 */
void intercom_connect_dev(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_list_note_t *note = (gui_list_note_t *)obj;
    uint16_t index = note->index;
    gui_log("Connect to device index: %d", index);
#if CONFIG_WALKIE_TALKIE
    switch (index)
    {
    case 0:
        intercom_gui_to_app(EVENT_BUS_TOPIC_INTERCOM_GUI_CONNECT_DEV1, NULL, 0);
        break;
    case 1:
        intercom_gui_to_app(EVENT_BUS_TOPIC_INTERCOM_GUI_CONNECT_DEV2, NULL, 0);
        break;
    case 2:
        intercom_gui_to_app(EVENT_BUS_TOPIC_INTERCOM_GUI_CONNECT_DEV3, NULL, 0);
        break;
    }
#endif
}

void intercom_update_connect_status(gui_obj_t *obj, const char *topic, void *data, uint16_t len)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(data);
    GUI_UNUSED(len);

    if (strcmp(topic, "walkie_talkie_conn") == 0)
    {
        gui_text_content_set((gui_text_t *)connection_label, "Connected", 9);
    }
    else if (strcmp(topic, "walkie_talkie_disconn") == 0)
    {
        gui_text_content_set((gui_text_t *)connection_label, "Disconnected", 11);
    }

    gui_fb_change();
}

void intercom_update_user_name(gui_obj_t *obj, const char *topic, void *data, uint16_t len)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(topic);
    GUI_UNUSED(data);
    GUI_UNUSED(len);

#if CONFIG_WALKIE_TALKIE
    T_WALKIE_TALKIE_DEV *dev_info = (T_WALKIE_TALKIE_DEV *)data;
    gui_text_set((gui_text_t *)intercom_device_name_label, dev_info->connected_dev.dev_name,
                 GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 12, 36);
#endif
}

void intercom_update_receive_status(gui_obj_t *obj, const char *topic, void *data, uint16_t len)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(topic);
    GUI_UNUSED(data);
    GUI_UNUSED(len);

    if (strcmp(topic, "walkie_talkie_receive_start") == 0)
    {
        intercom_receiving = true;
        //receive_timer_count = 0; // Reset receive timer count when receiving starts

        if (waveform_image != NULL)
        {
            gui_obj_create_timer((gui_obj_t *)waveform_image, 50, true, receive_timer_cb);
            gui_obj_start_timer((gui_obj_t *)waveform_image);
        }

        waveform_frame_index = 0;

        /* RX-1: Talk button receiving (green) */
        gui_img_set_src((gui_img_t *)talk_btn, (const uint8_t *)"/app_intercom/talk_btn_receiving.bin",
                        IMG_SRC_FILESYS);
        gui_img_refresh_size((gui_img_t *)talk_btn);

        /* RX-3: Status text */
        gui_text_content_set((gui_text_t *)status_text_label, "Receiving...", 12);
    }
    else if (strcmp(topic, "walkie_talkie_receive_stop") == 0)
    {
        intercom_receiving = false;
        // receive_timer_count = 0; // Reset receive timer count when receiving stops

        if (waveform_image != NULL)
        {
            gui_obj_stop_timer((gui_obj_t *)waveform_image);
        }

        /* RX-4: Talk button back to normal */
        gui_img_set_src((gui_img_t *)talk_btn, (const uint8_t *)"/app_intercom/talk_btn_normal.bin",
                        IMG_SRC_FILESYS);
        gui_img_refresh_size((gui_img_t *)talk_btn);

        /* RX-5: Waveform back to idle */
        gui_img_set_src((gui_img_t *)waveform_image,
                        (const uint8_t *)"/app_intercom/waveform/idle/idle_frame_00.bin",
                        IMG_SRC_FILESYS);
        gui_img_refresh_size((gui_img_t *)waveform_image);

        /* RX-6: Status text */
        gui_text_content_set((gui_text_t *)status_text_label, "Hold to Talk", 12);
    }
}

/**
 * talk_btn_press: Press-to-hold start talking.
 * Lazily creates waveform animation timer on first call.
 * TK-1~TK-3: Switch to transmitting state.
 */
void talk_btn_press(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    if (intercom_receiving)
    {
        return;
    }
    /* Lazy-create waveform animation timer (50ms recurring) */
    if (waveform_image != NULL)
    {
        gui_obj_create_timer((gui_obj_t *)waveform_image, 50, true, talking_timer_cb);
        gui_obj_start_timer((gui_obj_t *)waveform_image);
    }

    intercom_talking = true;

    /* TK-1: Talk button active (cyan) */
    gui_img_set_src((gui_img_t *)talk_btn, (const uint8_t *)"/app_intercom/talk_btn_active.bin",
                    IMG_SRC_FILESYS);
    gui_img_refresh_size((gui_img_t *)talk_btn);

    /* TK-2: Switch waveform to transmitting frame 0 */
    waveform_frame_index = 0;
    gui_img_set_src((gui_img_t *)waveform_image,
                    (const uint8_t *)"/app_intercom/waveform/transmitting/transmitting_frame_00.bin",
                    IMG_SRC_FILESYS);
    gui_img_refresh_size((gui_img_t *)waveform_image);

    /* TK-3: Update status text */
    gui_text_content_set((gui_text_t *)status_text_label, "Transmitting...", 15);

    gui_fb_change();

#if CONFIG_WALKIE_TALKIE
    intercom_gui_to_app(EVENT_BUS_TOPIC_INTERCOM_TRANSMIT_START, NULL, 0);
#endif
}

/**
 * talk_btn_release: Release to stop talking.
 * TK-4~TK-6: Switch to idle state.
 */
void talk_btn_release(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    if (intercom_receiving)
    {
        return;
    }

    intercom_talking = false;

    if (waveform_image != NULL)
    {
        gui_obj_stop_timer((gui_obj_t *)waveform_image);
    }

    /* TK-4: Talk button normal (idle) */
    gui_img_set_src((gui_img_t *)talk_btn, (const uint8_t *)"/app_intercom/talk_btn_normal.bin",
                    IMG_SRC_FILESYS);
    gui_img_refresh_size((gui_img_t *)talk_btn);

    /* TK-5: Switch waveform to idle frame 0 */
    waveform_frame_index = 0;
    gui_img_set_src((gui_img_t *)waveform_image,
                    (const uint8_t *)"/app_intercom/waveform/idle/idle_frame_00.bin",
                    IMG_SRC_FILESYS);
    gui_img_refresh_size((gui_img_t *)waveform_image);

    /* TK-6: Update status text */
    gui_text_content_set((gui_text_t *)status_text_label, "Hold to Talk", 12);

    gui_fb_change();

#if CONFIG_WALKIE_TALKIE
    intercom_gui_to_app(EVENT_BUS_TOPIC_INTERCOM_TRANSMIT_STOP, NULL, 0);
#endif
}


/**
 * mute_btn_on: Mute enabled (hg_button handles image switching).
 * MT-1: Set muted flag, suppress receiving waveform.
 */
void mute_btn_on(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    intercom_muted = true;
    gui_fb_change();
}

/**
 * mute_btn_off: Mute disabled (hg_button handles image switching).
 * MT-3: Clear muted flag, allow receiving waveform.
 */
void mute_btn_off(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    intercom_muted = false;
    gui_fb_change();
}

/**
 * intercom_disconnect: Disconnect from connected device.
 * Called when intercom_talk_back_win is clicked.
 */
void intercom_disconnect(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

#if CONFIG_WALKIE_TALKIE
    intercom_gui_to_app(EVENT_BUS_TOPIC_INTERCOM_GUI_DISCONNECT, NULL, 0);
#endif
}


void talking_timer_cb(void *obj)
{
    GUI_UNUSED(obj);

    char frame_path[64];

    /* If user is talking, skip receive simulation and just animate transmitting waveform */
    if (intercom_talking)
    {
        waveform_frame_index = (waveform_frame_index + 1) % 30;
        snprintf(frame_path, sizeof(frame_path),
                 "/app_intercom/waveform/transmitting/transmitting_frame_%02d.bin",
                 waveform_frame_index);
        gui_img_set_src((gui_img_t *)waveform_image, (const uint8_t *)frame_path, IMG_SRC_FILESYS);
        gui_img_refresh_size((gui_img_t *)waveform_image);
        gui_fb_change();
    }
}

/**
 * receive_timer_cb: Simulate incoming voice (called every 100ms by timer)
 * - Count to 60 (6s): start receiving → RX-1~RX-3
 * - Count to 80 (8s = 6+2): stop receiving → RX-4~RX-6, reset counter
 * - Also handles waveform frame animation cycling
 */
void receive_timer_cb(void *obj)
{
    GUI_UNUSED(obj);

    char frame_path[64];

    if (intercom_receiving)
    {
        /* Animate receiving waveform (if not muted: MT-2) */
        if (!intercom_muted)
        {
            waveform_frame_index = (waveform_frame_index + 1) % 30;
            snprintf(frame_path, sizeof(frame_path),
                     "/app_intercom/waveform/receiving/receiving_frame_%02d.bin",
                     waveform_frame_index);
            gui_img_set_src((gui_img_t *)waveform_image, (const uint8_t *)frame_path, IMG_SRC_FILESYS);
            gui_img_refresh_size((gui_img_t *)waveform_image);
        }
    }

    gui_fb_change();
}
