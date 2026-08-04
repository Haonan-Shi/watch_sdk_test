/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_TASK_H_
#define _APP_TASK_H_

#include "app_msg.h"
#include "stdbool.h"
#include "event_bus.h"

/**  @brief IO subtype definitions for @ref IO_MSG_TYPE_WRISTBNAD type */
typedef enum
{
    APP_MSG_BWPS_TX_VALUE,
    APP_MSG_RTC_TIMEROUT_WALL_CLOCK,
    APP_MSG_SENSOR_WAKE_UP,
    APP_MSG_LED_TWINKLE,
    APP_MSG_MOTOR_VIBRATE,
    APP_MSG_CHARGER_STATE,
    APP_MSG_RTC_LOW_BATTERY_RESTORE,
    APP_MSG_RTC_ALARM,
    APP_MSG_POWER_OFF,
    APP_MSG_POWER_ON,
    APP_MSG_HRS_EVENT,
    APP_MSG_SENSOR_MOTION_INTERRUPT,
    APP_MSG_UART_CMD_DEBUG,
    APP_MSG_HRS_TIMEOUT_HANDLE,
    APP_MSG_UPDATE_CONPARA,
    APP_MSG_REPORT_BUTTON,
    APP_MSG_UART_GPS,
    APP_MSG_UART_DEBUG_RX,
    APP_MSG_WAS_RX_VALUE,
    APP_MSG_WAS_ENABLE_CCCD,
    APP_MSG_VOICE_DMA_RX,
    APP_MSG_TOUCH_GES,
    APP_MSG_TOUCH_INT,
    APP_MSG_TOUCH_TIMEOUT,
    APP_MSG_TOUCH_HANDLE,
    APP_MSG_MENU_TIMER,
    APP_MSG_LCD_SYNC,
    APP_MSG_ANCS_DISCOVERY,
} T_APP_TASK_SUB_MSG;

extern void *evt_queue_handle;  //!< Event queue handle
extern void *io_queue_handle;   //!< IO queue handle

/**
 * @brief  Initialize App task
 * @return void
 */
void app_task_init(void);
bool app_send_msg_to_apptask(T_IO_MSG *p_msg);
void app_main_task_queue_create(void);
int32_t event_bus_async_send_to_apptask(T_EVENT_BUS_ASYNC_EVENT *async_event, void *context);
#endif
