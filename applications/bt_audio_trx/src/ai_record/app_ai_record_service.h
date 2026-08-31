/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _RECORD_TRANS_SERVICE_H_
#define _RECORD_TRANS_SERVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "profile_server_ext.h"
#include "app_msg.h"
#include "gap.h"
#include "gatt.h"
#include "bt_types.h"

/** @defgroup RECORD_TRANS_SERVICE Record Transfer Service
  * @brief LE / BR-EDR Service to upload Record_pen recording files to phone APK.
  *
  *        Phone to Record_pen (Write 0xBA26): variable-length control commands,
  *                  parsed by app_ai_record_file_trans module.
  *        Record_pen to Phone (Notify 0xBA27): command responses & file data
  *                  chunks, also constructed by app_ai_record_file_trans.
  *
  *        This service layer is transport-only: it does not parse payload
  *        format. Length, opcode, etc. are validated by the application.
  * @{
  */


/** @brief Service & Characteristic UUIDs (128-bit, advertised LSB-first). */
#define GATT_UUID128_RECORD_TRANS_SERVICE_ADV  0xF5, 0x17, 0x82, 0x91, 0x82, 0x67, 0x42, 0x6C, 0xBD, 0xA1, 0x70, 0x4A, 0xA9, 0x58, 0x4D, 0x49

#define GATT_UUID_CHAR_RECORD_TRANS_WRITE      0xBA26  /* phone to Record_pen (cmd) */
#define GATT_UUID_CHAR_RECORD_TRANS_NOTIFY     0xBA27  /* Record_pen to phone (data) */

/** @brief Attribute index in the service table (must stay in sync with
 *         gatt_extended_service_table[] in app_ai_record_service.c). */
#define BLE_SERVICE_CHAR_RECORD_TRANS_WRITE_INDEX          0x02
#define BLE_SERVICE_CHAR_RECORD_TRANS_NOTIFY_INDEX         0x04
#define BLE_SERVICE_CHAR_RECORD_TRANS_NOTIFY_CCCD_INDEX    0x05

/** @defgroup RECORD_TRANS_SVC_CB_MSG Callback Message Types
 *
 *  @brief All upstream events from the GATT layer are dispatched
 *         through a single application callback; the `type` argument
 *         tells the app which member of T_RECORD_TRANS_UPSTREAM_MSG_DATA
 *         is active. This indirection lets the integrator route writes
 *         from a single service into multiple business modules
 *         (e.g. dispatch by cmd_id) without touching service.c.
 *  @{ */
#define GATT_MSG_RECORD_TRANS_SERVER_WRITE                 0x00
#define GATT_MSG_RECORD_TRANS_SERVER_CCCD_UPDATE           0x01
/** @} */

/** @brief  Variable-length write payload descriptor.
 *
 *  The Record-Trans control point accepts variable-length commands
 *  (filename + offset + max_chunk for CMD_UPLOAD_FILE, etc.), so unlike
 *  the legacy 2-byte {opcode, value} idiom this struct carries a
 *  pointer + length rather than the bytes by value.
 *
 *  Memory ownership:
 *    - p_value points into the GATT stack's internal RX buffer.
 *    - The pointer is valid ONLY within the callback context.
 *    - Application MUST copy the bytes out if it needs to defer
 *      processing (e.g. post to a task queue).
 *
 *  Wire layout starts with the 2-byte LE cmd_id, followed by the body.
 */
typedef struct
{
    uint8_t  *p_value;     /**< raw payload: [cmd_id(2B LE)][body...] */
    uint16_t  length;      /**< total length, >= 2. */
} T_RECORD_TRANS_WRITE_MSG;

/** @brief  Discriminated union of upstream events.
 *
 *  Active member is determined by the `type` parameter passed to the
 *  application callback (see GATT_MSG_RECORD_TRANS_*).
 */
typedef union
{
    uint8_t                  notification_index;  /**< CCCD update: affected notify char index */
    T_RECORD_TRANS_WRITE_MSG write;                /**< Write event: raw payload pointer + length */
} T_RECORD_TRANS_UPSTREAM_MSG_DATA;

/** @brief  Service callback data passed to the application callback.
 *
 *  Application is informed of the link (conn_id / conn_handle / cid /
 *  chann_type) plus the event-specific msg_data. The (conn_handle, cid)
 *  pair is what the application later passes back to
 *  record_trans_service_send_notification() when pushing data.
 */
typedef struct
{
    uint8_t                          conn_id;       /**< LE conn_id; 0xFF for BR/EDR. */
    uint16_t                         conn_handle;   /**< ACL handle. */
    uint16_t                         cid;           /**< ATT channel id. */
    uint16_t                         chann_type;    /**< T_GAP_CHANN_TYPE. */
    T_RECORD_TRANS_UPSTREAM_MSG_DATA msg_data;
} T_RECORD_TRANS_CALLBACK_DATA;

/** @brief Application callback prototype.
 *
 *  Return value is meaningful only for WRITE events - it becomes the
 *  ATT response status. CCCD events ignore the return.
 */
typedef T_APP_RESULT(*P_FUN_RECORD_TRANS_SERVER_APP_CB)(uint8_t type, void *p_data);

/** @brief Public service id (0xFF until registered). */
extern T_SERVER_ID rt_srv_id_local;

/**
 * @brief  Register record-trans service to GATT layer.
 *
 *         Caller decides whether to wire a CCCD callback. Pass NULL if
 *         the application does not care about notify-enable transitions
 *         (the file-trans module does want to know).
 *
 * @param  app_cb  Application callback for CCCD events.
 * @return Service ID assigned by the profile layer (0xFF on failure).
 */
T_SERVER_ID record_trans_reg_srv(P_FUN_RECORD_TRANS_SERVER_APP_CB app_cb);

/**
 * @brief  Send notification on Record-Trans Notify characteristic.
 *
 *         Caller MUST provide a valid (conn_handle, cid) pair obtained
 *         from a prior CCCD-enable event or write callback. This routine
 *         does NOT split oversize payload; chunking is the caller's job.
 *
 * @return true  if the notification was accepted by the GATT server
 *               (will be transmitted asynchronously).
 *         false if the GATT server's TX queue is full - caller should
 *               NOT advance its send state and should retry later.
 */
bool record_trans_service_send_notification(uint16_t conn_handle, uint16_t cid,
                                            uint8_t *p_data, uint16_t data_len);

#if CONFIG_RECORD_TRANS_GATT_OVER_BREDR
/**
 * @brief  Submit the record-trans SDP record to the SDP server.
 *
 *         Must be called AFTER record_trans_reg_srv() so that the ATT
 *         handle range advertised in the SDP record matches the actual
 *         allocation. The record currently uses hard-coded handle range;
 *         see app_ai_record_service.c if registration order changes.
 */
void record_trans_sdp_register(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _RECORD_TRANS_SERVICE_H_ */
