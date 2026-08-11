/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _AUDIO_RECORD_H_
#define _AUDIO_RECORD_H_


#ifdef __cplusplus
extern "C" {
#endif
/*============================================================================*
  *                           Header Files
  *============================================================================*/

#include <stdint.h>
#include <stdbool.h>
#include "autoconf.h"

#if  CONFIG_WIFI_AUDIO_PLAYER
void wifi_audio_player_init(void);
void wifi_audio_stop_play(void);
void wifi_audio_rtp_depayload(uint8_t *pkt, uint16_t len);
#endif //CONFIG_WIFI_AUDIO_PLAYER

#if  CONFIG_WIFI_AUDIO_RECORDER
void wifi_recorder_start(void);
void wifi_recorder_stop(void);
uint32_t wifi_audio_record_buffer_get_data_size(void);
uint32_t wifi_audio_record_buffer_read(uint8_t *data, uint32_t len);
#endif //CONFIG_WIFI_AUDIO_RECORDER


#ifdef __cplusplus
}
#endif

#endif //_AUDIO_RECORD_H_
