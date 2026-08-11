/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_PBAP_H_
#define _APP_PBAP_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/** @defgroup APP_PBAP App Pbap
  * @brief App Pbap
  * @{
  */

/*============================================================================*
 *                         Macros
 *============================================================================*/

/* Event bus topics for PBAP (App -> GUI) */
#define EVENT_BUS_TOPIC_PBAP_ALL_TOPIC        "pbap/*"
#define EVENT_BUS_TOPIC_PBAP_CALLER_ID       "pbap/caller_id"

/*============================================================================*
 *                         Types
 *============================================================================*/

/**
    * @brief  init pbap module.
    * @param  void
    * @return void
    */
void app_pbap_init(void);

/**
    * @brief  Get caller name reported by PBAP (for incoming call display)
    * @return caller name if available, empty string otherwise
    */
const char *app_pbap_get_caller_name(void);

/**
    * @brief  Clear caller name (should be called when new incoming call starts)
    */
void app_pbap_clear_caller_name(void);

/** End of APP_PBAP
* @}
*/


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _APP_PBAP_H_ */
