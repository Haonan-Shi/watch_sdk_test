/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _DLPS_H_
#define _DLPS_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EVENT_BUS_TOPIC_DLPS_ALL_TOPIC   "dlps/cmd/*"
#define EVENT_BUS_TOPIC_DLPS_CMD_ENTER   "dlps/cmd/enter"
#define EVENT_BUS_TOPIC_DLPS_CMD_EXIT    "dlps/cmd/exit"
#define EVENT_BUS_TOPIC_DLPS_CMD_RUN     "dlps/cmd/run"
#define EVENT_BUS_TOPIC_DLPS_CMD_STOP    "dlps/cmd/stop"
#define EVENT_BUS_TOPIC_DLPS_CMD_DEEP_SLEEP    "dlps/cmd/dp"

#define APP_DLPS_ENTER_CHECK_SHELL_APP        0x80000000

typedef struct
{
    uint32_t cpu_mhz;   /* 40, 100, or 200 */
} T_DLPS_RUN_DATA;

#ifdef __cplusplus
}
#endif

#endif /* _DLPS_H_ */
