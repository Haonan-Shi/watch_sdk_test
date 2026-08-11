/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_PHONE_USER_H
#define APP_PHONE_USER_H

#include "../callbacks/app_phone_callbacks.h"
#include "../ui/app_phone_ui.h"

/* Phone call view-switch and DISPLAY_ON wakeup are now driven from
 * phone_call_topic_state_cb() inside app_phone_user.c, in reaction to
 * call_state transitions delivered by bridge_phone_call. The bridge no
 * longer reaches into user-layer code; PHONE_APP_EVENT / phone_app_to_gui
 * are intentionally removed. */

/* Dial key callbacks */
void dial_key_0_cb(void *obj, gui_event_t *e);
void dial_key_1_cb(void *obj, gui_event_t *e);
void dial_key_2_cb(void *obj, gui_event_t *e);
void dial_key_3_cb(void *obj, gui_event_t *e);
void dial_key_4_cb(void *obj, gui_event_t *e);
void dial_key_5_cb(void *obj, gui_event_t *e);
void dial_key_6_cb(void *obj, gui_event_t *e);
void dial_key_7_cb(void *obj, gui_event_t *e);
void dial_key_8_cb(void *obj, gui_event_t *e);
void dial_key_9_cb(void *obj, gui_event_t *e);
void dial_key_star_cb(void *obj, gui_event_t *e);
void dial_key_hash_cb(void *obj, gui_event_t *e);

/* Control callbacks */
void delete_key_pressed(void *obj, gui_event_t *e);
void mute_toggle_cb(void *obj, gui_event_t *e);
void volume_up_cb(void *obj, gui_event_t *e);
void volume_down_cb(void *obj, gui_event_t *e);

/* Timer/init callbacks */
void incoming_view_init_cb_impl(void);
void calling_view_init_cb_impl(void);
void call_timer_tick_impl(void);
void incoming_ring_timer_cb_impl(void);

/* Message subscription callbacks */
void phone_update_incoming_caller_id(gui_obj_t *obj, const char *topic, void *data, uint16_t len);
void phone_update_incoming_number(gui_obj_t *obj, const char *topic, void *data, uint16_t len);
void phone_update_calling_label(gui_obj_t *obj, const char *topic, void *data, uint16_t len);

/* Call control callbacks */
void phone_outgoing_call_cb(void *obj, gui_event_t *e);
void phone_end_call_cb(void *obj, gui_event_t *e);
void phone_answer_call_cb(void *obj, gui_event_t *e);

/**
 * @brief Get the current dialed phone number
 * @return pointer to the phone number string
 */
const char *get_dialed_number(void);

/**
 * @brief Switch to dialer view and clear dialed number
 * This should be called when entering dialer view to ensure clean state
 */
void phone_switch_to_dialer_view(void);

/**
 * @brief Initialize phone user-layer state subscriptions.
 *
 * Registers a permanent gui_msg_subscribe() on gui_obj_get_root() so phone
 * call state transitions (notably HFP_INCOMING that arrive while the user is
 * on the watchface) drive the appropriate view switch + screen wakeup. Call
 * this once after the GUI root and main view exist, e.g. from app_init().
 */
void app_phone_user_init(void);

#endif // APP_PHONE_USER_H