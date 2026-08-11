/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_AUDIO_ROUTE_H_
#define _APP_AUDIO_ROUTE_H_

#include <stdint.h>
#include <stdbool.h>
#include "audio_route.h"
#include "audio_track.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/** @defgroup APP_AUDIO_ROUTE App Audio Route
  * @brief App Audio Route
  * @{
  */

typedef struct t_dsp_tool_gain_level_data
{
    uint16_t  cmd_id;
    uint16_t  cmd_len;
    uint16_t  gain;
    uint8_t   level;
    uint8_t   category;
} T_DSP_TOOL_GAIN_LEVEL_DATA;

void app_audio_route_gain_init(void);
void app_audio_route_path_bind(T_AUDIO_TRACK_HANDLE  handle,
                               T_AUDIO_ROUTE_PATH    path);
void app_audio_route_path_unbind(T_AUDIO_TRACK_HANDLE  handle);
bool app_audio_route_entry_get(T_AUDIO_CATEGORY       category,
                               uint32_t               device,
                               T_AUDIO_ROUTE_IO_TYPE  io_type,
                               T_AUDIO_ROUTE_ENTRY   *entry);
void app_audio_route_entry_update(T_AUDIO_CATEGORY      category,
                                  uint32_t              device,
                                  T_AUDIO_TRACK_HANDLE  handle,
                                  uint8_t               entry_num,
                                  T_AUDIO_ROUTE_ENTRY   entry[]);
bool app_audio_route_dac_gain_set(T_AUDIO_CATEGORY category, uint8_t level, uint16_t gain);
bool app_audio_route_adc_gain_set(T_AUDIO_CATEGORY category, uint8_t level, uint16_t gain);


/** End of APP_AUDIO_ROUTE
* @}
*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _APP_AUDIO_ROUTE_H_ */
