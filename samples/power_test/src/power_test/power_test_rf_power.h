/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */


#ifndef _CLI_TX_POWER_H_
#define _CLI_TX_POWER_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


void power_test_set_tx_power(uint8_t br_1M, uint8_t edr_2M, uint8_t edr_3M, uint8_t le_1M,
                             uint8_t le_2M, uint8_t *buf);
void power_test_cont_tx(uint8_t tx_power, uint8_t packet_type, uint8_t *buf);
void power_test_packet_rx(uint8_t packet_type, uint8_t *buf);
/** End of TX_POWER_TEST_CLI
* @}
*/


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _CLI_TX_POWER_H_ */
