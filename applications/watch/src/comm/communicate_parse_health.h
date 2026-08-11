/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef __COMMUNICATE_PARSE_HEALTH_H__
#define __COMMUNICATE_PARSE_HEALTH_H__


#ifdef __cplusplus
extern "C" {
#endif



#include <stdbool.h>
#include "stdint.h"

typedef enum
{
    KEY_REQUEST_DATA   = 0x01,
    KEY_RETURN_SPORTS_DATA  = 0x02,
    KEY_RETURN_SLEEP_DATA  = 0x03,
    KEY_MORE    = 0x04,
    KEY_RETURN_SLEEP_SETTING = 0x05,
    KEY_SET_STEPS_NOTIFY = 0x06,
    KEY_DATA_SYNC_START = 0x07,
    KEY_DATA_SYNC_END = 0x08,
    KEY_DAILY_DATA_SYNC = 0x09,
    KEY_LATEST_DATA_SYNC = 0x0A,
    KEY_DAILY_DATA_CALIBRATION = 0x0B,
    KEY_DAILY_DATA_CALIBRATION_RETURN = 0x0C,
    KEY_REQUEST_HEART_DATA = 0x0D,
    KEY_HEART_DATA_SAMPLE_SETTING = 0x0E,
    KEY_HEART_DATA_RETURN = 0x0F,
    KEY_CANCEL_HEART_SAMPLE = 0x10,
    KEY_REQUEST_HEART_SAMPLE_SETTING = 0x11,
    KEY_RETURN_HEART_SAMPLE_SETTING = 0x12,

    KEY_PPG_TRANS_GET_STORED_DATA_NUM = 0x14,
    KEY_PPG_TRANS_EARASE_ALL_DATA     = 0x15,
    KEY_PPG_TRANS_GET_STORED_DATA     = 0x16,
    KEY_PPG_TRANS_SET_WATCH_TIME      = 0x17,
    KEY_PPG_TRANS_START_PPG_01        = 0x19,
    KEY_PPG_TRANS_TRANS_PPG_DATA      = 0x20,
    KEY_PPG_TRANS_TRANS_PPG_DATA_STOP = 0x21,
} HEALTH_KEY;

void resolve_HealthData_command(uint8_t key, const uint8_t *pValue, uint16_t length);



#ifdef __cplusplus
}
#endif

#endif //__COMMUNICATE_PARSE_HEALTH_H__
