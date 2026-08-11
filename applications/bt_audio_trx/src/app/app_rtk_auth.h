/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_RTK_AUTH_H_
#define _APP_RTK_AUTH_H_

#include "stdint.h"
#include "app_msg.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void app_rtk_auth_key_ret_handle(T_IO_MSG *io_msg);
void app_rtk_auth_cmd_handle(uint8_t path, uint16_t length, uint8_t *p_value, uint8_t app_idx);
void app_rtk_auth_init(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _APP_RTK_AUTH_H_ */
