/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_GAP_H_
#define _APP_GAP_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @defgroup APP_GAP App Gap
  * @brief App Gap
  * @{
  */

#include "stdbool.h"

/*============================================================================*
 *                         Macros
 *============================================================================*/

/* Event bus topics for BT inquiry events (App -> Bridge).
 * Shares the "bt_evt/" namespace defined in app_bt_policy_api.h. */
#define EVENT_BUS_TOPIC_BT_EVT_INQUIRY_RESULT    "bt_evt/inquiry_result"
#define EVENT_BUS_TOPIC_BT_EVT_INQUIRY_CMPL      "bt_evt/inquiry_cmpl"

typedef enum
{
    SEARCH_STOP,
    SEARCH_START,
    SEARCH_FINISH
} T_SEARCH_STATUS;

/*============================================================================*
 *                              Functions
 *============================================================================*/
/** @defgroup APP_GAP_Exported_Functions App Gap Functions
    * @{
    */
/**
    * @brief  gap module init.
    * @param  void
    * @return void
    */
void app_gap_init(void);
bool get_linkback_status(void);
void set_linkback_status(bool flag);
void set_search_status(T_SEARCH_STATUS status);
T_SEARCH_STATUS get_search_status(void);

/**
 * @brief Start BR/EDR inquiry. Aborts any in-progress linkback first.
 *        Must run on the apptask thread.
 */
void app_bt_inquiry_start(void);

/**
 * @brief Stop BR/EDR inquiry. Must run on the apptask thread.
 */
void app_bt_inquiry_stop(void);


/** @} */ /* End of group APP_GAP_Exported_Functions */
/** End of APP_GAP
* @}
*/


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _APP_GAP_H_ */
