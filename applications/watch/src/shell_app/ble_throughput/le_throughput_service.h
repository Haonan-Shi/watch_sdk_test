/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _LE_THROUGHPUT_SERVICE_H_
#define _LE_THROUGHPUT_SERVICE_H_

#include <stdint.h>
#include <stdbool.h>
#include "bt_gatt_svc.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup LE_THROUGHPUT_SERVICE LE Throughput Test Service
  * @brief LE Throughput Test Service for measuring BLE throughput performance.
  *
  * This service implements the LE Throughput Test Profile specification v1.0.
  * It provides control (CT_TX/CT_RX) and data (DT_TX/DT_RX) channels for
  * throughput testing between BTSOC and Phone.
  *
  * Service UUID: 0xA00D
  * @{
  */

/*============================================================================*
 *                              Macros
 *============================================================================*/

/** @defgroup LE_THROUGHPUT_SERVICE_UUID LE Throughput Service UUIDs
  * @{
  */
#define BLE_UUID_LE_THROUGHPUT_SERVICE      0xA00D  /**< Primary Service UUID */
#define BLE_UUID_CT_TX_CHAR                 0xB001  /**< Control TX (Indication) UUID */
#define BLE_UUID_CT_RX_CHAR                 0xB002  /**< Control RX (Write Request) UUID */
#define BLE_UUID_DT_TX_CHAR                 0xB003  /**< Data TX (Notification) UUID */
#define BLE_UUID_DT_RX_CHAR                 0xB004  /**< Data RX (Write Command) UUID */
/** @} */

/** @defgroup LE_THROUGHPUT_ATTRIB_INDEX Attribute Table Indices
  * @{
  */
#define LE_THROUGHPUT_CT_TX_VALUE_INDEX     2   /**< CT_TX characteristic value index */
#define LE_THROUGHPUT_CT_TX_CCCD_INDEX      3   /**< CT_TX CCCD index */
#define LE_THROUGHPUT_CT_RX_VALUE_INDEX     5   /**< CT_RX characteristic value index */
#define LE_THROUGHPUT_DT_TX_VALUE_INDEX     7   /**< DT_TX characteristic value index */
#define LE_THROUGHPUT_DT_TX_CCCD_INDEX      8   /**< DT_TX CCCD index */
#define LE_THROUGHPUT_DT_RX_VALUE_INDEX     10  /**< DT_RX characteristic value index */
/** @} */

/** @defgroup LE_THROUGHPUT_OPCODE Signal Protocol OpCodes
  * @{
  */
#define LE_OPCODE_GET_MTU                   0x01  /**< Query BTSOC ATT MTU */
#define LE_OPCODE_GET_MTU_RSP               0x02  /**< Response with ATT MTU value */
#define LE_OPCODE_CONFIG_TX_PARAMS          0x11  /**< Configure TX test parameters */
#define LE_OPCODE_CONFIG_RX_PARAMS          0x12  /**< Configure RX test parameters */
#define LE_OPCODE_CONFIG_RXTX_PARAMS        0x13  /**< Configure RXTX test parameters */
#define LE_OPCODE_CONFIGPARAMS_RSP          0x21  /**< Configuration result response */
#define LE_OPCODE_TEST_COMPLETE             0x22  /**< Test complete notification */
#define LE_OPCODE_GET_TEST_REPORT           0x31  /**< Request test report */
#define LE_OPCODE_GET_TEST_REPORT_RSP       0x32  /**< Test report response */
/** @} */

/** @defgroup LE_THROUGHPUT_FLAG Throughput Test Flag Bits
  * @{
  */
#define THROUGHPUT_FLAG_DLE                 0x01  /**< Bit0: Data Length Extension */
#define THROUGHPUT_FLAG_2M_PHY              0x02  /**< Bit1: LE 2M PHY */
/** @} */

/** @defgroup LE_THROUGHPUT_STATUS Configuration Status
  * @{
  */
#define THROUGHPUT_STATUS_SUCCESS           0x00  /**< Config success */
#define THROUGHPUT_STATUS_MISMATCH          0x01  /**< Updated value mismatch */
#define THROUGHPUT_STATUS_TIMEOUT           0x02  /**< Config update timeout */
/** @} */

/** @defgroup LE_THROUGHPUT_CB_MSG Callback Message Types
  * @{
  */
#define GATT_MSG_LE_THROUGHPUT_WRITE        0x00  /**< Write received on control or data channel */
#define GATT_MSG_LE_THROUGHPUT_CT_TX_CCCD   0x01  /**< CT_TX CCCD updated */
#define GATT_MSG_LE_THROUGHPUT_DT_TX_CCCD   0x02  /**< DT_TX CCCD updated */
/** @} */

/*============================================================================*
 *                              Types
 *============================================================================*/

/** @brief LE_CONFIG_TX/RX/RXTX_PARAMS test start request structure (packed) */
typedef struct
{
    uint8_t  opcode;        /* 0x11 / 0x12 / 0x13 */
    uint16_t packet_len;    /* ATT packet length */
    uint16_t duration;      /* Test duration (seconds) */
    uint16_t min_interval;  /* Min Connection Interval (*1.25ms) */
    uint16_t max_interval;  /* Max Connection Interval (*1.25ms) */
    uint8_t  flag;          /* Bit0: DLE, Bit1: 2M PHY */
} __attribute__((packed)) T_TEST_START_REQUEST;

/** @brief LE_CONFIGPARAMS_RSP structure (packed) */
typedef struct
{
    uint8_t  opcode;        /* 0x21 */
    uint8_t  status;        /* 0=success, 1=mismatch, 2=timeout */
    uint16_t interval;      /* Actual Connection Interval (*1.25ms) */
    uint8_t  flag;          /* DLE/2M negotiation result */
} __attribute__((packed)) T_CONFIG_PARAMS_RESPONSE;

/** @brief LE_Get_MTU_Response structure (packed) */
typedef struct
{
    uint8_t  opcode;        /* 0x02 */
    uint16_t mtu;           /* ATT MTU value */
} __attribute__((packed)) T_GET_MTU_RESPONSE;

/** @brief LE_Get_Test_Report_Response structure (packed) */
typedef struct
{
    uint8_t  opcode;        /* 0x32 */
    uint32_t tx_rate;       /* TX throughput (Bps) */
    uint32_t rx_rate;       /* RX throughput (Bps) */
} __attribute__((packed)) T_TEST_REPORT_RESPONSE;

/** @brief Test mode state */
typedef enum
{
    THROUGHPUT_MODE_IDLE        = 0x00,  /**< Not in test mode */
    THROUGHPUT_MODE_TX_TEST     = 0x01,  /**< BTSOC -> Phone TX test */
    THROUGHPUT_MODE_RX_TEST     = 0x02,  /**< Phone -> BTSOC RX test */
    THROUGHPUT_MODE_RXTX_TEST   = 0x03,  /**< Bidirectional test */
} T_THROUGHPUT_TEST_MODE;

/** @brief Write data from CT_RX or DT_RX */
typedef struct
{
    uint16_t len;
    uint8_t *p_value;
    uint8_t  attr_index;    /**< Attribute index that was written */
} T_LE_THROUGHPUT_RX_DATA;

/** @brief Callback message data union */
typedef union
{
    uint8_t notification_indification_value;  /**< CCCD enable/disable value */
    T_LE_THROUGHPUT_RX_DATA rx_data;          /**< Received data */
} T_LE_THROUGHPUT_MSG_DATA;

/** @brief Callback data structure for application notification */
typedef struct
{
    uint8_t                  conn_id;
    T_SERVER_ID              service_id;
    uint16_t                 conn_handle;
    uint16_t                 cid;
    uint8_t                  attr_index;
    T_LE_THROUGHPUT_MSG_DATA msg_data;
} T_LE_THROUGHPUT_CALLBACK_DATA;

/** @brief Application callback function pointer */
typedef uint16_t (*P_FUN_LE_THROUGHPUT_APP_CB)(uint8_t type, void *p_data);

/*============================================================================*
 *                              Functions
 *============================================================================*/

/**
 * @brief Register the LE Throughput Test Service with the BLE stack.
 *
 * @param[in]  app_cb   Application callback for service events.
 * @param[in]  send_cb  Callback for send data complete notifications.
 * @return Service ID assigned by stack, 0xFF on failure.
 */
uint8_t le_throughput_reg_srv(P_FUN_LE_THROUGHPUT_APP_CB app_cb,
                              P_FUN_GATT_EXT_SEND_DATA_CB send_cb);

/**
 * @brief Send data via CT_TX (Indication) - control channel.
 *
 * @param[in] conn_handle  Connection handle.
 * @param[in] cid          L2CAP channel ID.
 * @param[in] p_value      Data to send.
 * @param[in] len          Data length.
 * @return true if send request was accepted, false otherwise.
 */
bool le_throughput_send_ct_tx_data(uint16_t conn_handle, uint16_t cid,
                                   uint8_t *p_value, uint16_t len);

/**
 * @brief Send data via DT_TX (Notification) - data channel.
 *
 * @param[in] conn_handle  Connection handle.
 * @param[in] cid          L2CAP channel ID.
 * @param[in] p_value      Data to send.
 * @param[in] len          Data length.
 * @return true if send request was accepted, false otherwise.
 */
bool le_throughput_send_dt_tx_data(uint16_t conn_handle, uint16_t cid,
                                   uint8_t *p_value, uint16_t len);

/**
 * @brief Check if DT_TX CCCD is enabled for a connection.
 *
 * @param[in] conn_handle  Connection handle.
 * @param[in] cid          L2CAP channel ID.
 * @return true if DT_TX notification is enabled.
 */
bool le_throughput_is_dt_tx_enabled(uint16_t conn_handle, uint16_t cid);

/**
 * @brief Check if CT_TX CCCD is enabled for a connection.
 *
 * @param[in] conn_handle  Connection handle.
 * @param[in] cid          L2CAP channel ID.
 * @return true if CT_TX indication is enabled.
 */
bool le_throughput_is_ct_tx_enabled(uint16_t conn_handle, uint16_t cid);

/** @} */ /* End of group LE_THROUGHPUT_SERVICE */

#ifdef __cplusplus
}
#endif

#endif /* _LE_THROUGHPUT_SERVICE_H_ */
