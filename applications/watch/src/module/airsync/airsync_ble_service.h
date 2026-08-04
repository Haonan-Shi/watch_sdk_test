/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */
/**
  * *************************************************************************************
  * @file     airsync_ble_service.h
  * @brief    Header file for the WeChat 0xFEE7 GATT service.
  *
  *           This service is the single home of the WeChat 0xFEE7 Service,
  *           which carries two protocols defined by Tencent / WeChat:
  *
  *             1) Simple BLE Protocol- currently implemented.
  *                Characteristics:
  *                  0xFEA1  Pedometer Measurement  (Read | Notify)
  *                  0xFEA2  Pedometer Target       (Read | Write | Indicate)
  *                  0xFEC9  Device MAC             (Read)
  *
  *             2) AirSync Protocol - reserved for future extension.
  *                Characteristics (NOT YET implemented):
  *                  0xFEC7  AirSync Write    (Write)
  *                  0xFEC8  AirSync Indicate (Indicate)
  *                  0xFEC9  AirSync Read     (Read, conflicts with Device MAC)
  *
  *           The "airsync" naming is kept on purpose so that AirSync-specific
  *           characteristics can be appended to this same file later.
  *
  * @author   melody
  * @date     2025-9
  * @version  v0.2
  * *************************************************************************************
  */

/* Define to prevent recursive inclusion */
#ifndef _AIRSYNC_BLE_SERVICE_H_
#define _AIRSYNC_BLE_SERVICE_H_

#ifdef __cplusplus
extern "C"  {
#endif      /* __cplusplus */

/* Add Includes here */
#include "bt_gatt_svc.h"

/** @addtogroup RTK_Profile_Module RTK Profile Module
  * @{
  */

/** Reserved feature flag (legacy). */
#define AIRSYNC_READ_SOM                                    1

///@cond
/*-----------------------------------------------------------------------------
 * UUIDs
 *---------------------------------------------------------------------------*/

/** AirSync (WeChat 0xFEE7) primary service UUID. */
#define GATT_UUID_AIRSYNC_SERVICE                           0xFEE7

/* ---- Simple BLE Protocol characteristic UUIDs ----------------------------*/
/** 0xFEA1 Pedometer Measurement (Read | Notify). */
#define GATT_UUID_AIRSYNC_MEASUREMENT                       0xFEA1
/** 0xFEA2 Pedometer Target      (Read | Write | Indicate). */
#define GATT_UUID_AIRSYNC_TARGET                            0xFEA2
/** 0xFEC9 Device MAC            (Read).
 *  NOTE: Same UUID is also used by AirSync "Read" characteristic; the two
 *        cannot coexist in the same service instance. */
#define GATT_UUID_AIRSYNC_MAC                               0xFEC9

/* ---- AirSync Protocol characteristic UUIDs (reserved) --------------------*/
/* #define GATT_UUID_AIRSYNC_WRITE                          0xFEC7  */
/* #define GATT_UUID_AIRSYNC_INDICATE                       0xFEC8  */
/* #define GATT_UUID_AIRSYNC_READ                           0xFEC9  */ /* shared with MAC */

/*-----------------------------------------------------------------------------
 * Attribute table indices
 *
 *   Layout of airsync_ble_service_tbl[] in airsync_ble_service.c :
 *
 *     index 0 : Primary Service Declaration (0xFEE7)
 *     --- Simple BLE Protocol ---
 *     index 1 : Char Decl   - Pedometer Measurement
 *     index 2 : Char Value  - 0xFEA1
 *     index 3 : CCCD        - for 0xFEA1 Notify
 *     index 4 : Char Decl   - Pedometer Target
 *     index 5 : Char Value  - 0xFEA2
 *     index 6 : CCCD        - for 0xFEA2 Indicate
 *     index 7 : Char Decl   - Device MAC
 *     index 8 : Char Value  - 0xFEC9
 *---------------------------------------------------------------------------*/

/* ---- Simple BLE Protocol indices ----------------------------------------*/
#define GATT_UUID_AIRSYNC_CHAR_MEASUREMENT_INDEX            0x02
#define GATT_UUID_AIRSYNC_CHAR_MEASUREMENT_CCCD_INDEX       0x03
#define GATT_UUID_AIRSYNC_CHAR_TARGET_INDEX                 0x05
#define GATT_UUID_AIRSYNC_CHAR_TARGET_CCCD_INDEX            0x06
#define GATT_UUID_AIRSYNC_CHAR_MAC_INDEX                    0x08

/*-----------------------------------------------------------------------------
 * CCCD state encoding reported to upper layer via callback
 *---------------------------------------------------------------------------*/

#define GATT_UUID_AIRSYNC_CHAR_NOTIFY_ENABLE                0x01
#define GATT_UUID_AIRSYNC_CHAR_NOTIFY_DISABLE               0x02

#define GATT_UUID_AIRSYNC_CHAR_INDICATION_ENABLE            0x01
#define GATT_UUID_AIRSYNC_CHAR_INDICATION_DISABLE           0x02
///@endcond

/** @defgroup AIRSYNC_SVC_CB_MSG
  * @brief AirSync server callback messages
  * @{
  */
#define GATT_MSG_AIRSYNC_SERVER_READ                        0x00
#define GATT_MSG_AIRSYNC_SERVER_WRITE                       0x01
#define GATT_MSG_AIRSYNC_SERVER_CCCD_UPDATE                 0x02
/** @} End of AIRSYNC_SVC_CB_MSG */

/**
 * @brief AirSync server application callback prototype.
 *
 *        Used by the AirSync service to notify the upper application of
 *        read / write / CCCD update events.
 *
 *        @ref AIRSYNC_SVC_CB_MSG defines the @p type values.
 */
typedef uint16_t (*P_FUN_AIRSYNC_SERVER_APP_CB)(uint8_t type, void *p_data);

/** Write payload reported to the application. */
typedef struct
{
    uint8_t   opcode;
    uint16_t  length;
    uint8_t  *pValue;
} T_AIRSYNC_WRITE_MSG;

/** Union of message payloads carried by T_AIRSYNC_CALLBACK_DATA. */
typedef union
{
    uint8_t              notification_indification_index;  /* CCCD state, see above */
    T_AIRSYNC_WRITE_MSG  write;
} T_AIRSYNC_UPSTREAM_MSG_DATA;

/** AirSync service callback data passed to the application. */
typedef struct
{
    uint8_t                       conn_id;     /* Valid only on LE channels (0xFF on BR/EDR). */
    uint16_t                      chann_type;  /* T_GAP_CHANN_TYPE */
    T_AIRSYNC_UPSTREAM_MSG_DATA   msg_data;
} T_AIRSYNC_CALLBACK_DATA;

/** @} */

/** @} End of AIRSYNC_Exported_Types */

/** @defgroup AIRSYNC_Exported_Functions AirSync Exported Functions
  * @brief Functions that other .c files may use are all declared here.
  * @{
  */

/**
 * @brief    Register the AirSync (WeChat 0xFEE7) GATT service.
 * @param    app_cb  Callback invoked on read / write / CCCD update events.
 * @return   Service ID assigned by the GATT stack, or 0xFF on failure.
 */
T_SERVER_ID airsync_reg_srv(P_FUN_AIRSYNC_SERVER_APP_CB app_cb);

/**
 * @brief    Send a Notification on the Pedometer Measurement (0xFEA1)
 *           characteristic.
 *
 *           Caller MUST pass a valid (conn_handle, cid) pair, typically
 *           cached from a prior CCCD-enable / write callback. The function
 *           resolves the transport internally and drops the request if the
 *           link is no longer valid or the transport is disabled by the
 *           current build config (CONFIG_AIRSYNC_GATT_OVER_BREDR).
 *
 *           Both LE and BR/EDR ATT bearers are handled transparently - the
 *           caller does not need to distinguish between transports.
 *
 * @param    conn_handle  ACL handle of the target link.
 * @param    cid          ATT channel identifier of the target bearer.
 * @param    p_data       Notification payload.
 * @param    data_len     Length of @p p_data in bytes.
 */
void airsync_service_send_measurement_notification(uint16_t conn_handle, uint16_t cid,
                                                   uint8_t *p_data, uint16_t data_len);

/** @} End of AIRSYNC_Exported_Functions */

/** @} End of AIRSYNC */

/** @} End of RTK_Profile_Module */

#ifdef __cplusplus
}
#endif

#endif /* _AIRSYNC_BLE_SERVICE_H_ */
