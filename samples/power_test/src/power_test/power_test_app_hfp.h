/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _POWER_TEST_APP_HFP_H_
#define _POWER_TEST_APP_HFP_H_

#include "bt_hfp.h"
#include "power_test_link.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/** @defgroup POWER_TEST_APP_HFP Power Test App HFP
  * @brief this file handle hfp profile related process
  * @{
  */

/*============================================================================*
 *                              Functions
 *============================================================================*/
/** @defgroup POWER_TEST_APP_HFP_Exported_Functions Power Test App HFP Functions
    * @{
    */
/**
    * @brief  Power Test App HFP module init.
    * @param  void
    * @return void
    */
void app_power_test_hfp_init(void);

/** @} */ /* End of group POWER_TEST_APP_HFP_Exported_Functions */
/** End of POWER_TEST_APP_HFP
* @}
*/


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _POWER_TEST_APP_HFP_H_ */
