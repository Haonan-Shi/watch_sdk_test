/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_IO_MSG_H_
#define _APP_IO_MSG_H_

#include <stdint.h>
#include <stdbool.h>
#include "app_msg.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @defgroup APP_IO_MSG App IO Msg
  * @brief App IO Msg
  * @{
  */

/**
    * @brief  Send an IO message to the application task queue.
    * @param  io_msg @ref T_IO_MSG
    * @return true on success, false otherwise
    */
bool app_io_msg_send(T_IO_MSG *io_msg);

/**
    * @brief  Dispatch a received IO message to the proper handler by type.
    * @param  io_driver_msg_recv @ref T_IO_MSG
    * @return void
    */
void app_io_msg_handler(T_IO_MSG io_driver_msg_recv);

/** End of APP_IO_MSG
* @}
*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _APP_IO_MSG_H_ */
