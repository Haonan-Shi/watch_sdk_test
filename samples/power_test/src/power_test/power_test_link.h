/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _POWER_TEST_LINK_H_
#define _POWER_TEST_LINK_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @defgroup POWER_TEST_LINK Power Test APP Link
  * @brief Power Test APP Link
  * @{
  */

/*============================================================================*
 *                         Macros
 *============================================================================*/
/** @addtogroup POWER_TEST_LINK_Exported_Macros Power Test Link Macros
  * @brief
  * @{
  */

/** @brief  Define links number. range: 0-4 */
#define POWER_TEST_MAX_BR_LINK_NUM                 4

/** @brief  Define bitmask of profiles */
#define A2DP_PROFILE_MASK               0x00000001    /**< A2DP profile bitmask */
#define AVRCP_PROFILE_MASK              0x00000002    /**< AVRCP profile bitmask */
#define HFP_PROFILE_MASK                0x00000004    /**< HFP profile bitmask */
#define RDTP_PROFILE_MASK               0x00000008    /**< Remote Control vendor profile bitmask */
#define SPP_PROFILE_MASK                0x00000010    /**< SPP profile bitmask */
#define IAP_PROFILE_MASK                0x00000020    /**< iAP profile bitmask */
#define PBAP_PROFILE_MASK               0x00000040    /**< PBAP profile bitmask */
#define HSP_PROFILE_MASK                0x00000080    /**< HSP profile bitmask */
#define HID_PROFILE_MASK                0x00000100    /**< HID profile bitmask */
#define MAP_PROFILE_MASK                0x00000200    /**< MAP profile bitmask */
#define GATT_PROFILE_MASK               0x00008000    /**< GATT profile bitmask */
#define GFPS_PROFILE_MASK               0x00010000    /**< GFPS profile bitmask */
#define XIAOAI_PROFILE_MASK             0x00020000    /**< XIAOAI profile bitmask */
#define AMA_PROFILE_MASK                0x00040000    /**< AMA profile bitmask */
#define AVP_PROFILE_MASK                0x00080000    /**< AVP profile bitmask */
#define DID_PROFILE_MASK                0x80000000    /**< DID profile bitmask */

#define ALL_PROFILE_MASK                0xffffffff

/** End of POWER_TEST_LINK_Exported_Macros
* @}
*/

/*============================================================================*
 *                         Types
 *============================================================================*/
/** @defgroup POWER_TEST_LINK_Exported_Types Power Test Link Types
  * @brief
  * @{
  */

/**  @brief  APP's Bluetooth BR/EDR link connection database */
typedef struct
{
    uint8_t             bd_addr[6];
    bool                used;
    uint8_t             id;

    uint32_t            connected_profile;
    uint8_t             sdp_hfp_ag_hf_record_num;
    uint8_t             sdp_hfp_ag_hs_record_num;
    uint32_t            sdp_active_inquire_profile;
} T_POWER_TEST_LINK;

/**  @brief  Define global app data structure */
typedef struct
{
    T_POWER_TEST_LINK               app_link[POWER_TEST_MAX_BR_LINK_NUM];
} T_POWER_TEST_APP_DB;
/** End of POWER_TEST_LINK_Exported_Types
* @}
*/

/*============================================================================*
 *                         Functions
 *============================================================================*/
/** @defgroup POWER_TEST_LINK_Exported_Functions Power Test Link Exported Functions
  * @brief
  * @{
  */

/**
 * @brief     Find the APP data instance with Remote BT address.
 * @param[in] bd_addr Remote BT address.
 * @return    APP data instance. If returned handle is NULL, the APP
 *            data instance was failed to create.
 */
T_POWER_TEST_LINK *power_test_find_link(uint8_t *bd_addr);

/**
 * @brief     Allocate the APP data instance with Remote BT address.
 * @param[in] bd_addr Remote BT address.
 * @return    APP data instance. If returned handle is NULL, the APP
 *            data instance was failed to create.
 */
T_POWER_TEST_LINK *power_test_alloc_link(uint8_t *bd_addr);

/**
 * @brief     Free the APP data instance with Remote BT address.
 * @param[in] bd_addr Remote BT address.
 * @return    APP data instance. If returned handle is NULL, the APP
 *            data instance was failed to create.
 */
bool power_test_free_link(T_POWER_TEST_LINK *p_link);

/** @} End of POWER_TEST_LINK_Exported_Functions */

/** @} End of POWER_TEST_LINK */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _POWER_TEST_LINK_H_ */
