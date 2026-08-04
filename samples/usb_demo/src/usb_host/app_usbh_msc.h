/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef __APP_USBH_MSC_H
#define __APP_USBH_MSC_H
#include "app_io_msg.h"

#define IO_MSG_TYPE_USBH_MSC 0xF0

void app_usbh_msg_handle(T_IO_MSG *msg);
void app_usbh_msc_init(void);

#endif // !__APP_USBH_MSC_H