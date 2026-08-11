/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_KEY_PROCESS_H_
#define _APP_KEY_PROCESS_H_

#include <stdint.h>
#include <stdbool.h>
#include "app_msg.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @defgroup APP_KEY_PROCESS App Key Process
  * @brief App Key Process
  * @{
  */

void app_gpio_button_handle_msg(T_IO_MSG *io_driver_msg_recv);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _APP_KEY_PROCESS_H_ */
