/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_MULTILINK_H_
#define _APP_MULTILINK_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/** @defgroup APP_MULTILINK App Multilink
  * @brief App Multilink
  * @{
  */

/*============================================================================*
 *                              Macros
 *============================================================================*/
/** @defgroup APP_MULTILINK_Exported_Macros App Multilink Macros
   * @{
   */
#define UPDATE_ACTIVE_A2DP_INDEX_TIMER  1500
/** End of APP_MULTILINK_Exported_Macros
    * @}
    */

/*============================================================================*
 *                              Types
 *============================================================================*/
/** @defgroup APP_MULTILINK_Exported_Types App Multilink Types
    * @{
    */
/** @brief Multilink handle connect, disconnect, related player status event */
typedef enum
{
    JUDGE_EVENT_A2DP_CONNECTED,
    JUDGE_EVENT_MEDIAPLAYER_PLAYING,
    JUDGE_EVENT_A2DP_START,
    JUDGE_EVENT_A2DP_DISC,
    JUDGE_EVENT_DSP_SILENT,
    JUDGE_EVENT_A2DP_STOP,

    JUDGE_EVENT_TOTAL
} T_APP_JUDGE_A2DP_EVENT;

/** @brief Multilink update active a2dp link, delay role switch timer */
typedef enum
{
    MULTILINK_UPDATE_ACTIVE_A2DP_INDEX,

    MULTILINK_TOTAL
} T_MULTILINK_TIMER;
/** End of APP_MULTILINK_Exported_Types
    * @}
    */

/*============================================================================*
 *                              Functions
 *============================================================================*/
/** @defgroup APP_MULTILINK_Exported_Functions App Multilink Functions
    * @{
    */

/** @} */ /* End of group APP_MULTILINK_Exported_Functions */

/** End of APP_MULTILINK
* @}
*/


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _APP_MULTILINK_H_ */
