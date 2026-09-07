/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_FINDMY_TASK_H_
#define _APP_FINDMY_TASK_H_
#include <stdint.h>
#include "fmna_gatt.h"
#include "app_msg.h"
extern void *app_findmy_queue_handle;
typedef void (*app_sched_event_handler_t)(void *p_event_data, uint16_t event_size);

typedef struct
{
    void *p_event_data;
    uint16_t event_size;
    app_sched_event_handler_t handler;
} T_APP_SCHED_EVT;

typedef struct
{
    uint16_t length;
    uint16_t conn_handle;
    FMNA_Service_Opcode_t opcode;
    void *buf;
} T_FINDMY_BLE_INDICATION;

/**@brief Function for scheduling an event.
 *
 * @details Puts an event into the event queue.
 *
 * @param[in]   p_event_data   Pointer to event data to be scheduled.
 * @param[in]   event_size     Size of event data to be scheduled.
 * @param[in]   handler        Event handler to receive the event.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t app_findmy_sched_event_put(void const *p_event_data,
                                    uint16_t event_size,
                                    app_sched_event_handler_t handler);

#endif

