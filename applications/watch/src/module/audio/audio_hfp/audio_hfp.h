/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _AUDIO_HFP_H_
#define _AUDIO_HFP_H_

#include "bt_hfp.h"
#include "btm.h"
#include "audio.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/** @defgroup AUDIO_HFP Audio Hfp
  * @brief this file handle hfp profile related process
  * @{
  */

/*============================================================================*
 *                         Macros
 *============================================================================*/

/* Event bus topics for HFP audio (App -> GUI) */
#define EVENT_BUS_TOPIC_HFP_AUDIO_ALL_TOPIC    "hfp_audio/*"
#define EVENT_BUS_TOPIC_HFP_AUDIO_VOLUME       "hfp_audio/volume"

/*============================================================================*
 *                         Types
 *============================================================================*/

typedef enum
{
    AUDIO_COEXIST_CALL_STATUS_UPDATE,
    AUDIO_COEXIST_SCO_CONN_IND,
} T_AUDIO_COEXIST_EVENT;

/*============================================================================*
 *                              Functions
 *============================================================================*/
/** @defgroup APP_HFP_Exported_Functions App Hfp Functions
    * @{
    */
void audio_coexist_handle(T_AUDIO_COEXIST_EVENT event);

void audio_sco_discon_handle(T_BT_EVENT_PARAM_SCO_DISCONNECTED *sco_disconnected);

void audio_sco_conn_cmpl_handle(T_BT_EVENT_PARAM_SCO_CONN_CMPL *sco_conn_cmpl);

void audio_sco_data_read(T_AUDIO_EVENT_PARAM *param);

void audio_sco_data_ind(T_BT_EVENT_PARAM_SCO_DATA_IND *sco_data_ind);

/**
    * @brief  Send a command to synchronize speaker gain level.
    * @param  bd_addr: remote BT address.
    * @param  level: local gain level, range from 0 to 15.
    * @return The status of sending the command request.
    */
bool audio_hfp_speaker_gain_level_report(uint8_t *bd_addr, uint8_t level);

/**
    * @brief  control hfp mic mute.
    * @param  void
    * @return void
    */
void audio_hfp_mute_ctrl(void);

/**
    * @brief  hfp volume up.
    * @param  void
    * @return void
    */
void audio_hfp_volume_up(void);

/**
    * @brief  hfp volume down.
    * @param  void
    * @return void
    */
void audio_hfp_volume_down(void);

/**
    * @brief  hfp volume set.
    * @param  void
    * @return void
    */
void audio_hfp_set_volume(uint8_t volume);

bool audio_hfp_check_mic_mute_enable(void);

void app_mmi_mic_mute_set(void);

void app_mmi_mic_unmute_set(void);

void audio_hfp_mute_ctrl(void);

/** @} */ /* End of group APP_HFP_Exported_Functions */
/** End of APP_HFP
* @}
*/

uint8_t app_hfp_get_active_idx(void);

void audio_hfp_init(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _AUDIO_HFP_H_ */
