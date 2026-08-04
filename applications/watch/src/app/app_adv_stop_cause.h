/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_ADV_STOP_CAUSE_H_
#define _APP_ADV_STOP_CAUSE_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define APP_STOP_ADV_CAUSE_FINDMY                  (0x01)
#define APP_STOP_ADV_CAUSE_GFPS_ACTION_IDLE        (0x02)
#define APP_STOP_ADV_CAUSE_GFPS_FINDER             (0x03)
#define APP_STOP_ADV_CAUSE_DULT_PAUSE              (0x04)
#define APP_STOP_ADV_CAUSE_GFPS_FINDER_KEY_PRESS   (0x05)
#define APP_STOP_ADV_CAUSE_GFPS_FINDER_UPDATE_EIK  (0x06)
// you can add new cause in here
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _APP_BLE_GAP_H_ */
