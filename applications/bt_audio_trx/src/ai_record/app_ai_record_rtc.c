/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */



#if CONFIG_REALTEK_APP_AI_RECORD
#include "string.h"
#include <stdlib.h>
#include "app_cfg.h"
#include "app_cfg_nv.h"
#include "ftl.h"
#include "trace.h"
#include "app_ai_record_rtc.h"
#include "app_flags.h"

#define IsLeapYear(yr) (!((yr) % 400) || (((yr) % 100) && !((yr) % 4)))
#define YearLength(yr) (IsLeapYear(yr) ? 366 : 365)
#define BEGYEAR                     1900     // UTC started at 00:00:00 January 1, 2000
#define DAY                         86400UL  // 24 hours * 60 minutes * 60 seconds
#define ONE_MINUTE                  60UL     // 1 minute = 60 seconds
#define ONE_HOUR                    3600UL   // 1 hour = 60 * 60 seconds
#define LENGTH_OF_WEEK              (7)
#define LEAP_YEAR_DAYS              (366)
#define COMMON_YEAR_DAYS            (365)
#define ODD_MONTH_DAYS              (31)
#define EVEN_MONTH_DAYS             (30)
#define FEBRUARY_MIN_DAYS           (28)
#define MONTH_MAX                   (12)
#define HOUR_MAX                    (23)
#define MINUTE_MAX                  (59)
#define SECOND_MAX                  (59)

#if CONFIG_REALTEK_APP_RTC_CALENDAR_SUPPORT

static uint8_t app_ai_record_rtc_calendar_month_length_calc(uint8_t lpyr, uint8_t mon)
{
    uint8_t days = ODD_MONTH_DAYS;

    if (mon == 1)   // feb
    {
        days = (FEBRUARY_MIN_DAYS + lpyr);
    }
    else
    {
        if (mon > 6)   // aug-dec
        {
            mon--;
        }

        if (mon & 1)
        {
            days = EVEN_MONTH_DAYS;
        }
    }
    return (days);
}

static bool app_ai_record_rtc_calendar_check_utc_time(T_UTC_TIME *utc_time)
{
    if (utc_time->year < BEGYEAR)
    {
        return false;
    }

    if ((utc_time->month == 0) || (utc_time->month > MONTH_MAX))
    {
        return false;
    }

    if ((utc_time->day == 0) ||
        (utc_time->day > app_ai_record_rtc_calendar_month_length_calc(IsLeapYear(utc_time->year),
                                                                      utc_time->month)))
    {
        return false;
    }

    if ((utc_time->hour > HOUR_MAX) || (utc_time->minutes > MINUTE_MAX) ||
        (utc_time->seconds > SECOND_MAX))
    {
        return false;
    }

    return true;
}

#define UTC_START_DAY   6   //// GPS start(1980.01.06)
#define LEAP_SECONDS 18 // Todo: If user want to transfer actual UTC time, the could use this
T_UTC_TIME app_ai_record_rtc_gps_to_date(uint32_t gps_week, uint32_t gps_seconds)
{
    T_UTC_TIME utc_time = {0};
    // GPS start(1980.01.06)
    utc_time.year = 1980; // from 1900,this 80 is 1980
    utc_time.month = 0;   // jan from 0 month
    utc_time.day = 6;  // day
    utc_time.hour = 0;
    utc_time.minutes = 0;
    utc_time.seconds = 0;
    uint8_t base_day = UTC_START_DAY - 1;

    if (gps_seconds < LEAP_SECONDS)
    {
        gps_seconds = gps_seconds + DAY;
        base_day--;
    }
    gps_seconds = gps_seconds - LEAP_SECONDS;
    /* calculate the time less than a day - hours, minutes, seconds */
    {

        uint32_t day = gps_seconds % DAY;
        utc_time.seconds = day % ONE_MINUTE;
        utc_time.minutes = (day % ONE_HOUR) / ONE_MINUTE;
        utc_time.hour = day / ONE_HOUR;
    }

    /* Fill in the calendar - day, month, year */
    {
        uint32_t numDays = base_day + gps_week * LENGTH_OF_WEEK + gps_seconds / DAY;

//        utc_time.year = BEGYEAR;
        while (numDays >= YearLength(utc_time.year))
        {
            numDays -= YearLength(utc_time.year);
            utc_time.year++;
        }

        utc_time.month = 0;

        while (numDays >= app_ai_record_rtc_calendar_month_length_calc(IsLeapYear(utc_time.year),
                                                                       utc_time.month))
        {
            uint16_t month_len = app_ai_record_rtc_calendar_month_length_calc(IsLeapYear(utc_time.year),
                                                                              utc_time.month);
            numDays -= app_ai_record_rtc_calendar_month_length_calc(IsLeapYear(utc_time.year),
                                                                    utc_time.month);
            utc_time.month++;
        }

        utc_time.day = numDays + 1;
        utc_time.month++;
    }

    return utc_time;
}

static void app_ai_record_rtc_info_load(uint8_t *data, uint8_t length)
{
    ftl_load_from_storage(data, APP_RW_UTC_INFO_ADDR, length);
}

void app_ai_record_rtc_info_save(uint8_t *data, uint8_t length)
{
    T_UTC_TIME utc_time;
    memcpy(&utc_time, data, sizeof(utc_time));
    if (app_ai_record_rtc_calendar_check_utc_time(&utc_time))
    {
        ftl_save_to_storage(data, APP_RW_UTC_INFO_ADDR, length);
    }
    else
    {
        APP_PRINT_ERROR6("app_ai_record_rtc_info_save %d-%d-%d-%d-%d-%d",
                         utc_time.year, utc_time.month, utc_time.day,
                         utc_time.hour, utc_time.minutes, utc_time.seconds);
    }
}

bool app_ai_record_rtc_set_utc_time(T_UTC_TIME *utc_time)
{
    return rtc_calendar_set_utc_time(utc_time);
}

void app_ai_record_rtc_get_utc_time(T_UTC_TIME *utc_time)
{
    rtc_calendar_get_utc_time(utc_time);
}

void app_ai_record_rtc_get_utc_time_dec_val(T_UTC_TIME *p_utc_time, uint32_t *date, uint32_t *time)
{
    T_UTC_TIME utc_time;
    uint32_t date_dec = 0;
    uint32_t time_dec = 0;
    rtc_calendar_get_utc_time(&utc_time);
    APP_PRINT_TRACE6("app_ai_record_rtc_get_utc_time_oct_val %d-%d-%d-%d-%d-%d",
                     utc_time.year, utc_time.month, utc_time.day,
                     utc_time.hour, utc_time.minutes, utc_time.seconds);

    date_dec = utc_time.year * 10000 + utc_time.month * 100 + utc_time.day;
    time_dec = utc_time.hour * 10000 + utc_time.minutes * 100 + utc_time.seconds;

    *date = date_dec;
    *time = time_dec;
    *p_utc_time = utc_time;
}

bool app_ai_record_rtc_compare_utc_time(T_UTC_TIME *t1, T_UTC_TIME *t2)
{
    if (t1 == NULL || t2 == NULL)
    {
        return false;
    }

    if (t1->year != t2->year)
    {
        return t1->year > t2->year;
    }

    if (t1->month != t2->month)
    {
        return t1->month > t2->month;
    }

    if (t1->day != t2->day)
    {
        return t1->day > t2->day;
    }

    if (t1->hour != t2->hour)
    {
        return t1->hour > t2->hour;
    }

    if (t1->minutes != t2->minutes)
    {
        return t1->minutes > t2->minutes;
    }

    return t1->seconds > t2->seconds;
}

static void app_ai_record_rtc_callback(T_UTC_TIME *global_time)
{
    uint32_t time_stamp = rtc_calendar_get_timestamp();
    IO_PRINT_INFO7("app_ai_record_rtc_callback: year %d, month %d, day %d, hour %d, minute %d, second %d, time_stamp %d",
                   global_time->year, global_time->month,
                   global_time->day, global_time->hour,
                   global_time->minutes, global_time->seconds, time_stamp);
}

void app_ai_record_rtc_calendar_init(void)
{
    T_UTC_TIME defut_utc_time;

    app_ai_record_rtc_info_load((uint8_t *)&defut_utc_time, APP_RW_UTC_INFO_SIZE);
    if (app_ai_record_rtc_calendar_check_utc_time(&defut_utc_time) == false)
    {
        defut_utc_time.year = 2025;
        defut_utc_time.month = 05;
        defut_utc_time.day = 20;
        defut_utc_time.hour = 10;
        defut_utc_time.minutes = 41;
        defut_utc_time.seconds = 0;
    }

    rtc_calendar_register_callback(app_ai_record_rtc_callback);

    if (!rtc_calendar_int(&defut_utc_time, 10))
    {
        IO_PRINT_INFO0("app_ai_record_rtc_calendar_init: wrong defut_utc_time");
    }
}
#endif

#endif
