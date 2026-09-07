/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_FINDMY_H_
#define _APP_FINDMY_H_

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*
 *                                Macros
 *============================================================================*/


/*============================================================================*
 *                                Types
 *============================================================================*/
typedef enum
{
    FINDMY_ENTER_PAIRING,
    FINDMY_PUT_SERIAL_NUMBER,
    FINDMY_CUST_ADV_ENABLE,
    FINDMY_CUST_ADV_DISABLE,
    FINDMY_CUST_FACTORY_RESET,
} T_FINDMY_ACTION;

typedef enum
{
    FINDMY_APP = 0,
#if CONFIG_REALTEK_FINDMY_SUPPORT_CUSTOMIZED_APP
    CUSTOMIZED_APP,
#endif
    MAX_APP_NUM,
} T_APP_SELECT;

typedef enum
{
    FINDMY_BOND = 0,
    CUST_BOND_1,
    CUST_BOND_2,
    MAX_BOND_NUM,
} T_APP_BOND;

/** @brief  FMNA only needs 1 bond information, GFPS does not need to bond */
#define BLE_BOND_NUM        MAX_BOND_NUM

/**
 * @brief  APP global data struct definition.
 */
typedef struct
{
    bool fmna_pairing_adv_enable;

    uint8_t app_bond_idx[BLE_BOND_NUM];
} T_APP_GLOBAL_DATA;

/*============================================================================*
 *                              Variables
 *============================================================================*/
extern T_APP_GLOBAL_DATA app_global_data;

/*============================================================================*
 *                              Functions
 *============================================================================*/

void app_findmy_handle_cmd_set(uint8_t app_idx, uint8_t cmd_path, uint8_t *cmd_ptr,
                               uint16_t cmd_len, uint8_t *ack_pkt);

void fmna_bond_info_restore(void);

void app_global_data_init(void);

void app_findmy_crypto_init(void);

void app_findmy_init(void);

#ifdef __cplusplus
}
#endif

#endif

