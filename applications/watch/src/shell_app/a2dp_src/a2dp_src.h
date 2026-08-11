/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */
#ifndef _A2DP_SRC_H_
#define _A2DP_SRC_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* gm-owned inbound command topics. The module subscribes to "a2dp_src/cmd/ *"
 * async (event_bus_async_send_to_apptask), so its callback runs on the APP TASK
 * and can call the real BT/audio public APIs directly. */
#define EVENT_BUS_TOPIC_A2DP_ALL_TOPIC      "a2dp_src/cmd/*"
#define EVENT_BUS_TOPIC_A2DP_CMD_ENTER      "a2dp_src/cmd/enter"
#define EVENT_BUS_TOPIC_A2DP_CMD_EXIT       "a2dp_src/cmd/exit"
#define EVENT_BUS_TOPIC_A2DP_CMD_SCAN_ON    "a2dp_src/cmd/scan_on"
#define EVENT_BUS_TOPIC_A2DP_CMD_SCAN_OFF   "a2dp_src/cmd/scan_off"
#define EVENT_BUS_TOPIC_A2DP_CMD_CONNECT    "a2dp_src/cmd/connect"
#define EVENT_BUS_TOPIC_A2DP_CMD_DISCONNECT "a2dp_src/cmd/disconnect"
#define EVENT_BUS_TOPIC_A2DP_CMD_PLAY       "a2dp_src/cmd/play"
#define EVENT_BUS_TOPIC_A2DP_CMD_STOP       "a2dp_src/cmd/stop"

/* Payload for CONNECT / DISCONNECT. */
typedef struct
{
    uint8_t bd_addr[6];
} T_A2DP_ADDR_DATA;

/* Payload for PLAY. */
typedef struct
{
    uint8_t  synth;          /* 1 = synthetic SBC injector, 0 = file/normal start */
    uint8_t  name_len;       /* length of name (0 = no file -> app_audio_start)   */
    char     name[32];       /* music file name when name_len > 0                 */
} T_A2DP_PLAY_DATA;

/* Synthetic SBC source (a2dp_src_audio.c). Called on the app task only. */
void a2dp_src_synth_start(void);
void a2dp_src_synth_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* _A2DP_SRC_H_ */
