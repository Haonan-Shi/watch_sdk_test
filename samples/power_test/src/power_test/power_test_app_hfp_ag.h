/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _POWER_TEST_APP_HFP_AG_
#define _POWER_TEST_APP_HFP_AG_

#include "bt_hfp_ag.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @defgroup POWER_TEST_APP_HFP_AG Power Test App HFP AG
  * @brief this file handle hfp ag profile related process
  * @{
  */

/*============================================================================*
 *                              Functions
 *============================================================================*/
/** @defgroup POWER_TEST_APP_HFP_AG_Exported_Functions Power Test App HFP AG Functions
    * @{
    */
/**
    * @brief  Power Test App HFP AG module init.
    * @param  void.
    * @return void.
    */
void app_power_test_hfp_ag_init(void);

/**
    * @brief  Power Test App HFP AG connect.
    * @param  bd_addr active link bt device address.
    * @param  is_hfp  HFP HSP profile flag.
    * \return         The status of Power Test App HFP AG connect.
    * \retval true    Power Test App HFP AG connect successfully.
    * \retval false   Power Test App HFP AG connect failed.
    */
bool app_power_test_connect_hfp_ag(uint8_t *bd_addr, bool is_hfp);

/**
    * @brief  Power Test App HFP AG disconnect.
    * @param  bd_addr active link bt device address.
    * @param  is_hfp  HFP HSP profile flag.
    * \return         The status of Power Test App HFP AG diconnect.
    * \retval true    Power Test App HFP AG diconnect successfully.
    * \retval false   Power Test App HFP AG diconnect failed.
    */
bool app_power_test_disconnect_hfp_ag(uint8_t *bd_addr, bool is_hfp);

/** @} */ /* End of group POWER_TEST_APP_HFP_AG__Exported_Functions */
/** End of POWER_TEST_APP_HFP_AG
* @}
*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _POWER_TEST_APP_HFP_AG_ */
