/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_FINDMY_TASK_H_
#define _APP_FINDMY_TASK_H_
#include <stdint.h>
#include "fmna_gatt.h"
#include "event_bus.h"
#include "app_task.h"

#define EVENT_BUS_TOPIC_FINDMY_STATE_MACHINE_ALL_TOPIC                      "fm/sm/*"
#define EVENT_BUS_TOPIC_FINDMY_STATE_MACHINE_KEYROLL                        "fm/sm/keyroll"
#define EVENT_BUS_TOPIC_FINDMY_STATE_MACHINE_DISCONNECT                     "fm/sm/disconn"
#define EVENT_BUS_TOPIC_FINDMY_STATE_MACHINE_RECONNECT_TIMEOUT              "fm/sm/reconn_timeout"
#define EVENT_BUS_TOPIC_FINDMY_STATE_MACHINE_UT_START                       "fm/sm/ut_start"
#define EVENT_BUS_TOPIC_FINDMY_STATE_MACHINE_MT                             "fm/sm/mt"
#define EVENT_BUS_TOPIC_FINDMY_EVENT_GATT_SEND_PACKET_EXT_INDICATION        "fm/event/gatt/sp"
#define EVENT_BUS_TOPIC_FINDMY_EVENT_GATT_SEND_NEXT_PACKET                  "fm/event/gatt/snp"
#define EVENT_BUS_TOPIC_FINDMY_EVENT_MOTION_ACTIVE_POLL_DURATION_TIMEOUT    "fm/event/mot/apdt"
#define EVENT_BUS_TOPIC_FINDMY_EVENT_STATEMACHINE_SET_NEXT_2ND_KEY          "fm/event/sm/sn2k"
#define EVENT_BUS_TOPIC_FINDMY_EVENT_STATEMACHINE_UPDATE_NEXT_2ND_KEY       "fm/event/sm/un2k"
#define EVENT_BUS_TOPIC_FINDMY_EVENT_STATEMACHINE_STATE_UPDATE              "fm/event/sm/su"

extern void *app_findmy_queue_handle;

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
                                    const char *topic);

int32_t app_findmy_task_msg_handler(T_EVENT_BUS_EVENT_DATA *event_data);

#endif
