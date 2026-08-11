/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_CMD_RECORD_H_
#define _APP_CMD_RECORD_H_


#ifdef __cplusplus
extern "C" {
#endif
/*============================================================================*
  *                           Header Files
  *============================================================================*/

#include <stdint.h>
#include <stdbool.h>
#include "app_cmd.h"

#if F_APP_RECORD_EQ_SUPPORT
#include "audio_type.h"
#endif

#if F_APP_RECORD_SAVE_SUPPORT
#define PCM_FILE_NAME_STRING   ("record.pcm")
#define OPUS_FILE_NAME_STRING  ("record.opus")
#define MP3_FILE_NAME_STRING   ("record.mp3")
#endif

/** @defgroup _XIAOMI_MIC_RECORD_H_
  * @brief
  * @{
  */

bool app_record_play_volume_set(uint8_t volume);

uint8_t app_record_play_volume_get(void);

void app_record_handle_cmd(uint8_t app_idx, T_CMD_PATH cmd_path, uint8_t *cmd_ptr,
                           uint16_t cmd_len, uint8_t *ack_pkt);

void app_record_init(void);
T_AUDIO_FORMAT_INFO parse_audio_format(uint8_t *cmd_payload);

#if F_APP_RECORD_EQ_SUPPORT
T_AUDIO_EFFECT_INSTANCE app_record_eq_instance_get(void);
#endif

/** @} End of _XIAOMI_MIC_RECORD_H_ */

#ifdef __cplusplus
}
#endif

#endif //_XIAOMI_MIC_RECORD_H_
