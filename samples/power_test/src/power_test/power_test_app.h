/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _POWER_TEST_APP_H_
#define _POWER_TEST_APP_H_

#include "bt_hfp_ag.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @defgroup POWER_TEST_APP Power Test App
  * @brief this file handle hfp ag profile related process
  * @{
  */

/*============================================================================*
 *                              Functions
 *============================================================================*/
/** @defgroup POWER_TEST_APP_Exported_Functions Power Test App Functions
    * @{
    */
/**
    * @brief  Power Test App module init.
    * @param  void.
    * @return void.
    */
void app_power_test_init(void);

/** @} */ /* End of group POWER_TEST_APP_Exported_Functions */
/** End of POWER_TEST_APP
* @}
*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _POWER_TEST_APP_H_ */
