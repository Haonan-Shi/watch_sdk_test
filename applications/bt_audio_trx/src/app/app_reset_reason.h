/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_RESET_REASON_
#define _APP_RESET_REASON_


#ifdef __cplusplus
extern "C" { /* __cplusplus */
#endif

/**
 * The maximum value of RTL87x3E/RTL87x3EP is 0x3F, while the other ICs is 0xFF.
 * for RTL87x3E/RTL87x3EP
 *      1~31:   Reserved for app use
 *      32~63: Reserved for SDK lib and patch use
 * for other ICs such as RTL87x3D/RTL87x3G
 *      1~127:   Reserved for app use
 *      128~255: Reserved for SDK lib and patch use
*/
#define APP_RESET_REASON_CFU                        1
#define APP_RESET_REASON_OTA                        2
#define APP_RESET_REASON_SINGLE_TONE_TIMEOUT        3
#define APP_RESET_REASON_SINGLE_TONE_VP_STOP        4
#define APP_RESET_REASON_SINGLE_TONE_RINGTONE_STOP  5
#define APP_RESET_REASON_SWITCH_TO_HCI              6
#define APP_RESET_REASON_ONE_WIRE_UART              7
#define APP_RESET_REASON_MMI_REBOOT                 8
#define APP_RESET_REASON_DEVICE_TIMER_REBOOT        9
#define APP_RESET_REASON_DATA_CAPTURE               10
#define APP_RESET_REASON_MCU_CFG_LOAD_FAIL          11

#ifdef __cplusplus
} /* __cplusplus */
#endif

#endif
