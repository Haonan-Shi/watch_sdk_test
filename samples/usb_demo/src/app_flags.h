/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */


#ifndef _APP_FLAGS_H_
#define _APP_FLAGS_H_

//----- [Peripheral related] -----
#define F_APP_HIFI4_SUPPORT                 0
// #define F_APP_USB_AUDIO_SUPPORT             1
// #define F_APP_USB_HID_SUPPORT               1

#if CONFIG_REALTEK_USB_AUDIO_ENABLE
#undef F_APP_USB_AUDIO_SUPPORT
#define F_APP_USB_AUDIO_SUPPORT             1
#undef F_APP_USB_HID_SUPPORT
#define F_APP_USB_HID_SUPPORT               1
#endif

#if  CONFIG_REALTEK_USB_HID_MOUSE_ENABLE || CONFIG_REALTEK_USB_HID_KEYBOARD_ENABLE
#undef F_APP_USB_AUDIO_SUPPORT
#define F_APP_USB_AUDIO_SUPPORT             0
#undef F_APP_USB_HID_SUPPORT
#define F_APP_USB_HID_SUPPORT               1
#define GPIO_KEY_EN                         1
#endif

#if F_APP_USB_AUDIO_SUPPORT & (CONFIG_SOC_SERIES_RTL8773D || TARGET_RTL8773DFL)
#undef F_APP_HIFI4_SUPPORT
#define F_APP_HIFI4_SUPPORT                             1

#endif

#endif


