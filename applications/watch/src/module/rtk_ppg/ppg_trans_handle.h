/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef __PPG_TRANS_HANDLE_H__
#define __PPG_TRANS_HANDLE_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#include "stdint.h"
#include "app_msg.h"
#include "rtl876x.h"

typedef enum
{
    PPG_TRANS_FLASH_ERASE = 0,
} T_PPG_TRANS_MSG_EVENT;

typedef enum
{
    T_STATE_IDLE = 0x00,
    T_STATE_BUSY,
} T_REPORT_STATE;

typedef enum
{
    T_REPORT_TO = 0x00,
    T_TEST_TO = 0x01,
} T_REPORT_TIMER_ID;

typedef enum
{
    STOP_FLAG = 0,//0:receive cmd from phone to STOP real time send
    START_FLAG = 1,//1:receive cmd from phone to START real time send
} T_PPG_SEND_DATA;

typedef struct t_ppg_data_header
{
    struct t_ppg_data_header     *p_next;
    uint16_t   payload_length;
    uint16_t   frame_num;
    uint8_t    p_data[0];
} T_PPG_DATA_HEADER;

extern T_PPG_SEND_DATA ppg_data_send_flag;

void *ppg_data_peek(int offset);
uint8_t ppg_data_flush(uint16_t cnt);
bool ppg_data_in(uint8_t *packet_pt, uint16_t packet_length, uint16_t frame_num);
void resolve_hrs_setting_command(uint8_t key, const uint8_t *pValue, uint16_t length);
void app_ppg_trans_report_flash_data(void);
void ppg_trans_init(void);
void ppg_send_data(uint8_t conn_id, uint16_t event_id, uint8_t cmd_type,
                   uint16_t len, uint8_t *data);
#endif

