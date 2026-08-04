/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef __HUB_HRS_H__
#define __HUB_HRS_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#include "stdint.h"
#include "stdbool.h"
#include "app_msg.h"


void hrs_command_start(uint8_t type);
void hrs_command_stop(void);
void hrs_event_handler(T_IO_MSG msg);
void hrs_add_hub_task(void);
#endif
