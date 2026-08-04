/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "stdlib.h"
#include "stdio.h"
#include "gui_common.h"

static void (*gui_task_delay_enable_cb)(uint32_t delay_ms) = NULL;
static void (*gui_task_delay_disable_cb)(void) = NULL;

void gui_task_delay_enable(uint32_t delay_ms)
{
    if (gui_task_delay_enable_cb)
    {
        gui_task_delay_enable_cb(delay_ms);
    }
}

void gui_task_delay_disable(void)
{
    if (gui_task_delay_disable_cb)
    {
        gui_task_delay_disable_cb();
    }
}

void gui_task_delay_set_callbacks(void (*enable_cb)(uint32_t),
                                  void (*disable_cb)(void))
{
    gui_task_delay_enable_cb = enable_cb;
    gui_task_delay_disable_cb = disable_cb;
}



