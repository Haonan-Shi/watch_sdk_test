/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _WALKIE_TALKIE_APP_H_
#define _WALKIE_TALKIE_APP_H_

#include "rtl876x.h"
#include "app_msg.h"
#include "gap.h"

#define WALKIE_TALKIE_DEV_MAX_CNT 3

#define EVENT_BUS_TOPIC_WALKIE_TALKIE_ALL_TOPIC             "wk/*"
#define EVENT_BUS_TOPIC_WALKIE_TALKIE_SCAN_REPORT           "wk/scan_rep"
#define EVENT_BUS_TOPIC_WALKIE_TALKIE_CONNECTED             "wk/conn"
#define EVENT_BUS_TOPIC_WALKIE_TALKIE_DISCONNECTED          "wk/disconn"
#define EVENT_BUS_TOPIC_WALKIE_TALKIE_USER_NAME             "wk/usrn"
#define EVENT_BUS_TOPIC_WALKIE_TALKIE_RECEIVE_START         "wk/rec_start"
#define EVENT_BUS_TOPIC_WALKIE_TALKIE_RECEIVE_STOP          "wk/rec_stop"
#define EVENT_BUS_TOPIC_WALKIE_TALKIE_DEV_INFO_UPDATE       "wk/diu"

typedef enum
{
    WALKIE_TALKIE_ADV   = 0,
    WALKIE_TALKIE_CONN  = 1,
} T_WALKIE_TALKIE_MODE;

typedef enum
{
    WALKIE_TALKIE_CONN_SLAVE   = 0,
    WALKIE_TALKIE_CONN_MASTER  = 1,
} T_WALKIE_TALKIE_CONN_ROLE;

typedef struct t_walkie_talkie_cfg
{
    T_WALKIE_TALKIE_MODE  mode;
    T_WALKIE_TALKIE_CONN_ROLE  role;
} T_WALKIE_TALKIE_CFG;

typedef enum
{
    WALKIE_TALKIE_CP_NAME = 0,
    WALKIE_TALKIE_CP_REMOTE_TRANSMIT_START = 1,
    WALKIE_TALKIE_CP_REMOTE_TRANSMIT_STOP = 2,
} T_WALKIE_TALKIE_CONTROL_POINT_OP_CODE;

typedef struct t_walkie_talkie_dev_info
{
    uint8_t addr[6];
    uint8_t dev_name[12];
} T_WALKIE_TALKIE_DEV_INFO;

typedef struct t_walkie_talkie_dev
{
    uint8_t dev_cnt;
    T_WALKIE_TALKIE_DEV_INFO dev_info[WALKIE_TALKIE_DEV_MAX_CNT];
    T_WALKIE_TALKIE_DEV_INFO connected_dev;
} T_WALKIE_TALKIE_DEV;

extern T_WALKIE_TALKIE_CFG  walkie_talkie_cfg;

void walkie_talkie_init(void);
void walkie_talkie_deinit(void);
void walkie_talkie_on(void);
void walkie_talkie_off(void);
void walkie_talkie_receive_start(void);
void walkie_talkie_receive_stop(void);
void walkie_talkie_transmit_start(void);
void walkie_talkie_transmit_stop(void);
T_APP_RESULT walkie_talkie_service_handle_cp_req(uint8_t conn_handle, uint16_t cid, uint16_t length,
                                                 uint8_t *p_value);
void walkie_talkie_transmit_start_send_to_ble(void);

void walkie_talkie_transmit_stop_send_to_ble(void);
void walkie_talkie_scan_report(void *data);
void walkie_talkie_save_dev_name(void *data);
void walkie_talkie_connect_dev(uint8_t dev_to_connect);

#endif /* _WALKIE_TALKIE_APP_H_ */
