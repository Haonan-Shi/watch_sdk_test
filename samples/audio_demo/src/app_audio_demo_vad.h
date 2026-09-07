/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_AUDIO_DEMO_VAD_H_
#define _APP_AUDIO_DEMO_VAD_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum t_vad_sm_state
{
    VAD_STOPPED,
    VAD_STARTING,
    VAD_STARTED,
    VAD_STOPPING,
} T_VAD_SM_STATE;

/** @defgroup APP_AUDIO_DEMO_VAD Audio Vad
* @brief APP Audio Vad
* @{
*/

/*============================================================================*
 *                         Macros
 *============================================================================*/
/** @addtogroup APP_AUDIO_DEMO_VAD_Exported_Macros Audio Vad Exported Macros
  * @brief
  * @{
  */



/** End of APP_AUDIO_DEMO_VAD_Exported_Macros
* @}
*/

/*============================================================================*
 *                         Functions
 *============================================================================*/
/** @defgroup APP_AUDIO_DEMO_VAD_Exported_Functions Audio Vad Exported Functions
  * @brief
  * @{
  */

/**
    * @brief  Start/Stop Audio Vad
    * @param  void
    * @return bool
    */
bool app_audio_vad_enable(uint8_t  vad_type,
                          uint8_t  bit_width,
                          uint16_t frame_length);
bool app_audio_vad_disable(void);


/** @} End of APP_AUDIO_DEMO_VAD_Exported_Functions */

/** @} End of APP_AUDIO_DEMO_VAD */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _APP_AUDIO_DEMO_VAD_H_ */
