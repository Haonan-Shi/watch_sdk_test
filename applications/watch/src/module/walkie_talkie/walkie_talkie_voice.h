/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _WALKIE_TALKIE_VOICE_H_
#define _WALKIE_TALKIE_VOICE_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

void walkie_talkie_codec_init(void);
void walkie_talkie_encoder_create(void);
int walkie_talkie_encoder(uint16_t *pcm_in, uint8_t *out, int *out_len);
void walkie_talkie_encoder_destroy(void);
void walkie_talkie_decoder_create(void);
int walkie_talkie_decoder(uint8_t *in, int in_len, uint8_t *pcm_out, int *out_samples);
void walkie_talkie_decoder_destroy(void);

void walkie_talkie_player_start(void);
void walkie_talkie_player_stop(void);
void walkie_talkie_player_data_parser(uint8_t *p_data, uint16_t length);
void walkie_talkie_player_data_write(uint8_t *buf, uint16_t len, uint16_t frame_num);

void *voice_peek(int offset);
uint8_t voice_flush(uint16_t cnt);
bool voice_data_in(uint8_t *packet_pt, uint16_t packet_length, uint16_t frame_num);
void walkie_talkie_recorder_start(void);
void walkie_talkie_recorder_stop(void);
void walkie_talkie_recorder_set_voice_data(void);

#ifdef __cplusplus
}
#endif

#endif //_WALKIE_TALKIE_VOICE_H_
