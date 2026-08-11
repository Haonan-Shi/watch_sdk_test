/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_AVRCP_H_
#define _APP_AVRCP_H_

#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/** @defgroup APP_AVRCP App Avrcp
  * @brief App Avrcp
  * @{
  */

/*============================================================================*
 *                              Functions
 *============================================================================*/
/** @defgroup APP_AVRCP_Exported_Functions App Avrcp Functions
    * @{
    */
/**
    * @brief  avrcp module init
    * @param  void
    * @return void
    */

void app_avrcp_init(void);
void app_avrcp_support_absolute_volume(bool support);
bool app_avrcp_get_abs_vol_support(void);
void app_avrcp_set_abs_vol_support(bool support_abs);
/** @} */ /* End of group APP_AVRCP_Exported_Functions */

/** End of APP_AVRCP
* @}
*/


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _APP_AVRCP_H_ */
