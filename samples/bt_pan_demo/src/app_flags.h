/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_FLAGS_H_
#define _APP_FLAGS_H_

/** @defgroup  BT_HID_DEMO_Config App Configuration
    * @brief This file is used to config app functions.
    * @{
    */
/*============================================================================*
 *                              Constants
 *============================================================================*/

#define F_APP_PAN_PANU_SUPPORT     1
#define F_APP_PAN_NAP_SUPPORT      0
#define F_APP_PAN_GN_SUPPORT       0

#define F_APP_DSP_SHM_80KB_TO_MCU_CHECK_SUPPORT   1

#if (CONFIG_SOC_SERIES_RTL8773D || TARGET_RTL8773DFL || TARGET_RTL87X3EP ||CONFIG_SOC_SERIES_RTL87X3G)
#undef F_APP_DSP_SHM_80KB_TO_MCU_CHECK_SUPPORT
#define F_APP_DSP_SHM_80KB_TO_MCU_CHECK_SUPPORT         0
#endif
/** @} */ /* End of group BT_HID_DEMO_Config */

#endif
