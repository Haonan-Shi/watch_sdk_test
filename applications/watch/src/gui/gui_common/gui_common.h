/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _GUI_COMMON_H_
#define _GUI_COMMON_H_

void gui_task_delay_enable(uint32_t delay_ms);
void gui_task_delay_disable();
void gui_task_delay_set_callbacks(void (*enable_cb)(uint32_t),
                                  void (*disable_cb)(void));

#endif