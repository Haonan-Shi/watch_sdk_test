/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */
#ifndef _CLI_POWER_H_
#define _CLI_POWER_H_

#include <stdint.h>
#include <stdbool.h>
#include "pm.h"
#include "power_test_le.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/** @defgroup POWER_TEST_CLI App Cli
  * @brief POWER_TEST_CLI Cli
  * @{
  */

/** @defgroup POWER_TEST_CLI App Cli Power
  * @brief POWER_TEST_CLI Cli
  * @{
  */
typedef enum
{
    POWER_STATE_ID                  = 0xE0,
    DVFS_ID,
    CPU_FREQ_ID,
    DSP1_FREQ_ID,
#ifdef CONFIG_SOC_SERIES_RTL8773D
    MCLK2_ID,
#endif
    CLOCK_32K_ID,
    TX_POWER_ID,
    CONT_TX_ID,
    PACKET_RX_ID,
    PLATFORM_ID,
    GAP_LEGACY_ID,
    GAP_LE_ID,
} T_POWER_TEST_ID;

/** @brief Power test command */
typedef enum
{
    POWER_TEST_CMD_SET_LPS                  = 0x00,
    POWER_TEST_CMD_SET_DLPS                 = 0x01,
    POWER_TEST_CMD_SET_DLPS_RET             = 0x02,
    POWER_TEST_CMD_SET_DLPS_PFM             = 0x03,
    POWER_TEST_CMD_SET_POWER_DOWN           = 0x04,
    POWER_TEST_CMD_SET_POWER_OFF            = 0x05,
    POWER_TEST_CMD_SET_BT_MAC_SELLP         = 0x06,
    POWER_TEST_CMD_SET_BT_MAC_ACTIVE        = 0x07,
    POWER_TEST_CMD_STATE_MAX_INDEX          = 0x08,
} T_POWER_TEST_STATE_CMD;

typedef enum
{
#ifdef CONFIG_SOC_SERIES_RTL8773D
    POWER_TEST_CMD_SET_DVFS_NORMAL0V9       = 0x00,
    POWER_TEST_CMD_SET_DVFS_NORMAL0V8       = 0x01,
    POWER_TEST_CMD_SET_DVFS_LOW0V9          = 0x02,
    POWER_TEST_CMD_SET_DVFS_LOW0V8          = 0x03,
    POWER_TEST_CMD_SET_DVFS_LOW0V7          = 0x04,
    POWER_TEST_CMD_SET_DVFS_LOW0V65         = 0x05,
    POWER_TEST_CMD_SET_DVFS_LOW0V6125       = 0x06,
    POWER_TEST_CMD_SET_DVFS_HIGH            = 0x07,
    POWER_TEST_CMD_SET_DVFS_LOW             = 0x08,
    POWER_TEST_CMD_SET_VCORE2_CLOSE         = 0x09,
#else
    POWER_TEST_CMD_SET_DVFS_NORMAL1V1       = 0x00,
    POWER_TEST_CMD_SET_DVFS_NORMAL0V9       = 0x01,
    POWER_TEST_CMD_SET_DVFS_HIGH            = 0x02,
    POWER_TEST_CMD_SET_DVFS_LOW             = 0x03,
#endif
} T_POWER_TEST_DVFS_CMD;

typedef enum
{
    POWER_TEST_CMD_CPU_SLEEP                = 0x00,
    POWER_TEST_CMD_CPU_ACTIVE               = 0x01,
    POWER_TEST_CMD_CPU_FREQ_625K            = 0x02,
    POWER_TEST_CMD_CPU_FREQ_MAX             = 0x03,
} T_POWER_TEST_CPU_FREQ_CMD;

typedef enum
{
    POWER_TEST_CMD_DSP1_DISABLE             = 0x00,
} T_POWER_TEST_DSP1_FREQ_CMD;

#ifdef CONFIG_SOC_SERIES_RTL8773D
typedef enum
{
    POWER_TEST_CMD_MCLK2_XTAL               = 0x00,
    POWER_TEST_CMD_MCLK2_PLL                = 0x01,
} T_POWER_TEST_MCLK2_CMD;
#endif

typedef enum
{
    POWER_TEST_CMD_32K_ON                   = 0x00,
    POWER_TEST_CMD_32K_OFF                  = 0x01,
} T_POWER_TEST_32K_CMD;

typedef enum
{
    POWER_TEST_CMD_INQUIRY_SCAN_PARAM_SET   = 0x00,
    POWER_TEST_CMD_PAGE_SCAN_PARAM_SET      = 0x01,
    POWER_TEST_CMD_RADIO_MODE_SET           = 0x02,
    POWER_TEST_CMD_SNIFF_ENTER              = 0x03,
    POWER_TEST_CMD_SNIFF_EXIT               = 0x04,
    POWER_TEST_CMD_LINK_DEAULT_POLICY_SET   = 0x05,
    POWER_TEST_CMD_LINK_POLICY_SET          = 0x06,
    POWER_TEST_CMD_INQUIRY_START            = 0x07,
    POWER_TEST_CMD_INQUIRY_STOP             = 0x08,
    POWER_TEST_CMD_PAGE_START               = 0x09,
    POWER_TEST_CMD_PAGE_STOP                = 0x0A,
    POWER_TEST_CMD_HFP_AG_CONN              = 0x0B,
    POWER_TEST_CMD_HFP_AG_DISCON            = 0x0C,
    POWER_TEST_CMD_LEGACY_DISCONNECT        = 0x0D,
    POWER_TEST_CMD_REMOVE_BOND              = 0x0E,
} T_POWER_TEST_GAP_LEGACY_CMD;

/**
 * @brief This api is used to register power test command
 *
 * @retval none
 */

/** End of POWER_TEST_CLI
* @}
*/

/** End of POWER_TEST_CLI
* @}
*/


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _CLI_POWER_H_ */
