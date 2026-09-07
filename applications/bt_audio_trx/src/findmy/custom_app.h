/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _CUSTOM_APP__
#define _CUSTOM_APP__

#ifdef __cplusplus
extern "C" {
#endif
/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include "app_msg.h"
#include "gap.h"

#if CONFIG_REALTEK_FINDMY_SUPPORT_CUSTOMIZED_APP
/*============================================================================*
 *                                Types
 *============================================================================*/
typedef struct
{
    bool cust_enable;        //customized function is enabled
    bool cust_paired;
    uint8_t cust_conn_id;
} T_CUSTOM_DATA;

/*============================================================================*
 *                              Variables
 *============================================================================*/

/*============================================================================*
 *                              Functions
 *============================================================================*/
bool cust_app_is_cust_link(uint8_t conn_id);
bool cust_app_is_cust_adv(void);
void cust_app_disconnect(void);
void cust_adv_init(void);
void cust_data_init(void);
void cust_factory_reset(void);
void cust_handle_connected_evt(uint8_t conn_id, uint8_t *remote_bd,
                               T_GAP_REMOTE_ADDR_TYPE remote_bd_type);
bool cust_handle_disconnected_evt(uint8_t conn_id, uint16_t disc_cause);
void cust_set_paired_flag(bool flag);
bool cust_is_paired(void);
bool cust_feature_is_enabled(void);
void cust_feature_enable(void);
void cust_feature_disable(void);
void cust_feature_enable_disable_set(void);
void cust_adv_update_device_name(bool suffix);
uint8_t cust_get_conn_id(void);
bool cust_adv_start(uint16_t duration_10ms);
bool cust_adv_stop(int8_t app_cause);
#endif

#ifdef __cplusplus
}
#endif

#endif

