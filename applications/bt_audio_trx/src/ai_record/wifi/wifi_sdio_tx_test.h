/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _WIFI_SDIO_TX_TEST_H_
#define _WIFI_SDIO_TX_TEST_H_

typedef enum
{
    WIFI_SDIO_TX_EVENT = 0,     /**< Enter camera view */

} T_WIFI_SDIO_TO_APP_TYPE;

void sdio_to_app_task(T_WIFI_SDIO_TO_APP_TYPE type, void *data);

#endif
