/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef WATCHFACE_SPORT_CALLBACKS_H
#define WATCHFACE_SPORT_CALLBACKS_H

#include "gui_api.h"
#include "gui_text.h"
#include "gui_obj_focus.h"

// Event callback function declarations
void hg_arc_1766556753455_tg7q_msg_cb_0(gui_obj_t *obj, const char *topic, void *data,
                                        uint16_t len);
void hg_image_1766555014041_zggx_clicked_cb(void *obj, gui_event_t *e);
void hg_time_label_1768897114762_8ilu_time_update_cb(void *p);

#endif // WATCHFACE_SPORT_CALLBACKS_H
