/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _TP_CTL_H_
#define _TP_CTL_H_

#include <stdint.h>
#include "profile_server.h"

extern T_SERVER_ID tp_svc_id;

extern uint8_t para_mode;
extern uint8_t leng_mode;
extern uint8_t phy_mode;


#define TEST_MODE_INVALID   0
#define TEST_MODE_TX        1
#define TEST_MODE_RX        2
#define TEST_MODE_TRX       3

#define UPDATE_PARAM_RESULT_INVALID             0
#define UPDATE_PARAM_RESULT_REJECT              1
#define UPDATE_PARAM_RESULT_ACCEPT_NO_CHANGE    2
#define UPDATE_PARAM_RESULT_ACCEPT_CHANGE       3

#define UPDATE_DATA_LEN_RESULT_INVALID             0
#define UPDATE_DATA_LEN_RESULT_NOT_SUPPORT         1
#define UPDATE_DATA_LEN_RESULT_ACCEPT_NO_CHANGE    2
#define UPDATE_DATA_LEN_RESULT_ACCEPT_CHANGE       3

#define UPDATE_PHY_RESULT_INVALID           0
#define UPDATE_PHY_RESULT_1M                1
#define UPDATE_PHY_RESULT_2M                2

void tp_data_init(void);
void tp_update_complete(uint16_t conn_handle, uint16_t cid, uint16_t credit);
void tp_recv_ct_rx(uint16_t conn_handle, uint16_t cid,  uint8_t *p_value, uint16_t length);
void tp_recv_dt_rx(uint16_t conn_handle, uint16_t cid,  uint8_t *p_value, uint16_t length);
void tp_update_conn_para_callback(uint16_t conn_handle, uint16_t cid);

#endif
