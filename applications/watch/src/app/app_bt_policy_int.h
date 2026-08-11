/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_BT_POLICY_INT_H_
#define _APP_BT_POLICY_INT_H_

#include <stdint.h>
#include <stdbool.h>
#include "gap_br.h"
#include "btm.h"
#include "app_linkback.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define STOP_ADV_CAUSE                        (0x31)
#define FE_MP_RSSI                            (-60)
#define FE_TO_MIN                             (1)
#define FE_TO_UNIT                            (5000)
#define PEER_VALID_MAGIC                      (0x1234)

typedef enum
{
    STATE_INIT                                = 0x00, /* connection not allowed */
    STATE_STANDBY                             = 0x01, /* connection enable and no connection currently */
    STATE_LINKBACK                            = 0x02, /* linkback running */
    STATE_CONNECTED_PHONE                     = 0x03, /* connected a phone */
    STATE_CONNECTED_EARPHONE                  = 0x04, /* connected a earphone */
    STATE_CONNECTED_TWO                       = 0x05, /* connected a phone and a earphone */
} T_STATE;

typedef enum
{
    EVENT_BT_STARTUP                          = 0x00,
    EVENT_LINKBACK_START                      = 0x01,
    EVENT_LINKBACK_STOP                       = 0x02,
    EVENT_BT_CONN                             = 0x03,
    EVENT_BT_CONN_FAIL                        = 0x04,
    EVENT_BT_DISCONN                          = 0x05,
    EVENT_BT_IDLE                             = 0x06,
} T_EVENT;

typedef enum
{
    TIMER_ID_SHUTDOWN_STEP                    = 0x00,
    TIMER_ID_FIRST_ENGAGE_CHECK               = 0x01,
    TIMER_ID_PAIRING_TIMEOUT                  = 0x03,
    TIMER_ID_DISCOVERABLE_TIMEOUT             = 0x04,
    TIMER_ID_ROLE_SWITCH                      = 0x05,
    TIMER_ID_BUD_LINKLOST_TIMEOUT             = 0x06,
    TIMER_ID_LINKBACK_TIMEOUT                 = 0x07,
    TIMER_ID_LINKBACK_DELAY                   = 0x09,
    TIMER_ID_LATER_AVRCP                      = 0x0a,
    TIMER_ID_LATER_COUPLING                   = 0x0b,
} T_TIMER;

typedef enum
{
    STABLE_ENTER_MODE_NORMAL                  = 0x00,
    STABLE_ENTER_MODE_AGAIN                   = 0x01,
    STABLE_ENTER_MODE_DEDICATED_PAIRING       = 0x02,
    STABLE_ENTER_MODE_NOT_PAIRING             = 0x03,
    STABLE_ENTER_MODE_DIRECT_PAIRING          = 0x04,
} T_STABLE_ENTER_MODE;

typedef enum
{
    SHUTDOWN_STEP_BEGIN                       = 0x00,
    SHUTDOWN_STEP_START_DISCONN_B2S_PROFILE   = 0x10,
    SHUTDOWN_STEP_WAIT_DISCONN_B2S_PROFILE    = 0x11,
    SHUTDOWN_STEP_START_DISCONN_B2S_LINK      = 0x20,
    SHUTDOWN_STEP_WAIT_DISCONN_B2S_LINK       = 0x21,
    SHUTDOWN_STEP_START_DISCONN_B2B_LINK      = 0x30,
    SHUTDOWN_STEP_WAIT_DISCONN_B2B_LINK       = 0x31,
    SHUTDOWN_STEP_END                         = 0x40,
} T_SHUTDOWN_STEP;

typedef struct
{
    uint16_t last_src_conn_idx;
    uint16_t link_src_conn_idx;
} T_BP_ROLESWAP_INFO;

typedef enum
{
    REMOTE_MSG_ROLESWAP_INFO                  = 0x00,
    REMOTE_MSG_PRI_BP_STATE                   = 0x01,
    REMOTE_MSG_PRI_REQ                        = 0x02,
} T_REMOTE_MSG;

typedef struct
{
    uint8_t num_max;
    uint8_t num;
} T_B2S_CONNECTED;


typedef struct
{
    uint8_t *bd_addr;
    bool is_b2b;
    uint32_t prof;
    uint16_t cause;
    uint8_t bud_role;
    T_BT_LINK_KEY_TYPE key_type;
    uint8_t *link_key;
    T_BT_SDP_ATTR_INFO *sdp_info;
    bool is_special;
    T_LINKBACK_SEARCH_PARAM *search_param;
    bool check_bond_flag;
    bool not_check_addr_flag;
    bool is_visiable;
    bool is_connectable;
    bool is_force;
    bool is_suc;
    bool is_later_avrcp;
    bool is_multi_step;
} T_BT_PARAM;

typedef enum
{
    LINKBACK_SCENARIO_HF_A2DP_LAST_DEVICE,
    LINKBACK_SCENARIO_HFP_BASE,
    LINKBACK_SCENARIO_A2DP_BASE,
    LINKBACK_SCENARIO_SPP_BASE,
    LINKBACK_SCENARIO_HID_BASE,
    LINKBACK_SCENARIO_TOTAL,
} T_LINKBACK_SCENARIO;

typedef enum
{
    ROLESWITCH_EVENT_LINK_CONNECTED      = 0x00,
    ROLESWITCH_EVENT_LINK_ACTIVE         = 0x01,
    ROLESWITCH_EVENT_LINK_DISCONNECTED   = 0x02,
    ROLESWITCH_EVENT_FAIL_RETRY          = 0x03,
    ROLESWITCH_EVENT_FAIL_RETRY_MAX      = 0x04,
    ROLESWITCH_EVENT_ROLE_CHANGED        = 0x05,
} T_ROLESWITCH_EVENT;

typedef enum
{
    PRI_REQ_LET_SEC_TO_DISCONN           = 0x01,
} T_PRI_REQ;

typedef struct
{
    uint8_t num_phone;
    uint8_t phone_addr[6];
    uint8_t num_bud;
    uint8_t bud_addr[6];
} T_BT_CONNECTED;

uint32_t get_profs_by_bond_flag(uint32_t bond_flag);
void app_bt_policy_event_handle(T_EVENT event, void *param);
void app_bt_policy_enter_state(T_STATE state);
void app_bt_policy_discon_all_profiles_by_addr(uint8_t *bd_addr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _APP_BT_POLICY_H_ */
