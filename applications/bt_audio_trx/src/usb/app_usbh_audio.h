/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_USBH_AUDIO_H_
#define _APP_USBH_AUDIO_H_

#include <stdint.h>
#include "clk_mgr.h"

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/**
 * @brief   clk_mgr user of the USB host audio path.
 *          Created by app_usbh_audio_init(), voted high/normal by the
 *          CMD_USBH_AUDIO_CONTROL handler. NULL if the create failed.
 */
extern T_CLK_USER_HANDLE clk_user_usbh;

/**
 * @brief   Initialize the USB host audio module.
 */
void app_usbh_audio_init(void);

/**
 * @brief   Override the preferred stream format of the USB host audio path.
 * @param   rate        Sample rate in Hz.
 * @param   ch          Number of channels.
 * @param   bits        Bit resolution.
 * @param   buf_intrvl  Buffer process interval.
 */
void app_usbh_audio_set_param(uint32_t rate, uint32_t ch, uint32_t bits, uint32_t buf_intrvl);

#ifdef __cplusplus
}
#endif

#endif
