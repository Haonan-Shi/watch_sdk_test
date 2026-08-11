/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_CONTACTS_CALLBACKS_H
#define APP_CONTACTS_CALLBACKS_H

#include "gui_api.h"
#include "gui_text.h"
#include "gui_obj_focus.h"

// Event callback function declarations
void app_contact_window_key_0_cb(void *obj, gui_event_t *e);
void app_contacts_c_back_icon_bg_clicked_cb(void *obj, gui_event_t *e);
void app_contacts_c_window_key_0_cb(void *obj, gui_event_t *e);
void contact_main_list_bg2_clicked_cb(void *obj, gui_event_t *e);
void contact_main_list_bg3_clicked_cb(void *obj, gui_event_t *e);
void rect_1_clicked_cb(void *obj, gui_event_t *e);
void hg_time_label_1770691753179_vzi3_time_update_cb(void *p);
void app_contacts_c_time_text_time_update_cb(void *p);

#endif // APP_CONTACTS_CALLBACKS_H
