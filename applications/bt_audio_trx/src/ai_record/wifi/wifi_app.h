/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _WIFI_APP_H_
#define _WIFI_APP_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum
{
    EVENT_UART_RX = 0,
    EVENT_UART_CMD_FLOW_CTRL = 1,
    EVENT_SDIO_INT = 2,
    EVENT_USER_APP_DEFINE = 3,
} T_WIFI_EVENT;

typedef void (*wifi_msg_cb)(void *);

typedef struct
{
    uint16_t event;
    uint16_t subtype;
    wifi_msg_cb msg_cb;
    union
    {
        uint32_t  param;
        void     *buf;
    } u;
} T_WIFI_MSG;

void wifi_enable(bool enable);
bool app_send_msg_to_wifitask(T_WIFI_MSG *p_msg);
void wifi_init(void);

/* Create the WiFi task + msg queue once (quick, non-blocking, idempotent).
 * Safe from a BLE/GATT callback; lets work be posted before wifi_power_on(). */
void wifi_task_ensure(void);

/* Blocking module bring-up (chip_en timing + SDIO + upload port). Must run on
 * the WiFi task, not a BLE callback. */
void wifi_power_on(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _WIFI_APP_H_ */
