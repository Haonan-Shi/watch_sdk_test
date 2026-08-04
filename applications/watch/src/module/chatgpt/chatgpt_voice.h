/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef __CHATGPT_VOICE_H__
#define __CHATGPT_VOICE_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble_transport.h"

void chatgpt_start_record(void);
void chatgpt_record_init(_chatgpt_scene_thread *p_scene);
void chatgpt_stop_record(void);
void chatgpt_set_voice_recv_status(bool recv_complete);

void chatgpt_play_start(_chatgpt_scene_thread *p_scene);
void chatgpt_play_write(_chatgpt_scene_thread *p_scene);
void chatgpt_play_stop(void);
void chatgpt_play_timer_start(void);

#endif //__CHATGPT_VOICE_H__
