/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_AI_RECORD_RTC_H_
#define _APP_AI_RECORD_RTC_H_

#include "stdint.h"
#include "rtc_calendar.h"

T_UTC_TIME app_ai_record_rtc_gps_to_date(uint32_t gps_week, uint32_t gps_seconds);
void app_ai_record_rtc_info_save(uint8_t *data, uint8_t length);

bool app_ai_record_rtc_set_utc_time(T_UTC_TIME *utc_time);

void app_ai_record_rtc_get_utc_time(T_UTC_TIME *utc_time);

void app_ai_record_rtc_get_utc_time_dec_val(T_UTC_TIME *p_utc_time, uint32_t *date, uint32_t *time);

bool app_ai_record_rtc_compare_utc_time(T_UTC_TIME *t1, T_UTC_TIME *t2);

void app_ai_record_rtc_calendar_init(void);

/** @} */ /* End of group _APP_AI_RECORD_RTC_H_ */
#endif
