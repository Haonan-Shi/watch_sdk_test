/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef __PPG_RTL87X5_H__
#define __PPG_RTL87X5_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#include "stdint.h"
#include "stdbool.h"
#include "RTL87x5PPG.h"
#include "app_msg.h"


typedef struct
{
    uint8_t ppg_result_cnt;
    uint8_t ppg_raw_cnt;
    uint8_t acc_raw_cnt;
} Check_cnt_t;

typedef enum
{
    PPG_REAL_TIME_SEND_MODE = 1,//1:real time send data
    PPG_DATA_SAVE_MODE = 2,// 2:save data to flash
} T_PPG_SETTING_FLAG;

extern T_PPG_SETTING_FLAG ppg_setting_flag;

void hrs_driver_init(void);
void hrs_power_start_01(void);
void hrs_power_start_02(void);
Hrm_rsult_rem_t get_hrs_result(void);
void ppg_trans_event_handler(T_IO_MSG msg);
void hrs_ppg_int_event_handle(void);
void hrs_command_stop_event_handle(void);
void hrs_command_start_type1_event_handle(void);
void hrs_command_start_type2_event_handle(void);
void hrs_result_read_event(void);
#endif
