/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_AUDIO_DEMO_KWS_H_
#define _APP_AUDIO_DEMO_KWS_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum t_kws_sm_state
{
    KWS_STOPPED,
    KWS_STARTING,
    KWS_STARTED,
    KWS_STOPPING,
} T_KWS_SM_STATE;

/** @defgroup APP_AUDIO_DEMO_KWS Audio Kws
* @brief APP Audio Kws
* @{
*/

/*============================================================================*
 *                         Macros
 *============================================================================*/
/** @addtogroup APP_AUDIO_DEMO_KWS_Exported_Macros Audio Kws Exported Macros
  * @brief
  * @{
  */



/** End of APP_AUDIO_DEMO_KWS_Exported_Macros
* @}
*/

/*============================================================================*
 *                         Functions
 *============================================================================*/
/** @defgroup APP_AUDIO_DEMO_KWS_Exported_Functions Audio Kws Exported Functions
  * @brief
  * @{
  */

/**
    * @brief  Start Audio Kws
    * @param  void
    * @return bool
*/
bool app_audio_kws_enable(void);

/**
    * @brief  Stop Audio Kws
    * @param  void
    * @return bool
*/
bool app_audio_kws_disable(void);


/** @} End of APP_AUDIO_DEMO_KWS_Exported_Functions */

/** @} End of APP_AUDIO_DEMO_KWS */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _APP_AUDIO_DEMO_KWS_H_ */
