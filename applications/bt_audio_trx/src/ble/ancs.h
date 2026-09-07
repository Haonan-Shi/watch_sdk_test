/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _ANCS_SAMPLE_H__
#define _ANCS_SAMPLE_H__

#ifdef __cplusplus
extern "C" {
#endif
/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <gap_le.h>

/** @defgroup PERIPH_ANCS Peripheral ANCS
  * @brief Peripheral ANCS
  * @{
  */

/*============================================================================*
 *                              Macros
 *============================================================================*/
/** @defgroup PERIPH_ANCS_Exported_Macros ANCS Exported Macros
   * @{
   */
#define ANCS_MAX_ATTR_LEN 256 //!< Max ANCS attribute length

#define F_BT_ANCS_GET_APP_ATTR  1

#define F_BT_ANCS_APP_FILTER    1

#if F_BT_ANCS_GET_APP_ATTR
#define ANCS_APP_IDENTIFIER_MAX_LEN 30  //!< Max app identifier length
#endif


/** End of PERIPH_ANCS_Exported_Macros
    * @}
    */

/*============================================================================*
 *                         Types
 *============================================================================*/
/** @defgroup PERIPH_ANCS_Exported_Types ANCS Exported Types
    * @{
    */

/**  @brief  ANCS category id */
typedef enum
{
    ANCS_CATEGORY_ID_OTHER = 0,
    ANCS_CATEGORY_ID_INCOMING_CALL = 1,
    ANCS_CATEGORY_ID_MISSED_CALL = 2,
    ANCS_CATEGORY_ID_VOICE_MAIL = 3,
    ANCS_CATEGORY_ID_SOCIAL = 4,
    ANCS_CATEGORY_ID_SCHEDULE = 5,
    ANCS_CATEGORY_ID_EMAIL = 6,
    ANCS_CATEGORY_ID_NEWS = 7,
    ANCS_CATEGORY_ID_HEALTH_AND_FITNESS = 8,
    ANCS_CATEGORY_ID_BUSINESS_ADN_FINANCE = 9,
    ANCS_CATEGORY_ID_LOCATION = 10,
    ANCS_CATEGORY_ID_ENTERTAINMENT = 11,
    ANCS_CATEGORY_ID_ACTIVE_CALL = 12,
    ANCS_CATEGORY_ID_RESERVED = 255
} T_ANCS_CATEGORY_ID;

/**  @brief  App parse ANCS notification attribute state */
typedef enum
{
    DS_PARSE_NOT_START = 0x00,
    DS_PARSE_GET_NOTIFICATION_COMMAND_ID = 0x01,
    DS_PARSE_UID1,
    DS_PARSE_UID2,
    DS_PARSE_UID3,
    DS_PARSE_UID4,
    DS_PARSE_ATTRIBUTE_ID,
    DS_PARSE_ATTRIBUTE_LEN1,
    DS_PARSE_ATTRIBUTE_LEN2,
    DS_PARSE_ATTRIBUTE_READY
} T_DS_NOTIFICATION_ATTR_PARSE_STATE;

/**  @brief  App parse ANCS attribute state */
typedef enum
{
    DS_PARSE_GET_APP_COMMAND_ID = 0x10,
    DS_PARSE_APP_IDENTIFIER_START,
    DS_PARSE_APP_IDENTIFIER_END,
    DS_PARSE_APP_ATTRIBUTE_ID,
    DS_PARSE_APP_ATTRIBUTE_LEN1,
    DS_PARSE_APP_ATTRIBUTE_LEN2,
    DS_PARSE_APP_ATTRIBUTE_READY
} T_DS_APP_ATTR_PARSE_STATE;

/**  @brief  App parse ANCS notification attribute type */
typedef enum
{
    ANCS_NOTIFICATION_TYPE_TENCENT_WECHAT = 0x01,
    ANCS_NOTIFICATION_TYPE_MOBILE_SMS,
    ANCS_NOTIFICATION_TYPE_MOBILE_PHONE,
    ANCS_NOTIFICATION_TYPE_TENCENT_QQ,
} T_ANCS_NOTIFICATION_TYPE;

/**  @brief  Define notification attribute details data */
/**          App can acquire details information by attribute id */
typedef struct
{
    uint8_t    command_id;
    uint8_t    notification_uid[4];
    uint8_t    attribute_id;
    uint16_t   attribute_len;
    uint8_t    data[ANCS_MAX_ATTR_LEN];
} T_DS_NOTIFICATION_ATTR;

/**  @brief  Local app record notification attribute information */
typedef struct
{
    uint8_t    command_id;
    uint8_t    attribute_id;
    uint16_t   attribute_len;
    uint8_t    data[ANCS_MAX_ATTR_LEN];
} T_DS_APP_ATTR;

/** End of PERIPH_ANCS_Exported_Types
    * @}
    */

/*============================================================================*
 *                         Functions
 *============================================================================*/
/** @defgroup PERIPH_ANCS_Exported_Functions ANCS Exported Functions
    * @{
    */
/**
 * @brief  Perform a positive action to accept incoming call.
 *
 * @return Operation result.
 * @retval true Operation success.
 * @retval false Operation failure.
 */
bool ancs_accept_incoming_call(void);

/**
 * @brief  Perform a negative action to reject incoming call.
 *
 * @return Operation result.
 * @retval true Operation success.
 * @retval false Operation failure.
 */
bool ancs_reject_incoming_call(void);

/**
 * @brief  Perform a negative action to end active call.
 *
 * @return Operation result.
 * @retval true Operation success.
 * @retval false Operation failure.
 */
bool ancs_end_active_call(void);

/**
 * @brief  Get the current status of the ANCS link.
 *
 * @return ANCS link status.
 * @retval true The ANCS link is active.
 * @retval false The ANCS link is not active.
 */
bool ancs_get_link_status(void);

/**
 * @brief  App register ANCS client to bluetooth host.
 *
 * @return void
 */
void app_ancs_client_init(void);

/** @} */ /* End of group PERIPH_ANCS_Exported_Functions */
/** @} */ /* End of group PERIPH_ANCS */
#ifdef __cplusplus
}
#endif

#endif
