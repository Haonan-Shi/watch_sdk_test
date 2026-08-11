/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _ALIPAY_TASK_H_
#define _ALIPAY_TASK_H_

#include <os_task.h>
#include "alipay_transit.h"

typedef enum
{
    TRANSIT_STATUS_INVALID,
    TRANSIT_STATUS_AGENT_DISABLE,
    TRANSIT_STATUS_AGENT_ENABLE,
    TRANSIT_STATUS_ALLOC_FAIL,
    //card list
    TRANSIT_STATUS_LIST_OTHER_ERR,
    TRANSIT_STATUS_ONLINE_SUCCESS,
    TRANSIT_STATUS_CARD_LIST_EMPTY,
    TRANSIT_STATUS_NETWORK_ERROR,
    TRANSIT_STATUS_BUFFER_TOO_SHORT,
    TRANSIT_STATUS_CARD_NUM_EXPIRED,
    //get transit code
    TRANSIT_STATUS_CARD_SUCCESS,
    TRANSIT_STATUS_CARD_SERVER_FAIL,
    TRANSIT_STATUS_CARD_DATA_LIMITED,
    TRANSIT_STATUS_CARD_BUF_TOO_SHORT,
    TRANSIT_STATUS_CARD_UNSUPPORTED,
    TRANSIT_STATUS_CARD_BUSCARDDATA_INVALID,//need network
    TRANSIT_STATUS_CARD_OTHER_UNKNOWN_ERROR,
} e_transit_task_status;

typedef struct
{
    uint8_t is_valid;
    uint8_t error_code;
    e_transit_task_status status;
    uint32_t card_num;
    alipay_tansit_CardBaseVO_t *p_card_list;

} t_alipay_transit_list;

typedef struct
{
    uint8_t is_valid;
    uint32_t index;
    uint32_t transitCode_len;
    uint8_t *p_transitCode;
    e_transit_task_status status;
    alipay_tansit_CardBaseVO_t *p_default_card;
} t_alipay_transitCode;

typedef enum
{
    ALIPAY_MSG_INVALID,
    ALIPAY_MSG_GET_BIND_STATUS,
    ALIPAY_MSG_GET_BIND_STRING,
    ALIPAY_MSG_GET_PAYCODE,
    ALIPAY_MSG_GET_DEFAULT_TRANSIT_LIST,
    ALIPAY_MSG_GET_CARD_LIST_ONLINE,
    ALIPAY_MSG_GET_CARD_LIST_OFFLINE,
    ALIPAY_MSG_GET_DEFAULT_TRANSIT_CODE,
    ALIPAY_MSG_GET_TRANSIT_CODE,
    ALIPAY_MSG_TRANSIT_CODE_CHECK_UPDATE,
    ALIPAY_MSG_TRANSIT_SLIENT_UPDATE,
    ALIPAY_MSG_DEVICE_UNBIND,
} e_alipay_task_msg;

typedef void (*pfunc)(e_alipay_task_msg e, void *data);
typedef struct
{
    e_alipay_task_msg type;
    union
    {
        uint32_t  data;
        void     *buf;

    } u;
    pfunc    func;
} T_ALIPAY_MSG;

typedef struct
{
    binding_status_e status;
    uint8_t binded;
    bool triggle;
} alipay_device_status_;

void alipay_task_init(void);
bool alipay_send_msg_to_alipay_task(T_ALIPAY_MSG *p_msg);
bool alipay_task_get_transit_card_list(t_alipay_transit_list *p_transit_stg);
bool alipay_task_get_transit_online_card_list(t_alipay_transit_list *p_transit_stg);
bool alipay_task_get_transitCode(uint32_t index, t_alipay_transitCode *p_transitCode);
bool alipay_task_get_default_transit_code(t_alipay_transitCode *p_transitCode);
uint8_t alipay_task_check_local_card_list_exist(void);
uint8_t alipay_task_transit_slient_update(void);
uint8_t alipay_task_unbind_device(void);
alipay_device_status_ *alipay_task_get_bind_status(void);
uint8_t *alipay_task_get_paycode_status(void);

#endif//_ALIPAY_TASK_H_
