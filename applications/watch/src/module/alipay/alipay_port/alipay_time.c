/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */


#include "trace.h"
#include "string.h"
#include "app_task.h"
#include "rtl876x_rcc.h"
#include "rtl876x_rtc.h"
#include "rtl876x_nvic.h"
#include "hub_clock.h"
#include "module_global_data.h"
#include "communicate_protocol.h"
#include "communicate_sync_pedo.h"
#include "hub_task.h"
#include "vector_table.h"
#include "alipay_time.h"
#include "alipay_common.h"
#include "alipay_config.h"
#include "platform_utils.h"

#if  CONFIG_ALIPAY

#define     ClOCK_RTC_COMPARATOR        0
#define     RTC_CLOCK_SOURCE_FREQ       32000

uint32_t alipay_get_system_us(void)
{

    return alipay_get_system_second() * 1000 * 1000;
}


uint32_t alipay_get_system_ms(void)
{

    return alipay_get_system_second() * 1000;
}

uint32_t alipay_get_system_second_greenwich()
{
    return alipay_get_system_second() - 8 * 60 * 60;
}

uint32_t alipay_get_system_second(void)
{
    uint32_t cur_rtc_tick_count = RTC_GetCounter();
    uint32_t diff_second = 0;
    if (cur_rtc_tick_count > RtkWristbandSys.pre_rtc_tick_count)
    {
        diff_second = (cur_rtc_tick_count - RtkWristbandSys.pre_rtc_tick_count) / RTC_CLOCK_SOURCE_FREQ;
    }
    else
    {
        diff_second = (cur_rtc_tick_count + 0xffffff - RtkWristbandSys.pre_rtc_tick_count) /
                      RTC_CLOCK_SOURCE_FREQ;
    }
    return (RtkWristbandSys.SecondCountRTC + diff_second) /*- 8*60*60*/;
}
uint32_t alipay_get_system_hour(void)
{

    return alipay_get_system_second() / 60 / 60;
}

void alipay_set_system_time(PARAM_IN int32_t timestamp_s)
{
    //#warning alipay_set_system_time demo
}


void alipay_iot_gettimeofday(alipay_iot_timeval *now)
{
    // yuyin
    //  struct timespec ts = {0, 0};
    //  clock_gettime(CLOCK_REALTIME, &ts);
    //  now->tv_sec = (long)(ts.tv_sec);
    //  now->tv_usec = (int)(ts.tv_nsec / 1000);

    now->tv_sec = alipay_get_system_second_greenwich();
    now->tv_usec = 0;

//  AliPay_LOG("[Alipay] alipay_iot_gettimeofday");
//  AliPay_LOG("[Alipay] now->tv_sec %d", now->tv_sec);
}

void alipay_iot_get_local_time(alipay_iot_local_time *time)
{
    uint32_t second = get_system_clock_second();
    extern RtkWristBandSysType_t RtkWristbandSys;

    time->year = RtkWristbandSys.Global_Time.tm_year + 1900;
    time->month = RtkWristbandSys.Global_Time.tm_mon + 1;
    time->wday = RtkWristbandSys.Global_Time.tm_wday;
    time->day = RtkWristbandSys.Global_Time.tm_mday;
    time->hour = RtkWristbandSys.Global_Time.tm_hour;
    time->minute = RtkWristbandSys.Global_Time.tm_min;
    time->second = RtkWristbandSys.Global_Time.tm_sec;
    time->isdst = 0;

}
void alipay_iot_get_bootup_time(alipay_iot_timeval *time)
{
    alipay_iot_gettimeofday(time);
}
void alipay_iot_set_utc_time(int64_t utc)
{
    // no used
}

/*
* @brief: task sleep us
* @input: milliseconds value
*/
void alipay_task_sleep(uint32_t ms)
{
//    #warning alipay_task_sleep demo
//    struct timeval tval;
//    tval.tv_sec = ms / 1000;
//    tval.tv_usec = (ms * 1000) % 1000000;
//    select(0, NULL, NULL, NULL, &tval);
    platform_delay_ms(ms);

}

#else

uint32_t alipay_get_system_us(void)
{

    return 0;
}


uint32_t alipay_get_system_ms(void)
{

    return 0;
}

uint32_t alipay_get_system_second_greenwich()
{
    return 0;
}

uint32_t alipay_get_system_second(void)
{
    return 0;
}
uint32_t alipay_get_system_hour(void)
{
    return 0;
}

void alipay_set_system_time(PARAM_IN int32_t timestamp_s)
{
//#warning alipay_set_system_time demo
}



void alipay_iot_gettimeofday(alipay_iot_timeval *now)
{
    // yuyin

}

void alipay_iot_get_local_time(alipay_iot_local_time *time)
{
    //yuyin
}
void alipay_iot_get_bootup_time(alipay_iot_timeval *time)
{
    ;
}
void alipay_iot_set_utc_time(int64_t utc)
{
    // no used
}

#endif



