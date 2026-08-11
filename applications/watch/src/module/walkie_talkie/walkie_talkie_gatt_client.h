/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Define to prevent recursive inclusion */
#ifndef _WALKIE_TALKIE_GATT_CLIENT_H_
#define _WALKIE_TALKIE_GATT_CLIENT_H_

#ifdef  __cplusplus
extern "C" {
#endif      /* __cplusplus */

/* Add Includes here */
#include <stdint.h>
#include <stdbool.h>
#include "bt_gatt_client.h"

/*============================================================================*
 *                         Macros
 *============================================================================*/

#define GATT_MSG_WTS_CLIENT_DIS_DONE                                  0x00
#define GATT_MSG_WTS_CLIENT_CONTROL_POINT_NOTIFY                      0x01
#define GATT_MSG_WTS_CLIENT_NOTIFY                                    0x02
#define GATT_MSG_WTS_CLIENT_WRITE_RESULT                              0x03
#define GATT_MSG_WTS_CLIENT_CONTROL_POINT_CCCD_CFG_RESULT             0x04
#define GATT_MSG_WTS_CLIENT_NOTIFY_CCCD_CFG_RESULT                    0x05

/*============================================================================*
 *                         Types
 *============================================================================*/
/**
 * @brief P_FUN_WTS_CLIENT_APP_CB BAS Client Callback Function Point Definition Function
 *        pointer used in WTS client module, to send events to specific client module.
 *        The events @ref WTS_CLT_CB_MSG.
 */
typedef T_APP_RESULT(*P_FUN_WTS_CLIENT_APP_CB)(uint16_t conn_handle, uint8_t type, void *p_data);

/** @brief WTS client discovery result
 *         The message data for GATT_MSG_WTS_CLIENT_DIS_DONE.
*/
typedef struct
{
    bool     is_found;
    bool     load_from_ftl;
    uint8_t  srv_instance_num;
} T_WTS_CLIENT_DIS_DONE;

/** @brief WTS client configure cccd data info
 *         The message data for GATT_MSG_WTS_CLIENT_CCCD_CFG_RESULT.
*/
typedef struct
{
    uint8_t    srv_instance_id;
    uint16_t   cause;
    bool       enable;
} T_WTS_CLIENT_CCCD_CFG_RESULT;
/** End of WTS_CLIENT_Exported_Types
* @}
*/


/*============================================================================*
 *                         Functions
 *============================================================================*/

/**
  * @brief      Add wts client to application.
  * @param[in]  app_cb   Pointer of app callback function to handle specific client module data.
  * @return Result of add the specific client module.
  * @retval true  Add client module success.
  * @retval false Add client module failed.
  */
bool wts_client_init(P_FUN_WTS_CLIENT_APP_CB app_cb);

/**
  * @brief      Config the WTS service control point CCCD data.
  * @param[in]  conn_handle     Connection handle of the ACL link.
  * @param[in]  srv_instance_id Service instance id.
  * @param[in]  enable          Whether to enable CCCD.
  * \arg    true    Enable CCCD.
  * \arg    false   Disable CCCD.
  * @return Result of config the WTS service CCCD data.
  * @retval true  Config the WTS service CCCD data success.
  * @retval false Config the WTS service CCCD data failed.
  */
bool wts_client_cfg_control_point_cccd(uint16_t conn_handle, uint8_t srv_instance_id, bool enable);

/**
  * @brief      Config the WTS service notify CCCD data.
  * @param[in]  conn_handle     Connection handle of the ACL link.
  * @param[in]  srv_instance_id Service instance id.
  * @param[in]  enable          Whether to enable CCCD.
  * \arg    true    Enable CCCD.
  * \arg    false   Disable CCCD.
  * @return Result of config the WTS service CCCD data.
  * @retval true  Config the WTS service CCCD data success.
  * @retval false Config the WTS service CCCD data failed.
  */
bool wts_client_cfg_notify_cccd(uint16_t conn_handle, uint8_t srv_instance_id, bool enable);

/**
  * @brief  Used by application, to write voice data characteristic value.
  * @param[in]  conn_handle  Connection handle of the ACL link.
  * @param[in]  buf  data buf for voice data.
  * @param[in]  len  data length for voice data.
  * @retval true send write command to upper stack success.
  * @retval false send write command to upper stack failed.
  */
bool wts_client_write_voice_data(uint16_t conn_handle, uint8_t *buf, uint16_t len);

/**
  * @brief  Used by application, to write control point data characteristic value.
  * @param[in]  conn_handle  Connection handle of the ACL link.
  * @param[in]  buf  data buf for control point data.
  * @param[in]  len  data length for control point data.
  * @retval true send write command to upper stack success.
  * @retval false send write command to upper stack failed.
  */
bool wts_client_write_control_point_data(uint16_t conn_handle, uint8_t *buf, uint16_t len);


#ifdef  __cplusplus
}
#endif      /*  __cplusplus */

#endif  /* _WALKIE_TALKIE_GATT_CLIENT_H_ */
