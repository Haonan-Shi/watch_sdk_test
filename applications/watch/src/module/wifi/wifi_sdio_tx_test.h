/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef __WIFI_SDIO_TX_TEST_H__
#define __WIFI_SDIO_TX_TEST_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    WIFI_SDIO_TX_EVENT = 0,
} T_WIFI_SDIO_TO_APP_TYPE;

int  sdio_tx_test(char *p_param);
void sdio_to_app_task(T_WIFI_SDIO_TO_APP_TYPE type, void *data);
bool sdcard_test_start(void);
bool sdcard_test_end(void);

#ifdef __cplusplus
}
#endif

#endif /* __WIFI_SDIO_TX_TEST_H__ */
