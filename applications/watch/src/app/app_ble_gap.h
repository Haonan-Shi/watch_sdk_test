/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _PERIPHERAL_APP__
#define _PERIPHERAL_APP__

#ifdef __cplusplus
extern "C" {
#endif
/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <app_msg.h>
#include <gap_le.h>
#include <gap_msg.h>
#include "bt_gatt_svc.h"
#include "ble_ext_adv.h"
#include "gap_conn_le.h"


/** @defgroup PERIPH_APP Peripheral Application
  * @brief Peripheral Application
  * @{
  */
/**  @brief  App define le link connection state status */
typedef enum
{
    LE_LINK_STATE_DISCONNECTED,
    LE_LINK_STATE_CONNECTING,
    LE_LINK_STATE_CONNECTED,
    LE_LINK_STATE_DISCONNECTING,
} T_LE_LINK_STATE;

typedef void (*P_HANDLE_DEVICE_STATE_CB)(T_GAP_DEV_STATE new_state, uint16_t cause);
typedef void (*P_HANDLE_CONN_STATE_CB)(uint8_t conn_id, T_GAP_CONN_STATE new_state,
                                       uint16_t disc_cause);
typedef void (*P_HANDLE_MTU_INFO_CB)(uint8_t conn_id, uint16_t mtu_size);
typedef void (*P_HANDLE_CONN_PARAM_CB)(uint8_t conn_id, uint8_t new_state, uint16_t cause);
typedef void (*P_HANDLE_AUTHEN_STATE_CB)(uint8_t conn_id, uint8_t new_state, uint16_t cause);

typedef struct
{
    P_HANDLE_DEVICE_STATE_CB  device_state_cb;
    P_HANDLE_CONN_STATE_CB    conn_state_cb;
    P_HANDLE_MTU_INFO_CB      mtu_info_cb;
    P_HANDLE_CONN_PARAM_CB    conn_param_cb;
    P_HANDLE_AUTHEN_STATE_CB  authen_state_cb;
} T_FUN_GAP_MSG_CBS;

typedef struct t_le_adv_conn
{
    uint8_t adv_handle;
    uint8_t conn_id;
    T_GAP_ROLE  role;
    uint8_t    remote_bd[GAP_BD_ADDR_LEN];
    T_BLE_EXT_ADV_MGR_STATE state;
    const T_FUN_GAP_MSG_CBS *cb_tbl;
} T_LE_ADV_CONN;

typedef struct t_adv_conn_queue_pkt
{
    struct t_adv_conn_queue_pkt    *p_next;
    T_LE_ADV_CONN  *adv_conn;
} T_ADV_CONN_QUEUE_PKT;


/*============================================================================*
 *                              Variables
 *============================================================================*/

/*============================================================================*
 *                              Functions
 *============================================================================*/

/**
 * @brief    All the application messages are pre-handled in this function
 * @note     All the IO MSGs are sent to this function, then the event handling
 *           function shall be called according to the MSG type.
 * @param[in] io_msg  IO message data
 * @return   void
 */
void app_handle_io_msg(T_IO_MSG io_msg);

/**
  * @brief Callback for gap le to notify app
  * @param[in] cb_type callback msy type @ref GAP_LE_MSG_Types.
  * @param[in] p_cb_data point to callback data @ref T_LE_CB_DATA.
  * @retval result @ref T_APP_RESULT
  */
T_APP_RESULT app_gap_callback(uint8_t cb_type, void *p_cb_data);

/**
  * @brief gap bt ble ready check
  * @param[in] void.
  * @param[in] void.
  * @return void
  */
void app_gap_bt_ble_ready_check(void);

/**
    * @brief  init ble mgr
    * @param  void
    * @return void
    */
void app_ble_gap_ble_mgr_init(void);

T_LE_ADV_CONN *app_get_adv_conn_hdr_by_conn_id(uint8_t conn_id);

bool app_ble_gap_msg_handle_register(T_LE_ADV_CONN *adv_conn);

/** End of PERIPH_APP
* @}
*/


#ifdef __cplusplus
}
#endif

#endif
