/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _UI_TASK_H_
#define _UI_TASK_H_

#ifdef  __cplusplus
extern "C" {
#endif      /* __cplusplus */

#include "stdbool.h"
#include "app_msg.h"


/**  @brief IO type definitions for IO message, may extend as requested */
typedef enum
{
    HUB_MSG_LCD,
    HUB_MSG_GSENSOR,
    HUB_MSG_BATTERY_ADC,
    HUB_MSG_CHARGE,
    HUB_MSG_HRM,
    HUB_MSG_CLOCK,
    HUB_MSG_GPS,
    HUB_MSG_BUTTON,
    HUB_MSG_TOUCH,
    HUB_MSG_UART_DEBUG,
    HUB_MSG_USB,
    HUB_MSG_INTERNAL_CHARGER,
    HUB_MSG_PPG_TRANS,
} T_HUB_MSG_TYPE;

/**  @brief IO subtype definitions for @ref T_HUB_MSG_UART_DEBUG type */
typedef enum
{
    UART_DEBUG_MSG_INTERRUPT,
    UART_DEBUG_SIM_TP_INT,
    UART_DEBUG_SIM_TP_RELEASE,
} T_HUB_MSG_UART_DEBUG;

/**  @brief IO subtype definitions for @ref UI_MSG_LCD type */
typedef enum
{
    LCD_MSG_TIMER_MENU,
    LCD_MSG_FRAME_SYNC,
    LCD_MSG_INTERACT,
} T_HUB_MSG_LCD;

/**  @brief IO subtype definitions for @ref UI_MSG_CHARGE type */
typedef enum
{
    CHARGE_MSG_WAKEUP,
} T_HUB_MSG_CHARGE;

/**  @brief IO subtype definitions for @ref UI_MSG_ADC type */
typedef enum
{
    BATTERY_MSG_LOW_POWER,
    BATTERY_MSG_RESTORE,
    BATTERY_MSG_POWER_ON,
    BATTERY_MSG_POWER_OFF,
} T_HUB_MSG_BATTERY_ADC;

/**  @brief IO subtype definitions for @ref UI_MSG_GSENSOR type */
typedef enum
{
    GSENSOR_MSG_WAKEUP,
    GSENSOR_MSG_START,
    GSENSOR_MSG_STOP,
} T_HUB_MSG_GSENSOR;

/**  @brief IO subtype definitions for @ref UI_MSG_HRM type */
typedef enum
{
    HRM_SENSOR_MSG_WAKEUP,
    HRM_SENSOR_MSG_START,
    HRM_SENSOR_MSG_STOP,
    HRM_SENSOR_INT_RELEASE,
    HRM_SENSOR_INT_TRIGGER,
    HRS_COMMAND_STOP,
    HRS_COMMAND_START_TYPE1,
    HRS_COMMAND_START_TYPE2,
    HRM_SENSOR_HRM_READ,
} T_HUB_MSG_HRM;

/**  @brief IO subtype definitions for @ref UI_MSG_TOUCH type */
typedef enum
{
    TOUCH_MSG_INT,
    TOUCH_MSG_TIMEOUT,
} T_HUB_MSG_TOUCH;

/**  @brief IO subtype definitions for @ref UI_MSG_BUTTON type */
typedef enum
{
    SHORT_BUTTON_0,
    LONG_BUTTON_0,
    PRESS_BUTTON_0,
    RELEASE_BUTTON_0,
    PRESSING_BUTTON_0,
    SHORT_BUTTON_1,
    LONG_BUTTON_1,
    PRESS_BUTTON_1,
    RELEASE_BUTTON_1,
    SHORT_BUTTON_2,
    LONG_BUTTON_2,
    PRESS_BUTTON_2,
    RELEASE_BUTTON_2,
} T_BUTTON_MSG_TYPE;

/**  @brief IO subtype definitions for @ref UI_MSG_USB type */
typedef enum
{
    USB_MSG_ADP_IN,
    USB_MSG_ADP_OUT,
    USB_MSG_WKAE_UP,
} T_HUB_MSG_USB;

/**  @brief IO subtype definitions for @ref UI_MSG_INTERNAL_CHARGE type */
typedef enum
{
    GET_BATTERY_LEVEL,
    CHARGER_STATE_CHANGE,
} T_HUB_MSG_INTERNAL_CHARGE;

extern void *flash_mutex_handle;

/**
 * @brief  Initialize App task
 * @return void
 */
void hub_task_init(void);
void hub_task_handle_msg(T_IO_MSG ui_msg);
bool send_msg_to_hub_task(T_IO_MSG *p_msg, uint32_t line);

#ifdef  __cplusplus
}
#endif      /*  __cplusplus */
#endif
