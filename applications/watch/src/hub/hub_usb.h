/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _WRISTBAND_USB_DETECT_H_
#define _WRISTBAND_USB_DETECT_H_

#include "rtl876x.h"
#include "trace.h"
#include "app_msg.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    USB_STOPPED,
    USB_STARTED,
} T_HUB_USB_STATUS;

void usb_detect_add_hub_task(void);
void wristband_usb_driver_init(void);
void wristband_usb_enter_dlps(void);
void wristband_usb_exit_dlps(void);
bool wristband_usb_system_wakeup_dlps_check(void);
void usb_detect_event_handler(T_IO_MSG msg);

extern void *usb_sem_handle;

#ifdef __cplusplus
}
#endif

#endif /* _WRISTBAND_TOUCH_H_ */

