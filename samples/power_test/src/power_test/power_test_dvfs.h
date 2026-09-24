/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */


#ifndef _CLI_DVFS_H_
#define _CLI_DVFS_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief This api is used to CLI msg callback
 *
 */
void power_test_set_dvfs(uint16_t action, uint8_t *buf);
void power_test_set_cpu_freq(uint16_t action, uint8_t *buf);
void power_test_set_dsp1_freq(uint16_t action, uint8_t *buf);
void power_test_set_mclk2(uint16_t action, uint8_t *buf);
void power_test_set_32k(uint16_t action, uint8_t *buf);
/** End of DVFS_TEST_CLI
* @}
*/


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _CLI_DVFS_H_ */
