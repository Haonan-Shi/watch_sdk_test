/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _AUDIO_A2DP_SINK_H_
#define _AUDIO_A2DP_SINK_H_

#include <stdint.h>
#include <stdbool.h>
#include "btm.h"
#include "audio_track.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @defgroup APP_AUDIO App Audio
  * @brief App Audio
  * @{
  */

/*============================================================================*
 *                              Functions
 *============================================================================*/
/** @defgroup APP_AUDIO_Exported_Functions App Audio Functions
    * @{
    */


void audio_a2dp_sink_stream_start_handle(T_BT_EVENT_PARAM_A2DP_STREAM_START_IND *stream_ind);
void audio_a2dp_sink_data_ind(uint8_t *bd_addr, uint8_t *data_ind);
void audio_a2dp_sink_track_release(void);
void audio_a2dp_sink_volume_up(void);
void audio_a2dp_sink_volume_down(void);
void audio_a2dp_sink_set_volume(uint8_t volume);



/** @} */ /* End of group APP_AUDIO_Exported_Functions */

/** End of APP_AUDIO
* @}
*/


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _AUDIO_A2DP_SINK_H_ */
