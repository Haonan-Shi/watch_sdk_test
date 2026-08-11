/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <zephyr/device.h>
#include <time.h>
#include <zephyr/sys/timeutil.h>
#include "trace.h"
#include "string.h"
#include "app_task.h"
#include "hub_clock.h"
#include "module_global_data.h"
#include "communicate_protocol.h"
#include "communicate_sync_pedo.h"
#include "hub_task.h"

#define RTC_ALARM_IDX       0
#define RTC_ALARM_MASK      63
static const struct device *rtc = DEVICE_DT_GET(DT_ALIAS(rtc));

static const struct rtc_time rtc_default_time =
{
    .tm_sec = 0,
    .tm_min = 0,
    .tm_hour = 12,
    .tm_mday = 1,
    .tm_mon = 0,
    .tm_year = 125,
    .tm_wday = 3,
    .tm_yday = 0,
    .tm_isdst = -1,
    .tm_nsec = 0,
};

void clock_add_hub_task(void)
{
    clock_driver_init();
}


static void rtc_min_alarm_callback_handler(const struct device *dev, uint16_t id,
                                           void *user_data)
{
    time_t cur_time_sec;
    struct rtc_time alarm_time;

    rtc_get_time(rtc, (struct rtc_time *)&RtkWristbandSys.Global_Time);
    cur_time_sec = timeutil_timegm((struct tm *)(&RtkWristbandSys.Global_Time));
    RtkWristbandSys.SecondCountRTC = cur_time_sec;

    cur_time_sec += 60;
    gmtime_r(&cur_time_sec, (struct tm *)(&alarm_time));
    rtc_alarm_set_time(rtc, RTC_ALARM_IDX, RTC_ALARM_MASK, &alarm_time);

    T_IO_MSG clock_msg;
    clock_msg.type = HUB_MSG_CLOCK;
    send_msg_to_hub_task(&clock_msg, __LINE__);
}

void clock_driver_init(void)
{
    time_t cur_time_sec;
    struct rtc_time alarm_time;

    memcpy((void *)&RtkWristbandSys.Global_Time, (void *)&rtc_default_time, sizeof(struct rtc_time));

    rtc_set_time(rtc, &rtc_default_time);
    rtc_alarm_set_callback(rtc, RTC_ALARM_IDX, rtc_min_alarm_callback_handler, NULL);

    cur_time_sec = timeutil_timegm((struct tm *)(&rtc_default_time));
    RtkWristbandSys.SecondCountRTC = cur_time_sec;
    cur_time_sec += 60;
    gmtime_r(&cur_time_sec, (struct tm *)(&alarm_time));
    rtc_alarm_set_time(rtc, RTC_ALARM_IDX, RTC_ALARM_MASK, &alarm_time);
}

void system_clock_init(time_t second)
{
    gmtime_r(&second, (struct tm *)(&RtkWristbandSys.Global_Time));
    rtc_set_time(rtc, (struct rtc_time *)&RtkWristbandSys.Global_Time);
}

/* calculate day of week */
DAY_OF_WEEK get_day_of_week(time_t second)
{
    uint32_t day = second / DAY;

    DAY_OF_WEEK today = (DAY_OF_WEEK)(((day % LENGTH_OF_WEEK) + SYSTEM_ORIGIN_DAY_OF_WEEK) %
                                      LENGTH_OF_WEEK);

    return today;
}

void minute_system_clock_message_handle(void)
{
    APP_PRINT_INFO1("minute_system_clock_message_handle second = %d", RtkWristbandSys.SecondCountRTC);

    APP_PRINT_INFO6("year:%d,month:%d,day:%d,hour:%d,minute:%d,second:%d\r\n",
                    RtkWristbandSys.Global_Time.tm_year,
                    RtkWristbandSys.Global_Time.tm_mon, RtkWristbandSys.Global_Time.tm_mday,
                    RtkWristbandSys.Global_Time.tm_hour, RtkWristbandSys.Global_Time.tm_min,
                    RtkWristbandSys.Global_Time.tm_sec);
}

uint32_t convert_time_to_second(time_union_t time)
{
    /*
        The year field in 'time_union_t' is based on 2000.
        tm_year: years since 1900 (e.g. 2025 : 2025-1900 = 125)
        tm_mon: month, range [0, 11], 0 = January (e.g. November : 10)
        tm_mday: day of the month, range [1, 31]
        tm_hour: hours, range [0, 23]
        tm_min: minutes, range [0, 59]
        tm_sec: seconds, range [0, 60] (up to 60 to allow for leap seconds)
        tm_wday: day of the week, range [0, 6], 0 = Sunday, 1 = Monday, ..., 6 = Saturday
    */
    uint32_t second = 0;
    struct tm tm_to_convert =
    {
        .tm_sec  = time.time.seconds,
        .tm_min  = time.time.minute,
        .tm_hour = time.time.hours,
        .tm_mday = time.time.day,
        .tm_mon  = time.time.month - 1,
        .tm_year = time.time.year + 100,
    };
    second = timeutil_timegm(&tm_to_convert);
    return second;
}

void set_wristband_clock(time_union_t time)
{
    /*
        The year field in 'time_union_t' is based on 2000.
        tm_year: years since 1900 (e.g. 2025 : 2025-1900 = 125)
        tm_mon: month, range [0, 11], 0 = January (e.g. November : 10)
        tm_mday: day of the month, range [1, 31]
        tm_hour: hours, range [0, 23]
        tm_min: minutes, range [0, 59]
        tm_sec: seconds, range [0, 60] (up to 60 to allow for leap seconds)
        tm_wday: day of the week, range [0, 6], 0 = Sunday, 1 = Monday, ..., 6 = Saturday
    */
    struct rtc_time cur_time;
    RtkWristbandSys.Global_Time.tm_sec = time.time.seconds;
    RtkWristbandSys.Global_Time.tm_min = time.time.minute;
    RtkWristbandSys.Global_Time.tm_hour = time.time.hours;
    RtkWristbandSys.Global_Time.tm_mday = time.time.day;
    RtkWristbandSys.Global_Time.tm_mon = time.time.month - 1;
    RtkWristbandSys.Global_Time.tm_year = time.time.year + 100;
    RtkWristbandSys.SecondCountRTC = timeutil_timegm((struct tm *)(&RtkWristbandSys.Global_Time));
    RtkWristbandSys.Global_Time.tm_wday = get_day_of_week(RtkWristbandSys.SecondCountRTC);

    rtc_set_time(rtc, (struct rtc_time *)&RtkWristbandSys.Global_Time);
}

uint8_t get_system_clock_second(void)
{
    rtc_get_time(rtc, (struct rtc_time *)&RtkWristbandSys.Global_Time);
    return RtkWristbandSys.Global_Time.tm_sec;
}

volatile struct rtc_time *get_system_clock_time(void)
{
    rtc_get_time(rtc, (struct rtc_time *)&RtkWristbandSys.Global_Time);
    return &RtkWristbandSys.Global_Time;
}
