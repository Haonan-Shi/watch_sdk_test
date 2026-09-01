/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _POWER_TEST_SDP_H_
#define _POWER_TEST_SDP_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @defgroup POWER_TEST_SDP POWER_TEST SDP
* @brief POWER_TEST SDP
* @{
*/

/*============================================================================*
 *                         Macros
 *============================================================================*/
/** @addtogroup POWER_TEST_SDP_Exported_Macros POWER_TEST SDP Exported Macros
  * @brief
  * @{
  */

/**
* @brief Define POWER TEST local server channel
* @note  POWER TEST service num may be different in different POWER TEST applications
*
*/
#define RFC_HFP_CHANN_NUM               1
#define RFC_HSP_CHANN_NUM               2
#define RFC_HSP_AG_CHANN_NUM            21
#define RFC_HFP_AG_CHANN_NUM            22

/** End of POWER_TEST_SDP_Exported_Macros
* @}
*/

/*============================================================================*
 *                         Functions
 *============================================================================*/
/** @defgroup POWER_TEST_SDP_Exported_Functions POWER_TEST SDP Exported Functions
  * @brief
  * @{
  */

/**
    * @brief  POWER TEST SDP init
    * @param  void
    * @return void
    */
void power_test_sdp_init(void);

/** @} End of POWER_TEST_SDP_Exported_Functions */

/** @} End of POWER_TEST_SDP */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _POWER_TEST_SDP_H_ */
