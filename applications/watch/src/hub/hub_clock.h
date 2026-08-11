/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _HUB_CLOCK_H_
#define _HUB_CLOCK_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr/drivers/rtc.h>
#include <time.h>
#include <stdint.h>
#include "communicate_protocol.h"

#define IsLeapYear(yr) (!((yr) % 400) || (((yr) % 100) && !((yr) % 4)))

#define YearLength(yr) (IsLeapYear(yr) ? 366 : 365)



#define BEGYEAR                     1970     // UTC started at 00:00:00 January 1, 2000
#define DAY                         86400UL  // 24 hours * 60 minutes * 60 seconds
#define SYSTEM_ORIGIN_DAY_OF_WEEK   (Thur)  //1970-01-01 is Thur
#define LENGTH_OF_WEEK              (7)
#define RTC_CNT_MAX_VALUE           (1024*1024*16UL -1)         //RTC->CNT: [23:0]
#define RTC_PRESCALER_VALUE         0

typedef enum
{
    Sun  = 0,
    Mon  = 1,
    Tues  = 2,
    Wed  = 3,
    Thur = 4,
    Fri  = 5,
    Sat  = 6,
} DAY_OF_WEEK;

// #define ABS_PARAMS(a,b)             ((a>b) ? (a-b):(b-a))



void clock_add_hub_task(void);
void clock_driver_init(void);
void set_wristband_clock(time_union_t time);
DAY_OF_WEEK get_day_of_week(time_t second);//used to calculate day of week
void minute_system_clock_message_handle(void);
void system_clock_init(time_t second);
uint8_t get_system_clock_second(void);
volatile struct rtc_time *get_system_clock_time(void);
uint32_t convert_time_to_second(time_union_t time);


#ifdef __cplusplus
}
#endif


#endif //_HUB_CLOCK_H_

