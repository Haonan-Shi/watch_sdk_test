/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_FLAGS_H_
#define _APP_FLAGS_H_

/** @defgroup  AUDIO_DEMO_Config App Configuration
    * @brief This file is used to config app functions.
    * @{
    */
/*============================================================================*
 *                              Constants
 *============================================================================*/

/** @brief  Config Audio Demo */
#define NOTIFICATION_DEMO_SUPPORT   1
#define APT_DEMO_SUPPORT            1
#define LLAPT_DEMO_SUPPORT          1
#define LINE_IN_DEMO_SUPPORT        1
#define ANC_DEMO_SUPPORT            1
#define VAD_DEMO_SUPPORT            1
#define KWS_DEMO_SUPPORT            1
#define AUDIO_PIPE_DEMO_SUPPORT     1
#define AUDIO_TRACK_DEMO_SUPPORT    1
#define F_APP_HIFI4_SUPPORT         0

#if CONFIG_SOC_SERIES_RTL8773D
#undef F_APP_HIFI4_SUPPORT
#define F_APP_HIFI4_SUPPORT         1
#endif

/** @} */ /* End of group AUDIO_DEMO_Config */

#endif
