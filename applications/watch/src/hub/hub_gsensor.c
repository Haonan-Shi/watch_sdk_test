/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "string.h"
#include "trace.h"
#include "os_timer.h"
#include "gsensor_stk8321.h"
#include "hub_gsensor.h"
#include "hub_task.h"
#include "module_global_data.h"


void gsensor_add_hub_task(void)
{
    gsensor_init();
}

void gsensor_message_handle(void)
{
#if 0
    AxesRaw_t accData[32];
    uint8_t fifo_data_count;

    if ((RtkWristbandSys.flag_field.bond_state == false) || RtkWristbandSys.flag_field.low_power_flag ||
        RtkWristbandSys.flag_field.power_off_flag)
    {
        return;
    }

    fifo_data_count = gsensor_get_fifo_length();
    if (gsensor_get_fifo_data(fifo_data_count, accData))
    {
        for (uint8_t i = 0; i < fifo_data_count; i++)
        {
            if (RtkWristbandSys.flag_field.algorithm_started  && (RtkWristbandSys.charger_status == NoCharge))
            {
                // rtk_gsa_fsm((int16 *)&accData[i]);
            }
        }
    }
#endif
}

void gsensor_event_handler(T_IO_MSG msg)
{
    uint8_t value = msg.subtype;
    switch (value)
    {
    case GSENSOR_MSG_WAKEUP:
        {
            gsensor_message_handle();
        }
        break;
    case GSENSOR_MSG_START:
        {
            gsensor_enable();
        }
        break;
    case GSENSOR_MSG_STOP:
        {
            gsensor_disable();
        }
        break;
    default:
        {
            APP_PRINT_INFO2("file = %s, line = %d", TRACE_STRING(__FILE__), __LINE__);
        }
        break;
    }
}
