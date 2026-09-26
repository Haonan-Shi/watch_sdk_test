/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*
 * Zephyr port of the vendor tinyml_main.h.  The API surface consumed by the UART
 * parser is unchanged; the implementations in main.c are backed by Zephyr
 * kernel objects (k_msgq for messaging, k_timer + k_work for ts_timer).
 */

#ifndef TINYML_EDGE_MAIN_H
#define TINYML_EDGE_MAIN_H

#include <stdbool.h>
#include "app_msg.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*pfunc)(void *);

/* (Re)arm the single-shot RX inactivity timeout; func runs in thread context
 * (via a workqueue) when the timeout fires. */
void ts_timer_start(void (func)(void *xTimer));

/* Post an inter-task message to the edge worker thread's queue. */
bool tinyml_send_msg_to_task(T_IO_MSG *p_msg);

#ifdef __cplusplus
}
#endif

#endif /* TINYML_EDGE_MAIN_H */
