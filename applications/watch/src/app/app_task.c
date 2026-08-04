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
#include <gap.h>
#include <gap_le.h>
#include <gap_msg.h>
#include <app_task.h>
#include <app_msg.h>
#include <app_task.h>
#include "trace.h"
#include "sysm.h"
#include "app_ble_gap.h"
#include "app_audio_if.h"
#include "app_timer.h"
#include "event_bus.h"

/** @addtogroup  PERIPH_DEMO
    * @{
    */

/** @defgroup  PERIPH_APP_TASK Peripheral App Task
    * @brief This file handles the implementation of application task related functions.
    *
    * Create App task and handle events & messages
    * @{
    */
/*============================================================================*
 *                              Macros
 *============================================================================*/
#define APP_TASK_PRIORITY             2        //!< Task priorities
#if (CONFIG_APP_NANDBOOT == 1)
#define APP_TASK_STACK_SIZE           512 * 12
#else
#define APP_TASK_STACK_SIZE           512 * 10
#endif

#define MAX_NUMBER_OF_GAP_MESSAGE     0x20  //!<  GAP message queue size
#define MAX_NUMBER_OF_IO_MESSAGE      0x20  //!<  IO message queue size
#define MAX_NUMBER_OF_APP_TIMER       0x30    //!< indicate app timer queue size
#define MAX_NUMBER_OF_EVENT_MESSAGE   (MAX_NUMBER_OF_GAP_MESSAGE + MAX_NUMBER_OF_IO_MESSAGE + MAX_NUMBER_OF_APP_TIMER) //!< Event message queue size

/*============================================================================*
 *                              Variables
 *============================================================================*/
void *app_task_handle;   //!< APP Task handle
void *evt_queue_handle;  //!< Event queue handle
void *io_queue_handle;   //!< IO queue handle

/*============================================================================*
 *                              Functions
 *============================================================================*/
void app_main_task(void *p_param);

/**
 * @brief  Initialize App task
 * @return void
 */
void app_task_init()
{
    os_task_create(&app_task_handle, "app", app_main_task, 0, APP_TASK_STACK_SIZE,
                   APP_TASK_PRIORITY);
}

/**
 * @brief  Send msg to app task
 * @param[in]    p_msg    msg sending to the task
 * @return the status of sending result
 * @retval true     The msg was sent successfully.
 * @retval false    The msg was failed to send.
 */
bool app_send_msg_to_apptask(T_IO_MSG *p_msg)
{
    uint8_t event = EVENT_IO_TO_APP;

    if (os_msg_send(io_queue_handle, p_msg, 0) == false)
    {
        APP_PRINT_ERROR0("send_io_msg_to_app fail io queue");
        return false;
    }
    if (os_msg_send(evt_queue_handle, &event, 0) == false)
    {
        APP_PRINT_ERROR0("send_evt_msg_to_app fail event queue");
        return false;
    }
    return true;
}

/**
 * @brief Default async transport to the app task.
 */
int32_t event_bus_async_send_to_apptask(T_EVENT_BUS_ASYNC_EVENT *async_event, void *context)
{
    (void)context;

    T_IO_MSG io_msg;
    io_msg.type = IO_MSG_TYPE_WRISTBNAD;
    io_msg.subtype = IO_MSG_TYPE_EVENT_BUS;
    io_msg.u.buf = (void *)async_event;

    return app_send_msg_to_apptask(&io_msg) ? 0 : -1;
}

/**
 * @brief        create app task queue
 * @param[in]    void
 * @return       void
 */
void app_main_task_queue_create(void)
{
    os_msg_queue_create(&io_queue_handle, "ioQ", MAX_NUMBER_OF_IO_MESSAGE, sizeof(T_IO_MSG));
    os_msg_queue_create(&evt_queue_handle, "evtQ", MAX_NUMBER_OF_EVENT_MESSAGE, sizeof(uint8_t));
    app_init_timer(evt_queue_handle, MAX_NUMBER_OF_APP_TIMER);
}


/**
 * @brief        App task to handle events & messages
 * @param[in]    p_params    Parameters sending to the task
 * @return       void
 */
void app_main_task(void *p_param)
{
    uint8_t event;
    gap_start_bt_stack(evt_queue_handle, io_queue_handle, MAX_NUMBER_OF_GAP_MESSAGE);


    while (true)
    {
        if (os_msg_recv(evt_queue_handle, &event, 0xFFFFFFFF) == true)
        {
            if (EVENT_GROUP(event) == EVENT_GROUP_IO)
            {
                T_IO_MSG io_msg;

                if (os_msg_recv(io_queue_handle, &io_msg, 0) == true)
                {
                    if (event == EVENT_IO_TO_APP)
                    {
                        app_handle_io_msg(io_msg);
                    }
                }
            }
            else if (EVENT_GROUP(event) == EVENT_GROUP_STACK)
            {
                gap_handle_msg(event);
            }
            else if (EVENT_GROUP(event) == EVENT_GROUP_FRAMEWORK)
            {
                sys_mgr_event_handle(event);
            }
            else if (EVENT_GROUP(event) == EVENT_GROUP_APP)
            {
                app_timer_handle_msg(event);
            }
        }
    }
}

/** @} */ /* End of group PERIPH_APP_TASK */
/** @} */ /* End of group PERIPH_DEMO */
