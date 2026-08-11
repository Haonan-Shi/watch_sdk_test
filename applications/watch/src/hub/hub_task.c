/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <os_msg.h>
#include <os_task.h>
#include <os_sync.h>
#include <hub_task.h>
#include <app_msg.h>
#include "trace.h"
#include "string.h"
#include "math.h"
#include "hub_clock.h"
#if CONFIG_HUB_CHARGER
#include "hub_charger.h"
#endif
#if CONFIG_HUB_GSENSOR
#include "hub_gsensor.h"
#endif
#if CONFIG_RTK_PPG
#include "hub_hrs.h"
#include "ppg_rtl87x5.h"
#endif
/** @addtogroup  PERIPH_DEMO
    * @{
    */

/** @defgroup  SENSOR_HUB_TASK Peripheral App Task
    * @brief This file handles the implementation of application task related functions.
    *
    * Create App task and handle events & messages
    * @{
    */
/*============================================================================*
 *                              Macros
 *============================================================================*/

#define HUB_TASK_PRIORITY          1        //!< Task priorities
#define HUB_TASK_STACK_SIZE        512 * 4  //!<  Task stack size


/*============================================================================*
 *                              Variables
 *============================================================================*/

void *hub_task_handle = NULL;
void *hub_queue_handle = NULL;
void *flash_mutex_handle = NULL;
#if F_APP_SUPPORT_USB
void *usb_sem_handle = NULL;
#endif

/*============================================================================*
 *                              Functions
 *============================================================================*/
void hub_task(void *p_param);


void hub_task_init()
{
    os_task_create(&hub_task_handle, "hub_task", hub_task, 0, HUB_TASK_STACK_SIZE,
                   HUB_TASK_PRIORITY);
}

bool send_msg_to_hub_task(T_IO_MSG *p_msg, uint32_t line)
{
    if (os_msg_send(hub_queue_handle, p_msg, 0) == false)
    {
        APP_PRINT_ERROR1("send_io_msg_to_ui task fail io queue line = %d", line);
        return false;
    }
    return true;
}

void wristband_driver_init(void)
{
    clock_add_hub_task();

#if CONFIG_HUB_CHARGER
    charger_add_hub_task();
#endif

#if (F_APP_AUTO_SUPPORT == 0)
    //button_add_hub_task();
#endif
#if CONFIG_HUB_GSENSOR
    gsensor_add_hub_task();
#endif
#if CONFIG_RTK_PPG
    hrs_add_hub_task();
#endif
    //uart_debug_add_hub_task();
#if F_APP_SUPPORT_USB
    usb_detect_add_hub_task();
#endif
}

/**
 * @brief        sensor hub task
 * @param[in]    p_params    Parameters sending to the task
 * @return       void
 */

void hub_task(void *pvParameters)
{
    T_IO_MSG hub_msg;

    os_msg_queue_create(&hub_queue_handle, "hubQ", 0x10, sizeof(T_IO_MSG));
    os_mutex_create(&flash_mutex_handle);
#if F_APP_SUPPORT_USB
    os_sem_create(&usb_sem_handle, "usb_sem", 0, 1);
#endif

    wristband_driver_init();
    while (true)
    {
        if (os_msg_recv(hub_queue_handle, &hub_msg, 0xFFFFFFFF) == true)
        {
            hub_task_handle_msg(hub_msg);
        }
    }
}

void hub_task_handle_msg(T_IO_MSG hub_msg)
{
    uint16_t msg_type = hub_msg.type;
    switch (msg_type)
    {
#if F_APP_SUPPORT_USB
    case HUB_MSG_USB:
        {
            usb_detect_event_handler(hub_msg);
        }
        break;
#endif
#if CONFIG_HUB_CLOCK
    case HUB_MSG_CLOCK:
        {
            minute_system_clock_message_handle();
        }
        break;
#endif
#if CONFIG_HUB_GSENSOR
    case HUB_MSG_GSENSOR:
        {
            gsensor_event_handler(hub_msg);
        }
        break;
#endif
#if CONFIG_HUB_HRS
    case HUB_MSG_HRM:
        {
            hrs_event_handler(hub_msg);
        }
        break;
    case HUB_MSG_PPG_TRANS:
        {
            ppg_trans_event_handler(hub_msg);
        }
        break;
#endif
#if CONFIG_HUB_CHARGER
    case HUB_MSG_INTERNAL_CHARGER:
        {
            charger_event_handler(hub_msg);
        }
        break;
#endif
    default:
        break;
    }
}

/** @} */ /* End of group IO_TASK */
/** @} */ /* End of group PERIPH_DEMO */
