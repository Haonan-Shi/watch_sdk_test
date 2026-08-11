/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdlib.h>
#include "string.h"
#include <trace.h>
#include "hub_hrs.h"
#include "hub_task.h"
#include "ppg_rtl87x5.h"
#include "ppg_trans_handle.h"
#include "ppg_flash_handle.h"
#include "rtk_ppg_gatt.h"



void hrs_command_start(uint8_t type)
{
    T_IO_MSG hrs_msg;
    hrs_msg.type = HUB_MSG_HRM;
    if (type == 1)
    {
        hrs_msg.subtype = HRS_COMMAND_START_TYPE1;
    }
    else if (type == 2)
    {
        hrs_msg.subtype = HRS_COMMAND_START_TYPE2;
    }
    else
    {
        return;
    }

    send_msg_to_hub_task(&hrs_msg, __LINE__);
}

void hrs_command_stop(void)
{
    T_IO_MSG hrs_msg;
    hrs_msg.type = HUB_MSG_HRM;
    hrs_msg.subtype = HRS_COMMAND_STOP;

    send_msg_to_hub_task(&hrs_msg, __LINE__);
}

void hrs_event_handler(T_IO_MSG msg)
{
    APP_PRINT_INFO1("hrs_event_handler msg.subtype=0x%x", msg.subtype);
    switch (msg.subtype)
    {
    case HRM_SENSOR_INT_TRIGGER:
        {
            hrs_ppg_int_event_handle();
        }
        break;
    case HRS_COMMAND_STOP:
        {
            hrs_command_stop_event_handle();
        }
        break;
    case HRS_COMMAND_START_TYPE1:
        {
            hrs_command_start_type1_event_handle();
        }
        break;
    case HRS_COMMAND_START_TYPE2:
        {
            hrs_command_start_type2_event_handle();
        }
        break;
    case HRM_SENSOR_HRM_READ:
        {
            hrs_result_read_event();
        }
        break;
    default:
        {
            APP_PRINT_INFO2("file = %s, line = %d", TRACE_STRING(__FILE__), __LINE__);
            break;
        }
    }
}

void hrs_add_hub_task(void)
{
    ppg_trans_init();
    ppg_flash_data_init();
    hrs_driver_init();
    rtk_ppg_gatt_init();
}

