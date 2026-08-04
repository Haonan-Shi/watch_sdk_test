/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_A2DP_H_
#define _APP_A2DP_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdint.h>
#include "app_msg.h"
#include "stdbool.h"

typedef enum
{
    TIMER_ID_OPEN_A2DP_STREAM         = 0x00
} T_APP_A2DP_TIMER;

typedef enum
{
    APP_A2DP_SRC_DISCONN      = 0x00,
    APP_A2DP_SRC_CONN         = 0x01,
    APP_A2DP_SRC_STREAM_STOP  = 0x02,
    APP_A2DP_SRC_STREAM_CONN  = 0x03,
    APP_A2DP_SRC_STREAM_START = 0x04,
} T_APP_A2DP_SRC_STATE;

/*============================================================================*
 *                              Functions
 *============================================================================*/
/**
    * @brief  a2dp module init
    * @param  void
    * @return void
    */
void app_a2dp_init(void);
void app_a2dp_set_active_idx(uint8_t idx);
uint8_t app_a2dp_get_active_idx(void);
uint8_t app_a2dp_get_src_credits(void);
void app_a2dp_use_src_credits(void);

/** @} */
/* End of group APP_A2DP_Exported_Functions */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _APP_A2DP_H_ */
