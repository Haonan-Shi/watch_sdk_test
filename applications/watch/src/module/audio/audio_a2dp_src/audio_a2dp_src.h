/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _AUDIO_A2DP_SRC_H_
#define _AUDIO_A2DP_SRC_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdint.h>
#include "app_msg.h"
#include "stdbool.h"

typedef enum
{
    AUDIO_A2DP_SRC_EVENT_DATA_SEND  = 0xf0,
} T_AUDIO_A2DP_SRC_EVENT;

typedef enum
{
    AUDIO_A2DP_SRC_START,
    AUDIO_A2DP_SRC_STOP,
    AUDIO_A2DP_SRC_PIPE_CREATE,
    AUDIO_A2DP_SRC_PIPE_RELEASE,
    AUDIO_A2DP_SRC_STREAM_STOP,
    AUDIO_A2DP_SRC_PROFILE_DISCON,
} T_AUDIO_A2DP_SRC_PLAY_STATE_EVENT;

/*============================================================================*
 *                              Functions
 *============================================================================*/

void audio_a2dp_src_start(void);
void audio_a2dp_src_stop(void);
void audio_a2dp_src_pipe_create(void);
void audio_a2dp_src_volume_up(void);
void audio_a2dp_src_volume_down(void);
void audio_a2dp_src_set_volume(uint8_t volume);
void audio_a2dp_src_handle_msg(T_IO_MSG *msg);
void audio_a2dp_src_init(void);
void audio_a2dp_src_play_state_handle(T_AUDIO_A2DP_SRC_PLAY_STATE_EVENT event);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _AUDIO_A2DP_SRC_H_ */
