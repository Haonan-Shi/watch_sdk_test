/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _COMMUNICATE_TASK_H_
#define _COMMUNICATE_TASK_H_

#include "app_msg.h"
#include "stdbool.h"

#define ASSERT(x) if(!(x)) { DBG_DIRECT("Assert(%s) fail in %s,%d\n", #x,__FILE__, __LINE__, 3); while(1) {;}}
/**
 * @brief  Initialize App task
 * @return void
 */
void communicate_task_init(void);
bool send_msg_to_communicatetask(T_IO_MSG *p_msg);

#endif
