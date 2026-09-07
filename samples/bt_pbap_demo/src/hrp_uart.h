/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _CONSOLE_UART_H_
#define _CONSOLE_UART_H_

#include <stdint.h>
#include <stdbool.h>
#include "console.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


bool hrp_uart_tx(uint8_t *buf, uint32_t len);


bool hrp_uart_init(P_CONSOLE_CALLBACK p_callback);


/** End of APP_RWS_CONSOLE
* @}
*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _CONSOLE_UART_H_ */
