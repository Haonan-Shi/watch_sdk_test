/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Define to prevent recursive inclusion */
#ifndef _HRS_SVC_DEF_H
#define _HRS_SVC_DEF_H

#ifdef __cplusplus
extern "C"  {
#endif      /* __cplusplus */

#include "bt_gatt_svc.h"

/** @defgroup HRS Heart Rate Service
  * @brief  Heart Rate Service
  * @details

    The Heart Rate Service exposes heart rate and other data related to a heart rate sensor intended for fitness applications.

    Applications shall register heart rate service during initialization through @ref hrs_add_service function.

    The Heart Rate Measurement characteristic is used to send a heart rate measurement. Included in the characteristic are a Flags field (for showing the presence of optional fields and features
    supported), a heart rate measurement value field and, depending upon the contents of the Flags field, an Energy Expended field and an RR-Interval field. The RR-Interval represents the time between
    two consecutive R waves in an Electrocardiogram (ECG) waveform.
    Applications can send heart rate measurement value through @ref hrs_heart_rate_measurement_value_notify function.

    The Body Sensor Location characteristic of the device is used to describe the intended location of the heart rate measurement for the device.
    The value of the Body Sensor Location characteristic is static while in a connection.

    The Heart Rate Control Point characteristic is used to enable a Client to write control points to a Server to control behavior.
    Support for this characteristic is mandatory if the Server supports the Energy Expended feature.
  * @{
  */

/*============================================================================*
 *                         Macros
 *============================================================================*/
/** @defgroup HRS_Exported_Macros HRS Exported Macros
  * @brief
  * @{
  */

#define GATT_MSG_HRS_SERVER_WRITE_CP_OPCODE      0x01
#define GATT_MSG_HRS_SERVER_CCCD_UPDATE          0x02
#define GATT_MSG_HRS_SERVER_READ_BODY_SENSOR_LOCATION_VALUE_IND 0x03

/** @defgroup HRS_Read_Info HRS Read Info
  * @brief  Parameter for reading characteristic value.
  * @{
  */
#define HRS_READ_BODY_SENSOR_LOCATION_VALUE                 1
/**< Read Body Sensor Location characteristic value index. */
/** @} */

/** @defgroup HRS_Notify_Indicate_Info HRS Notify Indicate Info
  * @brief  Parameter for enabling or disabling notification or indication.
  * @{
  */
#define HRS_NOTIFY_INDICATE_MEASUREMENT_VALUE_ENABLE        1
/**< Heart Rate Measurement characteristic notification enabled index. */
#define HRS_NOTIFY_INDICATE_MEASUREMENT_VALUE_DISABLE       2
/**< Heart Rate Measurement characteristic notification disabled index. */
/** @} */

/** @defgroup HRS_Sensor_Location HRS Sensor Location
  * @brief  Body Sensor Location Value
  * @{
  */
#define  BODY_SENSOR_LOCATION_VALUE_OTHER                   0 //!< Other.
#define  BODY_SENSOR_LOCATION_VALUE_CHEST                   1 //!< Chest.
#define  BODY_SENSOR_LOCATION_VALUE_WRIST                   2 //!< Wrist.
#define  BODY_SENSOR_LOCATION_VALUE_FINGER                  3 //!< Finger.
#define  BODY_SENSOR_LOCATION_VALUE_HAND                    4 //!< Hand.
#define  BODY_SENSOR_LOCATION_VALUE_EAR_LOBE                5 //!< Ear Lobe.
#define  BODY_SENSOR_LOCATION_VALUE_FOOT                    6 //!< Foot.
/** @} */

/** @defgroup HRS_DEFs HRS Definitions
  * @brief  HRS definitions.
  * @{
  */
#define HRS_MAX_CP_VALUE_LEN                                1
/**< Maximum length of Heart Rate Control Point characteristic value. */
/** @} */

/** End of HRS_Exported_Macros
* @}
*/

/*============================================================================*
 *                         Types
 *============================================================================*/
/** @defgroup HRS_Exported_Types HRS Exported Types
  * @brief
  * @{
  */
typedef T_APP_RESULT(*P_FUN_HRS_SERVER_APP_CB)(uint16_t conn_handle, uint16_t cid,
                                               uint8_t type, void *p_data);
typedef enum
{
    HRS_HEART_RATE_CP_OPCODE_RESERVED = 0, /**< Reserved for Future Use. */
    /**< Reset Energy Expended. Resets the value of the Energy Expended field in the
      *  Heart Rate Measurement characteristic to 0 */
    HRS_HEART_RATE_CP_OPCODE_RESET_ENERGY_EXPENDED = 1,
} T_HRS_HEART_RATE_CP_OPCODE;

/** Notification indication flag. */
typedef struct
{
    uint8_t heart_rate_measurement_notify_enable: 1;
    uint8_t rfu: 7;
} HRS_NOTIFY_INDICATE_FLAG;

/** Heart Rate Measurement Value Flag. */
typedef struct
{
    uint8_t heart_rate_value_format_bit: 1;
    uint8_t sensor_contact_status_bits: 2;
    uint8_t energy_expended_status_bit: 1;
    uint8_t rr_interval_bit: 1;
    uint8_t rfu: 3;
} T_HEART_RATE_MEASUREMENT_VALUE_FLAG;

/** Heart Rate Measurement Value. */
typedef struct
{
    T_HEART_RATE_MEASUREMENT_VALUE_FLAG flag;
    uint16_t heart_rate_measurement_value;
    uint16_t energy_expended;
    uint16_t *p_rr_interval;
    uint16_t rr_interval_len;
} T_HEART_RATE_MEASUREMENT_VALUE;

/**
 * @brief HRS Control Point data, variable length during connection, maximum can reach 17 octets.
 *
 * HRS Control Point data used to store the Control Point Command received from the client.
*/
typedef struct
{
    uint8_t    cur_length; /**<  Length of current CSC Control Point data. */
    uint8_t    value[HRS_MAX_CP_VALUE_LEN]; /**<  Value of current CSC Control Point data. */
} T_HRS_CP_DATA;

typedef struct
{
    T_HRS_HEART_RATE_CP_OPCODE opcode;
} T_HRS_SERVER_WRITE_CP_OPCODE;

typedef struct
{
    uint8_t notification_enable_index;
} T_HRS_SERVER_CCCD_UPDATE;

typedef struct
{
    uint8_t read_value_index;
} T_HRS_SERVER_READ_BODY_SENSOR_LOCATION_VALUE;

/** End of HRS_Exported_Types
* @}
*/

/*============================================================================*
 *                         Functions
 *============================================================================*/
/** @defgroup HRS_Exported_Functions HRS Exported Functions
  * @brief
  * @{
  */

/**
 * @brief       Add hrs service to the BLE stack database.
 *
 * @param[in] app_cb   Callback when service attribute was read, write or cccd update.
 * @return Service id  generated by the BLE stack: @ref T_SERVER_ID.
 * @retval 0xFF        Operation failure.
 * @retval Others      Service id assigned by stack.
 *
 * <b>Example usage</b>
 * \code{.c}
    T_APP_RESULT app_hrs_service_callback(uint16_t conn_handle, uint16_t cid,
                                          uint8_t type, void *p_data)
    {
      APP_PRINT_INFO3("app_hrs_service_callback: conn_handle 0x%x, cid 0x%x, type 0x%x",
                      conn_handle, cid, type);

        switch (type)
        {
        case GATT_MSG_HRS_SERVER_WRITE_CP_OPCODE:
        {
          T_HRS_SERVER_WRITE_CP_OPCODE *p_hrs_cp;
          p_hrs_cp = (T_HRS_SERVER_WRITE_CP_OPCODE *)p_data;

          APP_PRINT_INFO1("app_hrs_service_callback: opcode 0x%x", p_hrs_cp->opcode);

        }
        break;

        case GATT_MSG_HRS_SERVER_CCCD_UPDATE:
        {
          T_HRS_SERVER_CCCD_UPDATE *p_hrs_cccd_update;
          p_hrs_cccd_update = (T_HRS_SERVER_CCCD_UPDATE *)p_data;
          APP_PRINT_INFO1("app_hrs_service_callback: notification enable index 0x%x", p_hrs_cccd_update->notification_enable_index);

        }
        break;

        case GATT_MSG_HRS_SERVER_READ_BODY_SENSOR_LOCATION_VALUE_IND:
        {
            T_HRS_SERVER_READ_BODY_SENSOR_LOCATION_VALUE *p_hrs_read_body_sensor_location;
            p_hrs_read_body_sensor_location = (T_HRS_SERVER_READ_BODY_SENSOR_LOCATION_VALUE *)p_data;

            if (p_hrs_read_body_sensor_location->read_value_index == HRS_READ_BODY_SENSOR_LOCATION_VALUE)
            {
                return APP_RESULT_PENDING;
            }
        }
          break;
        ......

        return APP_RESULT_SUCCESS;
    }

    void profile_init(uint16_t mode, uint8_t svc_num)
    {
         gatt_svc_init(mode, svc_num);
         hrs_id = hrs_reg_srv(app_hrs_service_callback);
    }
 * \endcode
 */
T_SERVER_ID hrs_reg_srv(P_FUN_HRS_SERVER_APP_CB app_cb);

/**
 * @brief Send Body Sensor Location read confirmation.
 *
 * @note APP can call this function when receive the body sensor location read callback.
 *       If application accept the read request, the cause of hrs read callback shall be
 *       set to @ref APP_RESULT_PENDING. And then call this API to send the read confirmation
 *       with cause @ref APP_RESULT_SUCCESS.
 *
 * \xrefitem Experimental_Added_API_2_14_0_0 "Experimental Added Since 2.14.0.0" "Experimental Added API"
 *
 * @param[in]   conn_handle    Connection handle of the ACL link.
 * @param[in]   cid            Local CID assigned by Bluetooth stack.
 * @param[in]   service_id               Service id.
 * @param[in]   hrs_body_sensor_location HRS body sensor location characteristic value.
 * @param[in]   cause                    Cause: @ref T_APP_RESULT.
 *
 * @return Operation result.
 * @retval true Operation success.
 * @retval false Operation failure.
 *
 * <b>Example usage</b>
 * \code{.c}
    T_APP_RESULT app_hrs_service_callback(uint16_t conn_handle, uint16_t cid,
                                          uint8_t type, void *p_data)
    {
        switch (type)
        {
        case GATT_MSG_HRS_SERVER_READ_BODY_SENSOR_LOCATION_VALUE_IND:
        {
            T_HRS_SERVER_READ_BODY_SENSOR_LOCATION_VALUE *p_hrs_read_body_sensor_location;
            p_hrs_read_body_sensor_location = (T_HRS_SERVER_READ_BODY_SENSOR_LOCATION_VALUE *)p_data;

            if (p_hrs_read_body_sensor_location->read_value_index == HRS_READ_BODY_SENSOR_LOCATION_VALUE)
            {
                return APP_RESULT_PENDING;
            }
        }
          break;
        ......

        return APP_RESULT_SUCCESS;
    }

    void test(void)
    {
        bool result = hrs_body_sensor_location_read_confirm(conn_handle, cid, service_id,
                                                            hrs_body_sensor_location,
                                                            cause);
    }
 * \endcode
 */
bool hrs_body_sensor_location_read_confirm(uint16_t conn_handle, uint16_t cid,
                                           T_SERVER_ID service_id, uint8_t hrs_body_sensor_location);

/**
 * @brief       Send Heart Rate Measurement characteristic notification data.
 *
 * @param[in]   conn_handle    Connection handle of the ACL link.
 * @param[in]   cid            Local CID assigned by Bluetooth stack.
 * @param[in]   service_id     Service id.
 * @param[in]   heart_rate_measurement_value  Heart rate measurement notify value: @ref T_HEART_RATE_MEASUREMENT_VALUE.
 *
 * @return Operation result.
 * @retval true Operation success.
 * @retval false Operation failure.
 *
 * <b>Example usage</b>
 * \code{.c}
    void test(void)
    {
        bool result = hrs_heart_rate_measurement_value_notify(conn_id, service_id, heart_rate_measurement_value);
    }
 * \endcode
 */
bool hrs_heart_rate_measurement_value_notify(uint16_t conn_handle, uint16_t cid,
                                             T_SERVER_ID service_id,
                                             T_HEART_RATE_MEASUREMENT_VALUE heart_rate_measurement_value);

/** @} End of HRS_Exported_Functions */

/** @} End of HRS */


#ifdef __cplusplus
}
#endif

#endif /* _HRS_SVC_DEF_H */
