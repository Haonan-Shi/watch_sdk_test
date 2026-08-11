/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef __PPG_FLASH_HANDLE_H__
#define __PPG_FLASH_HANDLE_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#include "stdint.h"


typedef struct
{
    uint8_t valid;
    uint8_t idx;
    uint16_t year;
    uint8_t mon;
    uint8_t day;
    uint8_t hr;
    uint8_t min;
    uint8_t sec;
    uint8_t rsv[3];
    uint32_t start_addr;
    uint32_t len;
    uint32_t sector_len;
} FLASH_SAVE_IDX;

void ppg_flash_save_data_init(void);
void ppg_flash_save_data(uint32_t len, uint8_t *data, bool is_last);
FLASH_SAVE_IDX ppg_flash_read_data_init(uint8_t idx);
void ppg_flash_get_data(uint8_t idx, uint32_t len, uint8_t *data);
void ppg_flash_erase_data(void);
void ppg_flash_data_init(void);
uint8_t ppg_flash_get_stored_data_num(void);
uint8_t ppg_flash_get_remain_data_num(void);
FLASH_SAVE_IDX ppg_flash_get_flash_saved_time(uint8_t idx);
uint32_t ppg_flash_update_remain(void);
#endif

