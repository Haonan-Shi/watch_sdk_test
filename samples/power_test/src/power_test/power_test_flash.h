/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */


#ifndef _POWER_TEST_FLASH_H_
#define _POWER_TEST_FLASH_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum
{
    POWER_TEST_FLASH_WRITE                  = 0x00,
    POWER_TEST_FLASH_READ                   = 0x01,
    POWER_TEST_FLASH_ERASE                  = 0x02,
    POWER_TEST_FLASH_XIP                    = 0x03,
    POWER_TEST_FLASH_CACHE                  = 0x04,
    POWER_TEST_FLASH_HALF_CACHE             = 0x05,
    POWER_TEST_FLASH_DMA_READ               = 0x06,
    POWER_TEST_FLASH_WRITE_PRE              = 0x07,
    POWER_TEST_FLASH_ERASE_PRE              = 0x08,
} T_POWER_TEST_FLASH_CMD;


/**
 * @brief This api is used to CLI msg callback
 *
 */
void power_test_flash_action(T_POWER_TEST_FLASH_CMD cmd, uint8_t *buf);

/** End of DVFS_TEST_CLI
* @}
*/


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _POWER_TEST_FLASH_H_ */
