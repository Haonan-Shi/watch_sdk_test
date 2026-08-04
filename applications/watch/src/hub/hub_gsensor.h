/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */


#ifndef __HUB_GSENSOR_H__
#define __HUB_GSENSOR_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "stdint.h"
#include "app_msg.h"


typedef struct
{
    int16_t AXIS_X;
    int16_t AXIS_Y;
    int16_t AXIS_Z;
} AxesRaw_t;



void gsensor_add_hub_task(void);
void gsensor_event_handler(T_IO_MSG msg);

#ifdef __cplusplus
}
#endif

#endif //__WRISTBAND_GSENSOR_H__


