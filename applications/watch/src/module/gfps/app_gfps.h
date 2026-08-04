/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_GFPS_H_
#define _APP_GFPS_H_

#if CONFIG_REALTEK_GFPS_FEATURE_SUPPORT
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "btm.h"
#include "gfps.h"
#include "remote.h"
#include "os_queue.h"
#include "ble_ext_adv.h"
#include "app_ble_gap.h"

/** @defgroup APP_RWS_GFPS App Gfps
  * @brief App Gfps
  * @{
  */
typedef enum
{
    GFPS_LOCATOR_TRACKER      = 1,
    GFPS_WATCH                = 146,
    GFPS_HEADPHONES           = 149,
    GFPS_EARPHONES            = 150,
} T_GFPS_DEVICE_TYPE;

typedef struct
{
    bool is_gfps_pairing;
    bool gfps_raw_passkey_received;
    bool le_bond_confirm_pending;
    bool edr_bond_confirm_pending;
    bool auth_param_change;

    uint8_t  io_cap;
    uint16_t auth_flags;
    uint32_t gfps_raw_passkey;
    uint32_t le_bond_passkey;
    uint32_t edr_bond_passkey;
    uint8_t  edr_bond_bd_addr[6];

    uint8_t          gfps_conn_id;
    uint16_t         gfps_msg_cid;
    T_GAP_CONN_STATE gfps_conn_state;

    uint8_t  *p_gfps_cmd;
    uint16_t gfps_cmd_len;
} T_GFPS_LINK;

typedef enum
{
    GFPS_ACTION_IDLE,
    GFPS_ACTION_ADV_DISCOVERABLE_MODE_WITH_MODEL_ID,
    GFPS_ACTION_ADV_NOT_DISCOVERABLE_MODE,
    GFPS_ACTION_ADV_NOT_DISCOVERABLE_MODE_HIDE_UI,
} T_GFPS_ACTION;

typedef struct
{
    T_GFPS_ACTION              gfps_curr_action;

    T_SERVER_ID                gfps_id;
    uint8_t                    current_conn_id;
    T_GFPS_BATTERY_INFO        gfps_battery_info;
    uint8_t                    random_address[6];
    bool                       gfps_linkback_init;
    bool                       force_enter_pair_mode;

    bool                       is_gfps_additional_pairing;
    uint8_t                    additional_default_io_cap;
    uint8_t                    additional_default_auth_flags;
    uint8_t                    gfps_additional_conn_id;
    bool                       gfps_additional_passkey_from_gfps_received;
    bool                       gfps_additional_passkey_from_gfps_pending;
    uint8_t                    gfps_additional_bond_addr[6];
    uint32_t                   gfps_additional_passkey_from_stack;
    uint32_t                   gfps_additional_passkey_from_gfps;
    T_GFPS_PASSKEY_STATUS_CODE status_code;
    uint8_t                    failure_count;
    bool                       receive_pair_req;
    bool                       receive_passkey;
    bool                       allow_write_account_key;
    bool                       is_gfps_retroactive_pairing;
    uint8_t                    account_key_write_counts;
    //T_GFPS_ADDITIONAL_BOND_TYPE gfps_additional_bond_type;
} T_GFPS_DB;

typedef enum
{
    GFPS_KEY_FORCE_ENTER_PAIR_MODE        = 0x00,
    GFPS_LE_DISCONN_FORCE_ENTER_PAIR_MODE = 0x01,
    GFPS_EXIT_PAIR_MODE                   = 0x02,
    GFPS_BT_POLICY_FORCE_EXIT_PAIR_MODE   = 0x03,
    GFPS_BT_POLICY_FORCE_ENTER_PAIR_MODE  = 0x04,

} T_GFPS_FORCE_ENTER_PAIR_MODE;

extern T_LE_ADV_CONN gfps_adv;

/**
 * @brief google Fast pair initialize
 *
 * @param void
 * @return void
 */
void app_gfps_init(void);

/**
 * @brief gfps adv initialize
 *
 */
void app_gfps_adv_init(void);

/**
 * @brief get gfps adv state
 *
 * @param void
 * @return gfps adv state
 */
T_BLE_EXT_ADV_MGR_STATE app_gfps_adv_get_state(void);

/**
 * @brief get gfps adv handle
 *
 * @param void
 * @return gfps adv handle
 */
uint8_t app_gfps_adv_get_handle(void);

/**
 * @brief get gfps adv curr action
 *
 * @param void
 * @return gfps adv cur acton
 */
T_GFPS_ACTION app_gfps_adv_get_curr_action(void);

/**
 * @brief start gfps adv according to expected gfps action
 *
 * @param gfps_next_action @ref T_GFPS_ACTION
 * @return true  success
 * @return false fail
 */
bool app_gfps_next_action(T_GFPS_ACTION gfps_next_action);

/**
 * @brief get resolvable private address used by gfps adv
 *
 * @param random_bd random reslovable privacy address
 * @return void
 */
void app_gfps_get_random_addr(uint8_t *random_bd);

/**
 * @brief update gfps adv interval
 *
 * default interval is:
 * in pairing mdoe : app_gfps_cfg.gfps_discov_adv_interval;
 * not in pairing mode: app_gfps_cfg.gfps_not_discov_adv_interval;
 *
 * @param adv_interval Range:[32,400], Unit:0.625ms
 * @return T_GAP_CAUSE
 */
T_GAP_CAUSE app_gfps_adv_update_adv_interval(uint32_t adv_interval);

/**
 * @brief handle user confirmation for legacy pairing.
 *
 * @param confirm_req  bt user confirm request
 * @return void
 */
void app_gfps_handle_bt_user_confirm(T_BT_EVENT_PARAM_LINK_USER_CONFIRMATION_REQ confirm_req);

/**
 * @brief handle user confirmation for le pairing.
 *
 * @param conn_id BLE connection id
 * @return void
 */
void app_gfps_handle_ble_user_confirm(uint8_t conn_id);

/**
 * @brief enter non discoverable mode
 *
 * @param void
 * @return void
 */
void app_gfps_enter_nondiscoverable_mode(void);

/**
 * @brief app_gfps_device_handle_b2s_ble_bonded
 *
 * @param conn_id                 le connection id
 * @param p_remote_identity_addr  remote identity address
 */
void app_gfps_handle_b2s_ble_bonded(uint8_t conn_id, uint8_t *p_remote_identity_addr);

/**
 * @brief gfps set ble connection parameter
 *
 * @param conn_id ble connection id
 */
void app_gfps_set_ble_conn_param(uint8_t conn_id);

/**
 * @brief start gfps adv and le audio adv for gfps pairing
 *
 * @param device_role    bud role @ref T_REMOTE_SESSION_ROLE
 * @return void
 */
void app_gfps_le_device_adv_start(void);

/**
 * @brief gfps handle ble connection information
 *
 * @param remote_bd_addr   remote bd address
 * @param remote_addr_type remote bd address type
 * @return true  success
 * @return false fail
 */
bool app_gfps_ble_conn_info_handle(uint8_t *remote_bd_addr, uint8_t remote_addr_type);

/**
 * @brief gfps le linkback information init
 *
 * @param conn_id le connection id
 */
void app_gfps_linkback_info_init(uint8_t conn_id);

/**
 * @brief gfps force enter pairing mode
 *
 * @param status @ref T_GFPS_FORCE_ENTER_PAIR_MODE
 */
void app_gfps_force_enter_pairing_mode(uint8_t status);

/**
 * @brief gfps update resolvable private address
 *
 * @param gfps_generate_rpa if gfps_generate_rpa is true
 *                         gfps will generate a valid rpa and set this rpa into stack, and update this rpa in gfps.
 *                         otherwise:
 *                         directly get rpa from stack, and update this rpa in gfps.
 *
 */
void app_gfps_update_rpa(bool gfps_generate_rpa);

void app_gfps_reset_failure_count(void);

void app_gfps_check_receive_pair_req(void);

void app_gfps_check_receive_passkey(void);

void app_gfps_set_allow_write_account_key(bool allow);

void app_gfps_reset_account_key_write_counts(void);

void app_gfps_get_ble_addr(uint8_t *ble_addr);

bool app_gfps_get_force_enter_pair_mode_state(void);
/** End of APP_RWS_GFPS
* @}
*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
#endif
