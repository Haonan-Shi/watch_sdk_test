/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdlib.h>
#include "trace.h"
#include "ppg_flash_handle.h"
#include "os_timer.h"
#include "app_msg.h"
#include "rtl876x_nvic.h"
#include "os_sched.h"
#include "ftl.h"
#include <string.h>
#include "time.h"
#include "fmc_api.h"
#include "flash_map.h"
#include "module_global_data.h"

#define FLASH_END_ADDR                           0x704C2000
#define TRANSLUCENT_BACKGROUND_BIN               0x70350000
#define HRS_DATA_START_ADDRESS                  ((0x70350000 / 4096 + 1) * 4096)

#define START_ADDR                              TRANSLUCENT_BACKGROUND_BIN
#define FLASH_MAX_SIZE                           (FLASH_END_ADDR - START_ADDR)
#define MAX_SAVE_LEN                             10

FLASH_SAVE_IDX cur_idx[MAX_SAVE_LEN];
FLASH_SAVE_IDX temp_saving_idx = {.idx = 0xff};
uint32_t write_offset = 0;
uint32_t read_offset = 0;
uint32_t remain_size = FLASH_MAX_SIZE;
uint8_t flash_erase_flag = 0;

uint32_t ppg_flash_update_remain(void)
{
    uint8_t idx = 0;
    uint32_t ret = 0;
    for (idx = 0; idx < MAX_SAVE_LEN; idx++)
    {
        if (cur_idx[idx].valid != 1)
        {
            break;
        }
        ret += cur_idx[idx].sector_len;
    }
    remain_size = FLASH_MAX_SIZE > ret ? (FLASH_MAX_SIZE - ret) : 0;
    return remain_size;
}

static FLASH_SAVE_IDX ppg_flash_find_start_addr(void)
{
    uint8_t idx = 0;
    FLASH_SAVE_IDX ret = {0};
    ret.idx = 0xff;
    ret.start_addr = START_ADDR;

    APP_PRINT_TRACE1("Cheat remain size %d", remain_size);
    if (remain_size == 0)
    {
        return ret;
    }

    for (idx = 0; idx < MAX_SAVE_LEN; idx++)
    {
        if (cur_idx[idx].valid != 1)
        {
            ret.idx = idx;
            APP_PRINT_TRACE2("Cheat get flash %d %x", idx, ret.start_addr);
            return ret;
        }
        ret.start_addr += cur_idx[idx].sector_len;
    }
    //fmc_flash_nor_erase(ret.start_addr, FMC_FLASH_NOR_ERASE_CHIP);
    return ret;
}

void ppg_flash_data_init(void)
{
    fmc_flash_nor_read(HRS_DATA_START_ADDRESS, (uint8_t *)&cur_idx[0],
                       sizeof(FLASH_SAVE_IDX) * MAX_SAVE_LEN);
    ppg_flash_update_remain();
}

void ppg_flash_save_data_init(void)
{
    temp_saving_idx = ppg_flash_find_start_addr();
    temp_saving_idx.year = RtkWristbandSys.Global_Time.tm_year;
    temp_saving_idx.mon = RtkWristbandSys.Global_Time.tm_mon;
    temp_saving_idx.day = RtkWristbandSys.Global_Time.tm_mday;
    temp_saving_idx.hr = RtkWristbandSys.Global_Time.tm_hour;
    temp_saving_idx.min = RtkWristbandSys.Global_Time.tm_min;
    temp_saving_idx.sec = RtkWristbandSys.Global_Time.tm_sec;

    write_offset = 0;
}
void ppg_flash_save_data(uint32_t len, uint8_t *data, bool is_last)
{
    APP_PRINT_TRACE2("Cheat save entry %d, %d", remain_size, temp_saving_idx.idx);
    if ((temp_saving_idx.idx < MAX_SAVE_LEN) && (remain_size > 0))
    {
        if (len != 0)
        {
            uint32_t retval;
            if (remain_size > write_offset + len)
            {
                retval = fmc_flash_nor_write(temp_saving_idx.start_addr + write_offset, data, len);
                write_offset += len;
            }
            else
            {
                len = write_offset + len - remain_size;
                retval = fmc_flash_nor_write(temp_saving_idx.start_addr + write_offset, data, len);
                write_offset += len;
                is_last = true;
            }
            APP_PRINT_TRACE2("Cheat save ret %d, %d", retval, write_offset);
        }

        if (is_last)
        {
            temp_saving_idx.len = write_offset;
            temp_saving_idx.sector_len = ((write_offset / 4096) + 1) * 4096;
            remain_size = remain_size > temp_saving_idx.sector_len ? remain_size - temp_saving_idx.sector_len :
                          0;
            temp_saving_idx.valid = 1;
            memcpy(&cur_idx[temp_saving_idx.idx], &temp_saving_idx, sizeof(FLASH_SAVE_IDX));
            bool ret = fmc_flash_nor_write(HRS_DATA_START_ADDRESS + temp_saving_idx.idx * sizeof(
                                               FLASH_SAVE_IDX), (uint8_t *)&cur_idx[temp_saving_idx.idx], sizeof(FLASH_SAVE_IDX));
            APP_PRINT_TRACE3("Cheat save ret %d, %d, %d",  ret, temp_saving_idx.len,
                             temp_saving_idx.sector_len);
            memset(&temp_saving_idx, 0, sizeof(FLASH_SAVE_IDX));
            temp_saving_idx.idx = 0xff;
        }
    }
}

FLASH_SAVE_IDX ppg_flash_read_data_init(uint8_t idx)
{
    FLASH_SAVE_IDX temp = {0};
    temp.idx = 0xff;
    read_offset = 0;
    if ((idx < MAX_SAVE_LEN) && (cur_idx[idx].valid == 1))
    {
        return cur_idx[idx];
    }
    return temp;
}

void ppg_flash_get_data(uint8_t idx, uint32_t len, uint8_t *data)
{
    if (idx < MAX_SAVE_LEN && cur_idx[idx].valid)
    {
        uint32_t retval = fmc_flash_nor_read(cur_idx[idx].start_addr + read_offset, data, len);
        APP_PRINT_TRACE3("Cheat load ret %d, %x, %d", retval, cur_idx[idx].start_addr, read_offset);
        read_offset += len;
    }
}

void ppg_flash_erase_data(void)
{
    uint32_t erase_size = 0;
    uint32_t erase_total_size = FLASH_END_ADDR - 0x2D4E000;
    uint32_t start_address = 0x2D4E000;

    uint32_t block_num = erase_total_size / FMC_SEC_BLOCK_LEN;
    for (uint32_t i = 0; i < block_num; i++)
    {
        APP_PRINT_TRACE1("app_erase_whole_image erase block start_address 0x%x", start_address);
        fmc_flash_nor_erase(start_address, FMC_FLASH_NOR_ERASE_BLOCK);
        erase_size += FMC_SEC_BLOCK_LEN;
        start_address += FMC_SEC_BLOCK_LEN;
    }
    uint8_t sector_num = (erase_total_size - erase_size) / FMC_SEC_SECTION_LEN;
    for (uint8_t i = 0; i < sector_num; i++)
    {
        APP_PRINT_TRACE1("app_erase_whole_image erase sector start_address 0x%x", start_address);
        fmc_flash_nor_erase(start_address, FMC_FLASH_NOR_ERASE_SECTOR);
        erase_size += FMC_SEC_SECTION_LEN;
        start_address += FMC_SEC_SECTION_LEN;
    }
    remain_size = FLASH_MAX_SIZE;
    memset(cur_idx, 0, sizeof(FLASH_SAVE_IDX) * MAX_SAVE_LEN);
    APP_PRINT_INFO3("erase_total_size %u, erase_size %u, not clear size %u",
                    erase_total_size, erase_size, erase_total_size - erase_size);
    flash_erase_flag = 1;
}

uint8_t ppg_flash_get_stored_data_num(void)
{
    uint8_t ret = 0;
    for (uint8_t idx = 0; idx < MAX_SAVE_LEN; idx ++)
    {
        if (cur_idx[idx].valid == 1)
        {
            ret += 1;
        }
        else
        {
            break;
        }
    }
    APP_PRINT_TRACE1("ppg_flash_get_stored_data_num %d", ret);
    return ret;
}

uint8_t ppg_flash_get_remain_data_num(void)
{
    uint8_t ret = 0;

    if (remain_size == 0)
    {
        return ret;
    }

    for (uint8_t idx = 0; idx < MAX_SAVE_LEN; idx ++)
    {
        if (cur_idx[idx].valid == 1)
        {
            ret += 1;
        }
        else
        {
            break;
        }
    }
    APP_PRINT_TRACE1("ppg_flash_get_remain_data_num %d", ret);
    return MAX_SAVE_LEN - ret;
}

FLASH_SAVE_IDX ppg_flash_get_flash_saved_time(uint8_t idx)
{
    FLASH_SAVE_IDX temp = {0};
    temp.idx = 0xff;
    if (idx < MAX_SAVE_LEN && cur_idx[idx].valid == 1)
    {
        return cur_idx[idx];
    }
    return temp;
}
