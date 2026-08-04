/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef __APP_MSG_HANDLE_H__
#define __APP_MSG_HANDLE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "app_msg.h"
#include "os_msg.h"
#include "app_task.h"

/* Defines ------------------------------------------------------------------*/
/**  @brief IO message definition for communications between tasks*/

void watch_handle_io_message(T_IO_MSG *p_watch_msg);
bool get_play_flag(void);
void set_play_flag(bool play_fg);

#ifdef __cplusplus
}
#endif

#endif /*__INTERFACE_TASK_H__*/
