/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "platform_os.h"
#include "os_msg.h"
#include "os_task.h"
#include "gap.h"
#include "gap_le.h"
#include "app_findmy_task.h"
#include "app_findmy.h"
#include "trace.h"
#include "fmna_state_machine.h"


/** @defgroup  PERIPH_APP_TASK Peripheral App Task
    * @brief This file handles the implementation of application task related functions.
    *
    * Create App task and handle events & messages
    * @{
    */
/*============================================================================*
 *                              Macros
 *============================================================================*/
#define APP_TASK_PRIORITY             1         //!< Task priorities
#define APP_TASK_STACK_SIZE           256 * 32  //!<  Task stack size
#define MAX_NUMBER_OF_GAP_MESSAGE     0x20      //!<  GAP message queue size
#define MAX_NUMBER_OF_IO_MESSAGE      0x20      //!<  IO message queue size
#define MAX_NUMBER_OF_EVENT_MESSAGE   (MAX_NUMBER_OF_GAP_MESSAGE + MAX_NUMBER_OF_IO_MESSAGE)    //!< Event message queue size

/*============================================================================*
 *                              Variables
 *============================================================================*/

void *app_findmy_queue_handle;


/*============================================================================*
 *                              Functions
 *============================================================================*/

int32_t app_findmy_task_msg_handler(T_EVENT_BUS_EVENT_DATA *event_data)
{
    const char *topic = event_data->topic;

    if (strcmp(topic, EVENT_BUS_TOPIC_FINDMY_STATE_MACHINE_KEYROLL) == 0 ||
        strcmp(topic, EVENT_BUS_TOPIC_FINDMY_STATE_MACHINE_DISCONNECT) == 0 ||
        strcmp(topic, EVENT_BUS_TOPIC_FINDMY_STATE_MACHINE_RECONNECT_TIMEOUT) == 0 ||
        strcmp(topic, EVENT_BUS_TOPIC_FINDMY_STATE_MACHINE_UT_START) == 0 ||
        strcmp(topic, EVENT_BUS_TOPIC_FINDMY_STATE_MACHINE_MT) == 0)
    {
        fmna_state_machine_handle_msg(topic);
    }

    return 0;
}

uint32_t app_findmy_sched_event_put(void const *p_event_data,
                                    uint16_t event_size,
                                    const char *topic)
{
    FMNA_LOG_INFO("app_findmy_sched_event_put: 0x%08x, 0x%08x", p_event_data, (uint32_t)event_size);
    if (topic == NULL)
    {
        APP_PRINT_ERROR0("app_findmy_sched_event_put: topic is NULL!");
        return -1;
    }

    return event_bus_publish(topic, (void *)p_event_data, (uint32_t)event_size);
}

/** @} */ /* End of group PERIPH_APP_TASK */
