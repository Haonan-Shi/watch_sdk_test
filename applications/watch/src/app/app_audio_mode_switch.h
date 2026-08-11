/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_AUDIO_MODE_SWITCH_H_
#define _APP_AUDIO_MODE_SWITCH_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define    EVENT_BUS_TOPIC_AUDIO_MODE "audio/mode"

/*============================================================================*
 *                              Functions
 *============================================================================*/
/** @defgroup APP_HFP_Exported_Functions App Hfp Functions
    * @{
    */
/**
    * @brief  audio mode switch init.
    * @param  void
    * @return void
    */
void app_audio_mode_switch_init(void);

/**
    * @brief  switch to playback, sink or src.
    * @param  mode
    * @return void
    */
void app_audio_mode_switch(uint8_t mode);

/**
    * @brief  to check whether it is in switching mode
    * @param  void
    * @return is_audio_mode_switch
    */
bool app_audio_mode_switch_status_get(void);

/**
    * @brief  get the audio mode to switch
    * @param  void
    * @return switch_to_mode
    */
uint8_t app_audio_mode_switch_mode_get(void);

/**
    * @brief  audio mode witch linkback
    * @param  void
    * @return void
    */
void app_audio_mode_switch_linkback(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _APP_HFP_H_ */
