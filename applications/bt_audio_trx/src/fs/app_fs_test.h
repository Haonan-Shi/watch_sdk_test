/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_FS_TEST_H_
#define _APP_FS_TEST_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "rtl876x.h"

void app_fs_test_handle_cmd_set(uint8_t app_idx, uint8_t cmd_path, uint8_t *cmd_ptr,
                                uint16_t cmd_len, uint8_t *ack_pkt);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _APP_FS_TEST_H_ */
