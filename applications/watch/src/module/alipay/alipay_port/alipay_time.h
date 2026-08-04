/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef __ALIPAY_TIME_H__
#define __ALIPAY_TIME_H__

#include "stdint.h"
uint32_t alipay_get_system_second_greenwich(void);

uint32_t alipay_get_system_us(void);

uint32_t alipay_get_system_ms(void);

uint32_t alipay_get_system_second(void);

uint32_t alipay_get_system_hour(void);

typedef struct
{
    long tv_sec;
    long tv_usec;
} alipay_iot_timeval;

typedef struct
{
    int32_t year;
    int32_t month;
    int32_t wday;
    int32_t day;
    int32_t hour;
    int32_t minute;
    int32_t second;
    int32_t isdst;
} alipay_iot_local_time;

/*
* @brief: get UTC time
* note:  1. us level
*        2.depend on system time
*        3. return seconds from 1970.01.01 00hour 00minutes 00 seconds
*/
void alipay_iot_gettimeofday(alipay_iot_timeval *now);


extern void alipay_iot_get_local_time(alipay_iot_local_time *time);

extern uint8_t get_system_clock_second(void);

extern void alipay_iot_get_bootup_time(alipay_iot_timeval *time);

void alipay_task_sleep(uint32_t ms);


#endif
