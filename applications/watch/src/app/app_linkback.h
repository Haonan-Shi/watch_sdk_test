/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _BT_LINKBACK_H_
#define _BT_LINKBACK_H_

#include <stdint.h>
#include <stdbool.h>
#include "btm.h"
#include "app_bond.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @defgroup APP_LINKBACK App Linkback
  * @brief App Linkback
  * @{
  */

typedef union
{
    bool is_source;
    bool is_target;
} T_LINKBACK_SEARCH_PARAM;

typedef struct
{
    uint16_t protocol_version;
    uint8_t server_channel;
    uint8_t local_server_chann;
    bool feature;
} T_LINKBACK_CONN_PARAM;

typedef struct
{
    uint16_t conn_retry_timeout;
    uint16_t conn_retry_cnt;
    uint16_t prof_retry_timeout;
    uint16_t prof_retry_cnt;
    uint16_t delay_timeout;
} T_LINKBACK_RETRY_PARAM;

typedef enum
{
    LINKBACK_IDLE,
    LINKBACK_CONNECT_ACL,
    LINKBACK_CONNECT_PROFILE,
} T_LINKBACK_STATUS;

typedef struct
{
    uint8_t bd_addr[6];
    uint32_t plan_profs;
    bool is_force;
    bool is_special;
    T_LINKBACK_SEARCH_PARAM search_param;
    bool check_bond_flag;
    T_LINKBACK_RETRY_PARAM user_set_retry_param;
    T_DEVICE_TYPE device_type;
} T_LINKBACK_NODE;

typedef struct st_linkback_node_item
{
    struct st_linkback_node_item *next;
    bool is_used;
    T_LINKBACK_NODE linkback_node;
} T_LINKBACK_NODE_ITEM;

typedef struct
{
    T_LINKBACK_NODE_ITEM *head;
    T_LINKBACK_NODE_ITEM *tail;
} T_LINKBACK_TODO_QUEUE;

typedef struct
{
    T_LINKBACK_NODE linkback_node;
    bool is_valid;
    bool is_exit;
    bool timecb_call_retry;
    uint32_t remain_profs;
    uint32_t doing_prof;
    bool is_sdp_ok;
    uint8_t prof_retry_cnt;
    uint8_t conn_retry_cnt;
    T_LINKBACK_CONN_PARAM linkback_conn_param;
    T_LINKBACK_STATUS linkback_sts;
} T_LINKBACK_ACTIVE_NODE;

typedef enum
{
    CANCEL_LINKBACK_BY_ADDR,
    CANCEL_LINKBACK_BY_TYPE,
} T_CANCEL_LINKBACK_MODE;

typedef struct
{
    T_CANCEL_LINKBACK_MODE mode;
    union
    {
        uint8_t addr[6];
        T_DEVICE_TYPE type;
    } u;
} T_CANCEL_LINKBACK_MSG;

bool linkback_profile_search_start(uint8_t *bd_addr, uint32_t prof, bool is_special,
                                   T_LINKBACK_SEARCH_PARAM *param);
bool linkback_profile_connect_start(uint8_t *bd_addr, uint32_t prof, T_LINKBACK_CONN_PARAM *param);
void linkback_profile_disconnect_start(uint8_t *bd_addr, uint32_t profs);

void linkback_todo_queue_init(void);
T_LINKBACK_NODE_ITEM *linkback_todo_queue_malloc_node_item(void);
void linkback_todo_queue_free_node_item(T_LINKBACK_NODE_ITEM *p_item);
void linkback_todo_queue_insert_normal_node(uint8_t *bd_addr, uint32_t plan_profs,
                                            T_DEVICE_TYPE device_type, T_LINKBACK_RETRY_PARAM retry_param);
void linkback_todo_queue_insert_force_node(uint8_t *bd_addr, uint32_t plan_profs,
                                           bool is_special, T_LINKBACK_SEARCH_PARAM *search_param, bool check_bond_flag,
                                           T_DEVICE_TYPE device_type, T_LINKBACK_RETRY_PARAM retry_param);

bool linkback_todo_queue_take_first_node(T_LINKBACK_NODE *node);
void linkback_todo_queue_remove_plan_profs(uint8_t *bd_addr, uint32_t plan_profs);
void linkback_todo_queue_delete_all_node(void);
void linkback_active_node_init(void);
void linkback_active_node_load_doing_prof(void);
void linkback_active_node_load(T_LINKBACK_NODE *node);
void linkback_active_node_step_suc_adjust_remain_profs(void);
void linkback_active_node_step_fail_adjust_remain_profs(void);
void linkback_active_node_src_conn_fail_adjust_remain_profs(void);
void linkback_active_node_remain_profs_add(uint32_t profs, bool check_bond_flag);
void linkback_active_node_remain_profs_sub(uint32_t profs);
bool linkback_active_node_judge_cur_conn_addr(uint8_t *bd_addr);
bool linkback_active_node_judge_cur_conn_prof(uint8_t *bd_addr, uint32_t prof);

bool linkback_load_bond_list(T_LINKBACK_RETRY_PARAM retry_param);
bool linkback_check_bond_flag(uint8_t *bd_addr, uint32_t prof);

bool linkback_check_br_link_connected_profile(uint8_t *bd_addr, uint32_t *profs);
void linkback_handle_sdp_attr_info(T_BT_EVENT_PARAM *param);
void linkback_handle_sdp_discov_cmpl(T_BT_EVENT_PARAM *param);
void linkback_handle_profile_conn(uint8_t *bd_addr, uint32_t prof);
bool linkback_run(void);
void linkback_stop(void);
/**
    * @brief  linkback_create_connection.
    * @param  bd_addr: bd addr to create connection
    * @param  profile_mask: profiles to connect
    * @param  device_type: the device is phone or ear
    * @param  retry_param:
              conn_retry_timeout; //retry interval after the acl connect fail event
              conn_retry_cnt; //max retry count for create acl link
              prof_retry_timeout; //retry interval after the profile connect fail event
              prof_retry_cnt; //max retry count for create profile connect
              delay_timeout; //create connect will actually start after the delay timeout
    * @return void
    */
void linkback_create_connection(uint8_t *bd_addr, uint32_t profile_mask, T_DEVICE_TYPE device_type,
                                T_LINKBACK_RETRY_PARAM retry_param);

void linkback_phone_create_connection(uint8_t *bd_addr);
void linkback_earphone_create_connection(uint8_t *bd_addr);
/**
    * @brief  linkback_cancel_connection.
    * @param  cancel_current_node: cancel the linkback active node
    * @param  delete_list_node: delete nodes in linkback list
    * @return void
    */
void linkback_cancel_connection(bool cancel_current_node, bool delete_list_node);
void linkback_cancel_connection_by_addr(uint8_t *bd_addr);
void linkback_cancel_connection_by_device_type(T_DEVICE_TYPE device_type);
void linkback_todo_queue_all_node(void);
void linkback_init(void);

/** End of APP_LINKBACK
* @}
*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
