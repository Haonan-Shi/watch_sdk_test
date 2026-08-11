/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_control_center_user.h"
#include "../ui/app_control_center_ui.h"
#include "../callbacks/app_control_center_callbacks.h"
#include "gui_list.h"
#include "gui_text.h"
#include "gui_rect.h"
#include "gui_img.h"
#include "gui_fb.h"
#include "gui_obj_tree.h"
#include <string.h>
#include <stdio.h>

#ifndef _HONEYGUI_SIMULATOR_
#include "app_bond.h"
#include "app_cfg.h"
#include "app_msg.h"
#include "app_task.h"
#include "app_main.h"
#include "app_gap.h"
#include "app_link_util.h"
#include "gap.h"
#include "app_bt_policy_api.h"
#include "bridge_bt_control.h"
#else
/* Mock types for simulator */
typedef struct
{
    uint8_t exist_addr_flag;
    uint8_t used;
    uint8_t device_name_len;
    uint8_t device_name[32];
    uint8_t bd_addr[6];
} T_APP_BOND_DEVICE;
#endif

/* Search device info (for bt_search_list) */
#define MAX_SEARCH_DEVICES 2

/* Pending device index for remove operation (0xFF = none) */
static uint8_t pending_remove_device_index = 0xFF;

#ifndef _HONEYGUI_SIMULATOR_
static char search_addr_buffer[MAX_SEARCH_DEVICES][18];  /* For bt_search_list address display */
#endif

/* Bluetooth address buffer for settings view */
static char bt_addr_buffer[18];

static uint8_t found_device_count = 0;

/* T_SEARCH_RESULT - must match definition in app_gap.c */
typedef struct
{
    uint8_t bd_addr[6];
    uint8_t nam_len;
    uint16_t device_name[25];  // UTF-16 encoded device name
    uint32_t cod;
} T_SEARCH_RESULT;

static T_SEARCH_RESULT found_devices[MAX_SEARCH_DEVICES];

/* ==================== Simulator Mock Data ==================== */
#ifdef _HONEYGUI_SIMULATOR_

/* Mock phone: iPhone 15 Pro, Not Connected */
static const char *sim_phone_name = "iPhone 15 Pro";

/* Mock 7 headphones (match screenshot) */
typedef struct
{
    const char *name;
    bool connected;
} sim_headphone_t;

#define SIM_HEADPHONE_COUNT 7
static const sim_headphone_t sim_headphones[SIM_HEADPHONE_COUNT] =
{
    { "AirPods Pro",        true  },  /* index 0: Connected */
    { "Sony WH-1000",       false },
    { "Galaxy Buds2",       false },
    { "Beats Studio3",      false },
    { "Jabra Elite 85h",    false },
    { "Bose QC35 II",       false },
    { "Sennheiser HD 450B", false },
};

/* Mock 2 search results: JBL Tune 510BT / Bose QC45 */
static const char *sim_search_names[MAX_SEARCH_DEVICES] =
{
    "JBL Tune 510BT",
    "Bose QC45",
};

/* Mock MAC addresses (non-zero, required for has_addr check) */
static const uint8_t sim_search_addrs[MAX_SEARCH_DEVICES][6] =
{
    { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 },
    { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF },
};

#endif /* _HONEYGUI_SIMULATOR_ */

/*
 * Helper: show/hide list notes by index range.
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

/*
 * Helper: get list note by index. Returns NULL if not found.
 */
static gui_list_note_t *get_list_note_by_index(gui_obj_t *list, uint16_t index)
{
    gui_node_list_t *node;
    gui_list_for_each(node, &list->child_list)
    {
        gui_list_note_t *note = (gui_list_note_t *)gui_list_entry(node, gui_obj_t, brother_list);
        if (note->index == index)
        {
            return note;
        }
    }
    return NULL;
}

/* Send BT toggle message to app task */
static void send_bt_toggle_msg_to_app(bool enable)
{
#ifndef _HONEYGUI_SIMULATOR_
    T_BT_CONTROL_TOGGLE_DATA toggle = { .enable = enable ? 1u : 0u };
    bt_control_gui_to_app(EVENT_BUS_TOPIC_BT_CONTROL_CMD_TOGGLE,
                          &toggle, sizeof(toggle));

    /* If BT enable and have bonded phone, reconnect to phone */
    if (enable)
    {
        const T_BT_CONTROL_DEVICE *phone = &bridge_bt_control_get_state()->bonded[0];
        if (phone->exist)
        {
            T_BT_CONTROL_ADDR_DATA addr;
            memcpy(addr.bd_addr, phone->bd_addr, 6);
            bt_control_gui_to_app(EVENT_BUS_TOPIC_BT_CONTROL_CMD_CONNECT_PHONE,
                                  &addr, sizeof(addr));
        }
    }
#else
    GUI_UNUSED(enable);
#endif
}

/**
 * @brief Helper function to connect or disconnect a headphone (for bond list)
 * @param index Headphone index (1-7), corresponds to bond_device[1-7]
 */
static void headphone_connect_or_disconnect(uint8_t index)
{
#ifndef _HONEYGUI_SIMULATOR_
    if (index < 1 || index > 7) { return; }

    const T_BT_CONTROL_STATE  *bt_state = bridge_bt_control_get_state();
    const T_BT_CONTROL_DEVICE *bond     = &bt_state->bonded[index];
    if (!bond->exist) { return; }

    T_BT_CONTROL_ADDR_DATA addr;
    memcpy(addr.bd_addr, bond->bd_addr, 6);

    if (bond->connected)
    {
        /* Currently connected → user wants to disconnect it. */
        bt_control_gui_to_app(EVENT_BUS_TOPIC_BT_CONTROL_CMD_DISCONNECT,
                              &addr, sizeof(addr));
    }
    else
    {
        /* Bridge decides whether a swap is needed (another active earphone)
         * and orchestrates disconnect-then-connect across views. */
        bt_control_gui_to_app(EVENT_BUS_TOPIC_BT_CONTROL_CMD_SWAP_TO,
                              &addr, sizeof(addr));
    }
#else
    GUI_UNUSED(index);
#endif
}

#ifndef _HONEYGUI_SIMULATOR_
/**
 * @brief Common connect logic for headphones found via search.
 * @param index Index into found_devices[] (0 ~ MAX_SEARCH_DEVICES-1)
 */
static void headphone_search_connect_common(uint8_t index)
{
    if (index >= MAX_SEARCH_DEVICES) { return; }

    uint8_t *addr = found_devices[index].bd_addr;

    /* Validate address (non-zero) */
    bool has_addr = false;
    for (int i = 0; i < 6; i++)
    {
        if (addr[i] != 0) { has_addr = true; break; }
    }
    if (!has_addr) { return; }

    /* Stop inquiry first if running */
    if (get_search_status() == SEARCH_START)
    {
        app_bt_bond_temp_cache_save_to_search();
        bt_control_gui_to_app(EVENT_BUS_TOPIC_BT_CONTROL_CMD_INQUIRY_STOP, NULL, 0);
    }

    /* Bridge handles swap orchestration; works across view boundaries because
     * the pending state lives in the bridge, not in any view's widget tree. */
    T_BT_CONTROL_ADDR_DATA cmd_addr;
    memcpy(cmd_addr.bd_addr, addr, 6);
    bt_control_gui_to_app(EVENT_BUS_TOPIC_BT_CONTROL_CMD_SWAP_TO,
                          &cmd_addr, sizeof(cmd_addr));
}
#endif

void bluetooth_toggle_on(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    /* Show headphones entry and phone section widgets */
    if (bt_headphones_entry_bg != NULL)
    {
        gui_obj_hidden((gui_obj_t *)bt_headphones_entry_bg, false);
    }
    if (headphones_entry_icon != NULL)
    {
        gui_obj_hidden((gui_obj_t *)headphones_entry_icon, false);
    }
    if (headphones_entry_label != NULL)
    {
        gui_obj_hidden((gui_obj_t *)headphones_entry_label, false);
    }
    if (phone_section_label != NULL)
    {
        gui_obj_hidden((gui_obj_t *)phone_section_label, false);
    }

    /* bt_list note structure:
     * index 0: phone item (show if phone bonded) */
    uint16_t note_count = 0;

#ifndef _HONEYGUI_SIMULATOR_
    if (bridge_bt_control_get_state()->bonded[0].exist)
    {
        note_count = 1;  /* Show phone item */
    }
#else
    /* Simulator: always show 1 phone (iPhone 15 Pro) */
    note_count = 1;
#endif

    /* Show bt_list first, then set note count */
    gui_obj_hidden((gui_obj_t *)bt_list, false);
    gui_list_set_note_num(bt_list, note_count);

    /* Send BT toggle on message to app task */
    send_bt_toggle_msg_to_app(true);

    gui_fb_change();
}

void bluetooth_toggle_off(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    /* Send BT toggle off message to app task */
    send_bt_toggle_msg_to_app(false);

    /* Hide headphones entry and phone section widgets */
    if (bt_headphones_entry_bg != NULL)
    {
        gui_obj_hidden((gui_obj_t *)bt_headphones_entry_bg, true);
    }
    if (headphones_entry_icon != NULL)
    {
        gui_obj_hidden((gui_obj_t *)headphones_entry_icon, true);
    }
    if (headphones_entry_label != NULL)
    {
        gui_obj_hidden((gui_obj_t *)headphones_entry_label, true);
    }
    if (phone_section_label != NULL)
    {
        gui_obj_hidden((gui_obj_t *)phone_section_label, true);
    }

    /* Hide bt_list and set note count to 0 */
    gui_obj_hidden((gui_obj_t *)bt_list, true);
    gui_list_set_note_num(bt_list, 0);
    gui_list_set_offset(bt_list, 0);

    gui_fb_change();
}

void bluetooth_search_devices(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

#ifndef _HONEYGUI_SIMULATOR_
    /* Clear stale results from a previous inquiry that may not have reached
     * inquiry_cmpl (e.g. user left the view mid-scan). */
    found_device_count = 0;
    memset(found_devices, 0, sizeof(found_devices));
    if (bt_search_list != NULL)
    {
        gui_obj_hidden((gui_obj_t *)bt_search_list, true);
        gui_list_set_note_num(bt_search_list, 0);
    }

    /* Start Bluetooth search via bridge cmd */
    bt_control_gui_to_app(EVENT_BUS_TOPIC_BT_CONTROL_CMD_INQUIRY_START, NULL, 0);
#else
    /* Simulator: populate 2 mock found devices */
    found_device_count = MAX_SEARCH_DEVICES;
    for (uint8_t i = 0; i < MAX_SEARCH_DEVICES; i++)
    {
        memset(&found_devices[i], 0, sizeof(T_SEARCH_RESULT));
        memcpy(found_devices[i].bd_addr, sim_search_addrs[i], 6);
        found_devices[i].nam_len = 0;  /* UTF-16 name not used; sim name handled in note_design */
    }
    if (bt_search_list != NULL)
    {
        gui_obj_hidden((gui_obj_t *)bt_search_list, false);
        gui_list_set_note_num(bt_search_list, found_device_count);
    }
#endif

    gui_fb_change();
}

void wifi_toggle_on(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    /* Expand scroll range to 3 items */
    gui_list_set_note_num(wifi_list, 3);

    /* Show wifi_list items index 1~2: saved networks section + network item */
    set_list_items_visible((gui_obj_t *)wifi_list, 1, 2, true);

    gui_fb_change();
}

void wifi_toggle_off(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    /* Hide wifi_list items index 1~2 */
    set_list_items_visible((gui_obj_t *)wifi_list, 1, 2, false);

    /* Shrink scroll range to only toggle item, reset scroll position */
    gui_list_set_note_num(wifi_list, 1);
    gui_list_set_offset(wifi_list, 0);

    gui_fb_change();
}

void phone_linkback_and_disconnect(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

#ifndef _HONEYGUI_SIMULATOR_
    const T_BT_CONTROL_DEVICE *phone = &bridge_bt_control_get_state()->bonded[0];
    if (!phone->exist) { return; }

    T_BT_CONTROL_ADDR_DATA addr;
    memcpy(addr.bd_addr, phone->bd_addr, 6);

    const char *topic = phone->connected ? EVENT_BUS_TOPIC_BT_CONTROL_CMD_DISCONNECT
                        : EVENT_BUS_TOPIC_BT_CONTROL_CMD_CONNECT_PHONE;
    bt_control_gui_to_app(topic, &addr, sizeof(addr));
#endif

    gui_fb_change();
}

void headphone1_linkback_and_disconnect(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj); GUI_UNUSED(e);
    headphone_connect_or_disconnect(1);
    gui_fb_change();
}
void headphone2_linkback_and_disconnect(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj); GUI_UNUSED(e);
    headphone_connect_or_disconnect(2);
    gui_fb_change();
}
void headphone3_linkback_and_disconnect(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj); GUI_UNUSED(e);
    headphone_connect_or_disconnect(3);
    gui_fb_change();
}
void headphone4_linkback_and_disconnect(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj); GUI_UNUSED(e);
    headphone_connect_or_disconnect(4);
    gui_fb_change();
}
void headphone5_linkback_and_disconnect(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj); GUI_UNUSED(e);
    headphone_connect_or_disconnect(5);
    gui_fb_change();
}
void headphone6_linkback_and_disconnect(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj); GUI_UNUSED(e);
    headphone_connect_or_disconnect(6);
    gui_fb_change();
}
void headphone7_linkback_and_disconnect(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj); GUI_UNUSED(e);
    headphone_connect_or_disconnect(7);
    gui_fb_change();
}

void phone_remove_paired_device(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj); GUI_UNUSED(e);
    pending_remove_device_index = 0;
    if (unbind_confirm_bg != NULL) { gui_obj_hidden((gui_obj_t *)unbind_confirm_bg, false); }
    if (unbind_cancel_bg != NULL)  { gui_obj_hidden((gui_obj_t *)unbind_cancel_bg,  false); }
    gui_fb_change();
}
void headphone1_remove_paired_device(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj); GUI_UNUSED(e);
    pending_remove_device_index = 1;
    if (unbind_confirm_bg != NULL) { gui_obj_hidden((gui_obj_t *)unbind_confirm_bg, false); }
    if (unbind_cancel_bg != NULL)  { gui_obj_hidden((gui_obj_t *)unbind_cancel_bg,  false); }
    gui_fb_change();
}
void headphone2_remove_paired_device(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj); GUI_UNUSED(e);
    pending_remove_device_index = 2;
    if (unbind_confirm_bg != NULL) { gui_obj_hidden((gui_obj_t *)unbind_confirm_bg, false); }
    if (unbind_cancel_bg != NULL)  { gui_obj_hidden((gui_obj_t *)unbind_cancel_bg,  false); }
    gui_fb_change();
}
void headphone3_remove_paired_device(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj); GUI_UNUSED(e);
    pending_remove_device_index = 3;
    if (unbind_confirm_bg != NULL) { gui_obj_hidden((gui_obj_t *)unbind_confirm_bg, false); }
    if (unbind_cancel_bg != NULL)  { gui_obj_hidden((gui_obj_t *)unbind_cancel_bg,  false); }
    gui_fb_change();
}
void headphone4_remove_paired_device(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj); GUI_UNUSED(e);
    pending_remove_device_index = 4;
    if (unbind_confirm_bg != NULL) { gui_obj_hidden((gui_obj_t *)unbind_confirm_bg, false); }
    if (unbind_cancel_bg != NULL)  { gui_obj_hidden((gui_obj_t *)unbind_cancel_bg,  false); }
    gui_fb_change();
}
void headphone5_remove_paired_device(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj); GUI_UNUSED(e);
    pending_remove_device_index = 5;
    if (unbind_confirm_bg != NULL) { gui_obj_hidden((gui_obj_t *)unbind_confirm_bg, false); }
    if (unbind_cancel_bg != NULL)  { gui_obj_hidden((gui_obj_t *)unbind_cancel_bg,  false); }
    gui_fb_change();
}
void headphone6_remove_paired_device(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj); GUI_UNUSED(e);
    pending_remove_device_index = 6;
    if (unbind_confirm_bg != NULL) { gui_obj_hidden((gui_obj_t *)unbind_confirm_bg, false); }
    if (unbind_cancel_bg != NULL)  { gui_obj_hidden((gui_obj_t *)unbind_cancel_bg,  false); }
    gui_fb_change();
}
void headphone7_remove_paired_device(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj); GUI_UNUSED(e);
    pending_remove_device_index = 7;
    if (unbind_confirm_bg != NULL) { gui_obj_hidden((gui_obj_t *)unbind_confirm_bg, false); }
    if (unbind_cancel_bg != NULL)  { gui_obj_hidden((gui_obj_t *)unbind_cancel_bg,  false); }
    gui_fb_change();
}

void remove_paired_device_confirm(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj); GUI_UNUSED(e);

#ifndef _HONEYGUI_SIMULATOR_
    if (pending_remove_device_index <= 7)
    {
        T_BT_CONTROL_REMOVE_DATA remove = { .index = pending_remove_device_index };
        bt_control_gui_to_app(EVENT_BUS_TOPIC_BT_CONTROL_CMD_REMOVE_BOND,
                              &remove, sizeof(remove));
    }
#endif

    pending_remove_device_index = 0xFF;
    gui_fb_change();
}

void remove_paired_device_cancel(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj); GUI_UNUSED(e);
    pending_remove_device_index = 0xFF;
    gui_fb_change();
}

void headphone1_connect(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj); GUI_UNUSED(e);
#ifndef _HONEYGUI_SIMULATOR_
    headphone_search_connect_common(0);
#endif
    gui_fb_change();
}

void headphone2_connect(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj); GUI_UNUSED(e);
#ifndef _HONEYGUI_SIMULATOR_
    headphone_search_connect_common(1);
#endif
    gui_fb_change();
}

void update_phone_list(gui_obj_t *obj, const char *topic, void *data, uint16_t len)
{
    GUI_UNUSED(obj); GUI_UNUSED(topic);

#ifndef _HONEYGUI_SIMULATOR_
    const T_BT_CONTROL_STATE *s = (data != NULL && len >= sizeof(T_BT_CONTROL_STATE))
                                  ? (const T_BT_CONTROL_STATE *)data
                                  : bridge_bt_control_get_state();
    if (s->bonded[0].exist)
    {
        gui_obj_hidden((gui_obj_t *)bt_list, false);
        gui_list_set_note_num(bt_list, 1);

        /* note_design only re-runs when count changes; refresh status label
         * directly so connect/disconnect events update the visible text. */
        if (phone_status_label != NULL)
        {
            if (s->bonded[0].connected)
            {
                gui_text_content_set(phone_status_label, "Connected", 9);
                gui_text_color_set(phone_status_label, gui_rgb(76, 217, 100));
            }
            else
            {
                gui_text_content_set(phone_status_label, "Not Connected", 13);
                gui_text_color_set(phone_status_label, gui_rgb(102, 102, 102));
            }
        }
    }
    else
    {
        gui_list_set_note_num(bt_list, 0);
    }
#else
    GUI_UNUSED(data); GUI_UNUSED(len);
    /* Simulator: always show 1 phone item (iPhone 15 Pro) */
    gui_obj_hidden((gui_obj_t *)bt_list, false);
    gui_list_set_note_num(bt_list, 1);
#endif

    gui_fb_change();
}

void update_headphone_list(gui_obj_t *obj, const char *topic, void *data, uint16_t len)
{
    GUI_UNUSED(obj); GUI_UNUSED(topic);

    if (headphone_list != NULL)
    {
#ifndef _HONEYGUI_SIMULATOR_
        const T_BT_CONTROL_STATE *s = (data != NULL && len >= sizeof(T_BT_CONTROL_STATE))
                                      ? (const T_BT_CONTROL_STATE *)data
                                      : bridge_bt_control_get_state();
        gui_obj_hidden((gui_obj_t *)headphone_list, false);
        gui_list_set_note_num(headphone_list, s->headphone_count);

        /* Refresh each existing headphone status label / icon directly so
         * connect/disconnect events update visible widgets even when note
         * count is unchanged (note_design only re-runs on count change). */
        for (uint8_t slot = 1; slot < BT_CONTROL_MAX_BONDED; slot++)
        {
            const T_BT_CONTROL_DEVICE *hp = &s->bonded[slot];
            if (!hp->exist) { continue; }

            char widget_name[32];
            gui_obj_t *status_obj = NULL;
            gui_obj_t *icon_obj   = NULL;

            snprintf(widget_name, sizeof(widget_name),
                     "headphones%d_status_label", slot);
            status_obj = gui_obj_get_handle((gui_obj_t *)headphone_list, widget_name);
            snprintf(widget_name, sizeof(widget_name),
                     "headphones%d_icon", slot);
            icon_obj = gui_obj_get_handle((gui_obj_t *)headphone_list, widget_name);

            if (status_obj != NULL)
            {
                if (hp->connected)
                {
                    gui_text_content_set((gui_text_t *)status_obj, "Connected", 9);
                    gui_text_color_set((gui_text_t *)status_obj, gui_rgb(76, 217, 100));
                }
                else
                {
                    gui_text_content_set((gui_text_t *)status_obj, "Not Connected", 13);
                    gui_text_color_set((gui_text_t *)status_obj, gui_rgb(102, 102, 102));
                }
            }
            if (icon_obj != NULL)
            {
                gui_img_set_src((gui_img_t *)icon_obj,
                                (const uint8_t *)(hp->connected
                                                  ? "/app_control_center/headphones_icon_connected.bin"
                                                  : "/app_control_center/headphones_icon_disconnected.bin"),
                                IMG_SRC_FILESYS);
            }
        }
#else
        GUI_UNUSED(data); GUI_UNUSED(len);
        gui_obj_hidden((gui_obj_t *)headphone_list, false);
        gui_list_set_note_num(headphone_list, SIM_HEADPHONE_COUNT);
#endif
    }

    gui_fb_change();
}

void update_search_list(gui_obj_t *obj, const char *topic, void *data, uint16_t len)
{
    GUI_UNUSED(obj); GUI_UNUSED(data); GUI_UNUSED(len);

    if (strcmp(topic, "bt/inquiry_result") == 0)
    {
        T_SEARCH_RESULT *result = (T_SEARCH_RESULT *)data;

        if (result != NULL && found_device_count < MAX_SEARCH_DEVICES)
        {
            bool already_exists = false;
            for (uint8_t i = 0; i < found_device_count; i++)
            {
                if (memcmp(found_devices[i].bd_addr, result->bd_addr, 6) == 0)
                {
                    already_exists = true;
                    break;
                }
            }

            if (!already_exists)
            {
                memcpy(&found_devices[found_device_count], result, sizeof(T_SEARCH_RESULT));
                found_device_count++;
            }

            gui_obj_hidden((gui_obj_t *)bt_search_list, false);
            gui_list_set_note_num(bt_search_list, found_device_count);
        }
    }
    else if (strcmp(topic, "bt/inquiry_cmpl") == 0)
    {
        gui_obj_hidden((gui_obj_t *)bt_search_list, true);
        gui_list_set_note_num(bt_search_list, 0);
        found_device_count = 0;
    }

    gui_fb_change();
}

void bt_phone_list_note_design(gui_obj_t *obj, void *param)
{
    GUI_UNUSED(param);

    gui_list_note_t *note = (gui_list_note_t *)obj;
    uint16_t index = note->index;

    if (index != 0) { return; }

    /* Resolve data source */
    const char *phone_name_ascii = NULL;
    bool phone_connected = false;

#ifndef _HONEYGUI_SIMULATOR_
    const T_BT_CONTROL_DEVICE *phone_dev = &bridge_bt_control_get_state()->bonded[0];
    if (!phone_dev->exist) { return; }
    phone_connected = phone_dev->connected;
#else
    phone_name_ascii = sim_phone_name;  /* "iPhone 15 Pro" */
    phone_connected = false;            /* Not Connected */
#endif

    /* Skip recreate when widgets already exist; status text is kept in sync
     * by update_phone_list on every event_bus snapshot. */
    gui_obj_t *existing_status = NULL;
    gui_obj_t *existing_name = NULL;
    existing_status = gui_obj_get_handle((gui_obj_t *)note, "phone_status_label");
    existing_name = gui_obj_get_handle((gui_obj_t *)note, "phone_name_label");
    if (existing_status != NULL || existing_name != NULL)
    {
        if (existing_status != NULL) { phone_status_label = (gui_text_t *)existing_status; }
        if (existing_name   != NULL) { phone_name_label   = (gui_text_t *)existing_name;   }
        return;
    }

    /* Create phone_item_bg */
    phone_item_bg = gui_rect_create((gui_obj_t *)note, "phone_item_bg", 24, 0, 362, 84, 12,
                                    gui_rgb(44, 44, 46));
    gui_obj_add_event_cb(phone_item_bg, (gui_event_cb_t)phone_item_bg_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);
    gui_obj_add_event_cb(phone_item_bg, (gui_event_cb_t)phone_item_bg_long_pressed_cb,
                         GUI_EVENT_TOUCH_LONG, NULL);

    /* Create phone icon */
    phone_icon = gui_img_create_from_fs((gui_obj_t *)note, "phone_icon",
                                        "/app_control_center/smartphone_icon.bin",
                                        352, 28, 28, 28);

    /* Create phone_name_label */
    phone_name_label = gui_text_create((gui_obj_t *)note, "phone_name_label", 40, 19, 260, 50);
#ifndef _HONEYGUI_SIMULATOR_
    if (phone_dev->device_name_len > 0)
    {
        gui_text_encoding_set(phone_name_label, UTF_16);
        gui_text_set(phone_name_label, (char *)phone_dev->device_name,
                     GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                     phone_dev->device_name_len * 2, 36);
    }
    else
    {
        gui_text_set(phone_name_label, "Unknown Phone", GUI_FONT_SRC_BMP,
                     gui_rgb(255, 255, 255), 13, 36);
    }
#else
    gui_text_set(phone_name_label, (char *)phone_name_ascii, GUI_FONT_SRC_BMP,
                 gui_rgb(255, 255, 255), strlen(phone_name_ascii), 36);
#endif
    gui_text_type_set(phone_name_label,
                      "/font/NotoSansSC_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set(phone_name_label, LEFT);
    gui_obj_hidden((gui_obj_t *)phone_name_label, false);

    /* Create phone_status_label */
    phone_status_label = gui_text_create((gui_obj_t *)note, "phone_status_label", 40, 52, 200, 38);
    if (phone_connected)
    {
        gui_text_set(phone_status_label, "Connected", GUI_FONT_SRC_BMP,
                     gui_rgb(76, 217, 100), 9, 28);
    }
    else
    {
        gui_text_set(phone_status_label, "Not Connected", GUI_FONT_SRC_BMP,
                     gui_rgb(102, 102, 102), 13, 28);
    }
    gui_text_type_set(phone_status_label,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set(phone_status_label, LEFT);
    gui_obj_hidden((gui_obj_t *)phone_status_label, false);
}

void bt_headphone_list_note_design(gui_obj_t *obj, void *param)
{
    GUI_UNUSED(param);

    gui_list_note_t *note = (gui_list_note_t *)obj;
    uint16_t index = note->index;

    if (index >= 7) { return; }

    /* Resolve data source */
    const char *hp_name_ascii = NULL;
    bool hp_connected = false;

#ifndef _HONEYGUI_SIMULATOR_
    const T_BT_CONTROL_STATE  *bt_state    = bridge_bt_control_get_state();
    const T_BT_CONTROL_DEVICE *headphone_dev = &bt_state->bonded[index + 1];

    if (!headphone_dev->exist)
    {
        if (headphone_list != NULL)
        {
            gui_obj_hidden((gui_obj_t *)headphone_list, false);
            gui_list_set_note_num(headphone_list, bt_state->headphone_count);
        }
        return;
    }
    hp_connected = headphone_dev->connected;
#else
    if (index >= SIM_HEADPHONE_COUNT) { return; }
    hp_name_ascii = sim_headphones[index].name;
    hp_connected = sim_headphones[index].connected;
#endif

    /* Skip recreate when widgets already exist; status text/icon are kept in
     * sync by update_headphone_list on every event_bus snapshot. */
    gui_obj_t *existing_status = NULL;
    gui_obj_t *existing_name = NULL;
    char name_buf[32];
    snprintf(name_buf, sizeof(name_buf), "headphones%d_status_label", index + 1);
    existing_status = gui_obj_get_handle((gui_obj_t *)note, name_buf);
    snprintf(name_buf, sizeof(name_buf), "headphones%d_name_label", index + 1);
    existing_name = gui_obj_get_handle((gui_obj_t *)note, name_buf);
    if (existing_status != NULL || existing_name != NULL) { return; }

    /* Create background */
    char bg_name[32];
    snprintf(bg_name, sizeof(bg_name), "headphones_item%d_bg", index + 1);
    gui_rounded_rect_t *headphone_item_bg = gui_rect_create((gui_obj_t *)note, bg_name,
                                                            24, 0, 362, 84, 12,
                                                            gui_rgb(44, 44, 46));

    /* Add click/long press events based on index */
    gui_event_cb_t click_cb = NULL;
    gui_event_cb_t long_cb = NULL;
    switch (index)
    {
    case 0: click_cb = (gui_event_cb_t)headphones_item1_bg_clicked_cb;
        long_cb = (gui_event_cb_t)headphones_item1_bg_long_pressed_cb; break;
    case 1: click_cb = (gui_event_cb_t)headphones_item2_bg_clicked_cb;
        long_cb = (gui_event_cb_t)headphones_item2_bg_long_pressed_cb; break;
    case 2: click_cb = (gui_event_cb_t)headphones_item3_bg_clicked_cb;
        long_cb = (gui_event_cb_t)headphones_item3_bg_long_pressed_cb; break;
    case 3: click_cb = (gui_event_cb_t)headphones_item4_bg_clicked_cb;
        long_cb = (gui_event_cb_t)headphones_item4_bg_long_pressed_cb; break;
    case 4: click_cb = (gui_event_cb_t)headphones_item5_bg_clicked_cb;
        long_cb = (gui_event_cb_t)headphones_item5_bg_long_pressed_cb; break;
    case 5: click_cb = (gui_event_cb_t)headphones_item6_bg_clicked_cb;
        long_cb = (gui_event_cb_t)headphones_item6_bg_long_pressed_cb; break;
    case 6: click_cb = (gui_event_cb_t)headphones_item7_bg_clicked_cb;
        long_cb = (gui_event_cb_t)headphones_item7_bg_long_pressed_cb; break;
    default: break;
    }
    if (click_cb) { gui_obj_add_event_cb(headphone_item_bg, click_cb, GUI_EVENT_TOUCH_CLICKED, NULL); }
    if (long_cb)  { gui_obj_add_event_cb(headphone_item_bg, long_cb,  GUI_EVENT_TOUCH_LONG,    NULL); }

    /* Create icon */
    char icon_name[32];
    snprintf(icon_name, sizeof(icon_name), "headphones%d_icon", index + 1);
    gui_img_create_from_fs((gui_obj_t *)note, icon_name,
                           hp_connected ? "/app_control_center/headphones_icon_connected.bin"
                           : "/app_control_center/headphones_icon_disconnected.bin",
                           352, 28, 28, 28);

    /* Create name label */
    char name_label_name[32];
    snprintf(name_label_name, sizeof(name_label_name), "headphones%d_name_label", index + 1);
    gui_text_t *name_label = gui_text_create((gui_obj_t *)note, name_label_name, 40, 19, 260, 50);
#ifndef _HONEYGUI_SIMULATOR_
    if (headphone_dev->device_name_len > 0)
    {
        gui_text_encoding_set(name_label, UTF_16);
        gui_text_set(name_label, (char *)headphone_dev->device_name,
                     GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                     headphone_dev->device_name_len * 2, 36);
    }
    else
    {
        gui_text_set(name_label, "Unknown Headphone", GUI_FONT_SRC_BMP,
                     gui_rgb(255, 255, 255), 17, 36);
    }
#else
    gui_text_set(name_label, (char *)hp_name_ascii, GUI_FONT_SRC_BMP,
                 gui_rgb(255, 255, 255), strlen(hp_name_ascii), 36);
#endif
    gui_text_type_set(name_label,
                      "/font/NotoSansSC_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set(name_label, LEFT);
    gui_obj_hidden((gui_obj_t *)name_label, false);

    /* Create status label */
    char status_label_name[32];
    snprintf(status_label_name, sizeof(status_label_name), "headphones%d_status_label", index + 1);
    gui_text_t *status_label = gui_text_create((gui_obj_t *)note, status_label_name, 40, 52, 200, 38);
    if (hp_connected)
    {
        gui_text_set(status_label, "Connected", GUI_FONT_SRC_BMP,
                     gui_rgb(76, 217, 100), 9, 28);
    }
    else
    {
        gui_text_set(status_label, "Not Connected", GUI_FONT_SRC_BMP,
                     gui_rgb(102, 102, 102), 13, 28);
    }
    gui_text_type_set(status_label,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set(status_label, LEFT);
    gui_obj_hidden((gui_obj_t *)status_label, false);
}

void bt_search_list_note_design(gui_obj_t *obj, void *param)
{
    GUI_UNUSED(param);

    gui_list_note_t *note = (gui_list_note_t *)obj;
    uint16_t index = note->index;

    if (index >= MAX_SEARCH_DEVICES) { return; }

    T_SEARCH_RESULT *dev = &found_devices[index];

    /* Check if device has valid address */
    bool has_addr = false;
    for (int i = 0; i < 6; i++)
    {
        if (dev->bd_addr[i] != 0) { has_addr = true; break; }
    }
    if (!has_addr) { return; }

    // Create found_device_bg (hg_rect)
    gui_rounded_rect_t *device_bg = NULL;
    gui_text_t *name_label = NULL;
    gui_text_t *status_label = NULL;
    gui_img_t *icon = NULL;

    switch (index)
    {
    case 0:
        device_bg = gui_rect_create((gui_obj_t *)note, "found_device1_bg", 24, 0, 362, 84, 12,
                                    gui_rgb(44, 44, 46));
        found_device1_name = gui_text_create((gui_obj_t *)note, "found_device1_name", 40, 19, 260, 50);
        found_device1_status = gui_text_create((gui_obj_t *)note, "found_device1_status", 40, 52, 200, 38);
        icon = gui_img_create_from_fs((gui_obj_t *)note, "found_device1_icon",
                                      "/app_control_center/headphones_icon_disconnected.bin",
                                      352, 19, 18, 18);
        name_label = found_device1_name;
        status_label = found_device1_status;
        gui_obj_add_event_cb(device_bg, (gui_event_cb_t)found_device1_bg_clicked_cb,
                             GUI_EVENT_TOUCH_CLICKED, NULL);
        break;
    case 1:
        device_bg = gui_rect_create((gui_obj_t *)note, "found_device2_bg", 24, 0, 362, 84, 12,
                                    gui_rgb(44, 44, 46));
        found_device2_name = gui_text_create((gui_obj_t *)note, "found_device2_name", 40, 19, 260, 50);
        found_device2_status = gui_text_create((gui_obj_t *)note, "found_device2_status", 40, 52, 200, 38);
        icon = gui_img_create_from_fs((gui_obj_t *)note, "found_device2_icon",
                                      "/app_control_center/headphones_icon_disconnected.bin",
                                      352, 19, 18, 18);
        name_label = found_device2_name;
        status_label = found_device2_status;
        gui_obj_add_event_cb(device_bg, (gui_event_cb_t)found_device2_bg_clicked_cb,
                             GUI_EVENT_TOUCH_CLICKED, NULL);
        break;
    default:
        break;
    }

    // Set device name
    if (name_label != NULL)
    {
#ifdef _HONEYGUI_SIMULATOR_
        /* Simulator: use mock device name (JBL Tune 510BT / Bose QC45) */
        const char *sim_name = sim_search_names[index];
        gui_text_set(name_label, (char *)sim_name, GUI_FONT_SRC_BMP,
                     gui_rgb(255, 255, 255), strlen(sim_name), 36);
        gui_text_type_set(name_label,
                          "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin",
                          FONT_SRC_FILESYS);
#else
        if (dev->nam_len > 0)
        {
            gui_text_encoding_set(name_label, UTF_16);
            gui_text_set(name_label, (char *)dev->device_name,
                         GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), dev->nam_len * 2, 36);
        }
        else
        {
            /* Show BD address if no device name */
            char *addr_str = search_addr_buffer[index];
            snprintf(addr_str, 18, "%02X:%02X:%02X:%02X:%02X:%02X",
                     dev->bd_addr[5], dev->bd_addr[4], dev->bd_addr[3],
                     dev->bd_addr[2], dev->bd_addr[1], dev->bd_addr[0]);
            gui_text_set(name_label, addr_str, GUI_FONT_SRC_BMP,
                         gui_rgb(255, 255, 255), strlen(addr_str), 36);
            gui_text_type_set(name_label,
                              "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin",
                              FONT_SRC_FILESYS);
        }
#endif
        gui_text_mode_set(name_label, LEFT);
        gui_obj_hidden((gui_obj_t *)name_label, false);
    }

    // Set device status: Available
    if (status_label != NULL)
    {
        gui_text_set(status_label, "Available", GUI_FONT_SRC_BMP,
                     gui_rgb(102, 102, 102), 9, 28);
        gui_text_type_set(status_label,
                          "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin",
                          FONT_SRC_FILESYS);
        gui_text_mode_set(status_label, LEFT);
        gui_obj_hidden((gui_obj_t *)status_label, false);
    }

    // Set device icon
    if (icon != NULL)
    {
        gui_obj_hidden((gui_obj_t *)icon, false);
    }
}

void setting_list_note_design(gui_obj_t *obj, void *param)
{
    GUI_UNUSED(param);

    gui_list_note_t *note = (gui_list_note_t *)obj;
    uint16_t index = note->index;

    switch (index)
    {
    case 0:
        {
            // Create device_name_bg
            device_name_bg = gui_rect_create((gui_obj_t *)note, "device_name_bg", 24, 0, 362, 84, 12,
                                             gui_rgb(44, 44, 46));
            // Create device_name_label
            device_name_label = gui_text_create((gui_obj_t *)note, "device_name_label", 40, 6, 300, 38);
            gui_text_set((gui_text_t *)device_name_label, "Device Name", GUI_FONT_SRC_BMP,
                         gui_rgb(102, 102, 102), 11, 28);
            gui_text_type_set((gui_text_t *)device_name_label,
                              "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)device_name_label, LEFT);
            gui_obj_hidden((gui_obj_t *)device_name_label, false);

            // Create device_name_value
            device_name_value = gui_text_create((gui_obj_t *)note, "device_name_value", 40, 30, 300, 50);
#ifndef _HONEYGUI_SIMULATOR_
            uint8_t name_len = strlen((char *)app_cfg_nv.device_name_legacy);
            if (name_len > 0)
            {
                gui_text_set((gui_text_t *)device_name_value, (char *)app_cfg_nv.device_name_legacy,
                             GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), name_len, 36);
            }
            else
#endif
            {
                gui_text_set((gui_text_t *)device_name_value, "Apple Watch S9", GUI_FONT_SRC_BMP,
                             gui_rgb(255, 255, 255), 14, 36);
            }
            gui_text_type_set((gui_text_t *)device_name_value,
                              "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)device_name_value, LEFT);
            gui_obj_hidden((gui_obj_t *)device_name_value, false);
            break;
        }
    case 1:
        {
            // Create bt_address_bg
            bt_address_bg = gui_rect_create((gui_obj_t *)note, "bt_address_bg", 24, 0, 362, 84, 12,
                                            gui_rgb(44, 44, 46));
            // Create bt_address_label
            bt_address_label = gui_text_create((gui_obj_t *)note, "bt_address_label", 40, 6, 300, 38);
            gui_text_set((gui_text_t *)bt_address_label, "BT Address", GUI_FONT_SRC_BMP,
                         gui_rgb(102, 102, 102), 10, 28);
            gui_text_type_set((gui_text_t *)bt_address_label,
                              "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)bt_address_label, LEFT);
            gui_obj_hidden((gui_obj_t *)bt_address_label, false);

            // Create bt_address_value
#ifndef _HONEYGUI_SIMULATOR_
            uint8_t bd_addr_local[6];
            gap_get_param(GAP_PARAM_BD_ADDR, bd_addr_local);
            snprintf(bt_addr_buffer, sizeof(bt_addr_buffer), "%02X:%02X:%02X:%02X:%02X:%02X",
                     bd_addr_local[5], bd_addr_local[4], bd_addr_local[3],
                     bd_addr_local[2], bd_addr_local[1], bd_addr_local[0]);
#else
            snprintf(bt_addr_buffer, sizeof(bt_addr_buffer), "A4:B1:C2:D3:E4:F5");
#endif

            bt_address_value = gui_text_create((gui_obj_t *)note, "bt_address_value", 40, 30, 300, 50);
            gui_text_set((gui_text_t *)bt_address_value, bt_addr_buffer, GUI_FONT_SRC_BMP,
                         gui_rgb(255, 255, 255), strlen(bt_addr_buffer), 40);
            gui_text_type_set((gui_text_t *)bt_address_value,
                              "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)bt_address_value, LEFT);
            gui_obj_hidden((gui_obj_t *)bt_address_value, false);
            break;
        }
    case 2:
        {
            // Create bt_version_bg
            gui_rect_create((gui_obj_t *)note, "bt_version_bg", 24, 0, 362, 84, 12,
                            gui_rgb(44, 44, 46));
            // Create bt_version_label
            gui_text_t *bt_version_label = gui_text_create((gui_obj_t *)note, "bt_version_label",
                                                           40, 6, 300, 38);
            gui_text_set((gui_text_t *)bt_version_label, "BT Version", GUI_FONT_SRC_BMP,
                         gui_rgb(102, 102, 102), 10, 28);
            gui_text_type_set((gui_text_t *)bt_version_label,
                              "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)bt_version_label, LEFT);
            gui_obj_hidden((gui_obj_t *)bt_version_label, false);

            // Create bt_version_value
            gui_text_t *bt_version_value = gui_text_create((gui_obj_t *)note, "bt_version_value",
                                                           40, 30, 300, 50);
            gui_text_set((gui_text_t *)bt_version_value, "Bluetooth 5.3", GUI_FONT_SRC_BMP,
                         gui_rgb(255, 255, 255), 13, 36);
            gui_text_type_set((gui_text_t *)bt_version_value,
                              "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)bt_version_value, LEFT);
            gui_obj_hidden((gui_obj_t *)bt_version_value, false);
            break;
        }
    case 3:
        {
            // Create wifi_ip_bg
            gui_rect_create((gui_obj_t *)note, "wifi_ip_bg", 24, 0, 362, 84, 12,
                            gui_rgb(44, 44, 46));
            // Create wifi_ip_label
            gui_text_t *wifi_ip_label = gui_text_create((gui_obj_t *)note, "wifi_ip_label",
                                                        40, 6, 300, 38);
            gui_text_set((gui_text_t *)wifi_ip_label, "Wi-Fi IP", GUI_FONT_SRC_BMP,
                         gui_rgb(102, 102, 102), 8, 28);
            gui_text_type_set((gui_text_t *)wifi_ip_label,
                              "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)wifi_ip_label, LEFT);
            gui_obj_hidden((gui_obj_t *)wifi_ip_label, false);

            // Create wifi_ip_value
            gui_text_t *wifi_ip_value = gui_text_create((gui_obj_t *)note, "wifi_ip_value",
                                                        40, 30, 300, 50);
            gui_text_set((gui_text_t *)wifi_ip_value, "192.168.1.42", GUI_FONT_SRC_BMP,
                         gui_rgb(255, 255, 255), 12, 36);
            gui_text_type_set((gui_text_t *)wifi_ip_value,
                              "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)wifi_ip_value, LEFT);
            gui_obj_hidden((gui_obj_t *)wifi_ip_value, false);
            break;
        }
    case 4:
        {
            // Create wifi_version_bg
            gui_rect_create((gui_obj_t *)note, "wifi_version_bg", 24, 0, 362, 84, 12,
                            gui_rgb(44, 44, 46));
            // Create wifi_version_label
            gui_text_t *wifi_version_label = gui_text_create((gui_obj_t *)note, "wifi_version_label",
                                                             40, 6, 300, 38);
            gui_text_set((gui_text_t *)wifi_version_label, "Wi-Fi Version", GUI_FONT_SRC_BMP,
                         gui_rgb(102, 102, 102), 13, 28);
            gui_text_type_set((gui_text_t *)wifi_version_label,
                              "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)wifi_version_label, LEFT);
            gui_obj_hidden((gui_obj_t *)wifi_version_label, false);

            // Create wifi_version_value
            gui_text_t *wifi_version_value = gui_text_create((gui_obj_t *)note, "wifi_version_value",
                                                             40, 30, 300, 50);
            gui_text_set((gui_text_t *)wifi_version_value, "802.11ac (Wi-Fi 5)", GUI_FONT_SRC_BMP,
                         gui_rgb(255, 255, 255), 18, 36);
            gui_text_type_set((gui_text_t *)wifi_version_value,
                              "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)wifi_version_value, LEFT);
            gui_obj_hidden((gui_obj_t *)wifi_version_value, false);
            break;
        }
    default:
        break;
    }
}

/**
 * @brief Timer callback for Bluetooth view - sync UI with BT toggle state
 * Called when entering app_control_centerBluetoothView
 */
void app_control_centerBluetoothView_timer_0_cb_impl(void)
{
    bool bt_enabled = bt_toggle_btn_get_state();

    if (bt_enabled)
    {
        /* BT is ON - show headphones entry and phone section */
        if (bt_headphones_entry_bg != NULL)
        {
            gui_obj_hidden((gui_obj_t *)bt_headphones_entry_bg, false);
        }
        if (headphones_entry_icon != NULL)
        {
            gui_obj_hidden((gui_obj_t *)headphones_entry_icon, false);
        }
        if (headphones_entry_label != NULL)
        {
            gui_obj_hidden((gui_obj_t *)headphones_entry_label, false);
        }
        if (phone_section_label != NULL)
        {
            gui_obj_hidden((gui_obj_t *)phone_section_label, false);
        }

        /* bt_list note structure:
         * index 0: phone item (show if phone bonded) */
        uint16_t note_count = 0;
#ifndef _HONEYGUI_SIMULATOR_
        if (bridge_bt_control_get_state()->bonded[0].exist)
        {
            note_count = 1;
        }
#else
        /* Simulator: always show 1 phone item (iPhone 15 Pro) */
        note_count = 1;
#endif

        gui_obj_hidden((gui_obj_t *)bt_list, false);
        gui_list_set_note_num(bt_list, note_count);
    }
    else
    {
        /* BT is OFF - hide headphones entry and phone section */
        if (bt_headphones_entry_bg != NULL)
        {
            gui_obj_hidden((gui_obj_t *)bt_headphones_entry_bg, true);
        }
        if (headphones_entry_icon != NULL)
        {
            gui_obj_hidden((gui_obj_t *)headphones_entry_icon, true);
        }
        if (headphones_entry_label != NULL)
        {
            gui_obj_hidden((gui_obj_t *)headphones_entry_label, true);
        }
        if (phone_section_label != NULL)
        {
            gui_obj_hidden((gui_obj_t *)phone_section_label, true);
        }

        gui_obj_hidden((gui_obj_t *)bt_list, true);
        gui_list_set_note_num(bt_list, 0);
        gui_list_set_offset(bt_list, 0);
    }

    gui_fb_change();
}
