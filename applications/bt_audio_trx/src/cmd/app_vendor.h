/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_VENDOR_H_
#define _APP_VENDOR_H_

#include "stdint.h"
#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief RF XTAL Auto K Command
 *
 * @param channel(1 byte)
 * @param upperbound (1 byte)
 * @param lowerbound (1 byte)
 * @param offset (1 byte)
 * @return void
 */
void app_vendor_rf_xtak_k(uint8_t channel, uint8_t upperbound, uint8_t lowerbound, uint8_t offset);

/**
 * @brief Get RF XTAL K result Command
 *
 * @param void
 * @return void
 */
void app_vendor_get_xtak_k_result(void);

/**
 * @brief Write RF XTAL K result command
 *
 * @param xtal_val RF XTAL K result
 * @return void
 */
void app_vendor_write_xtak_k_result(uint8_t xtal_val);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* _APP_VENDOR_H_ */
