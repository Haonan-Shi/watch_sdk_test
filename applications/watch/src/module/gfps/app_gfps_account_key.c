/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if CONFIG_REALTEK_GFPS_FEATURE_SUPPORT
#include "trace.h"
#include "remote.h"
#include "gfps.h"
#include "ftl.h"
#include "stdlib.h"
#include "app_gfps_account_key.h"
#include "app_main.h"
#include "app_gfps_cfg.h"
#include "rtk_errno.h"
#include <string.h>
#include "bt_gfps.h"
static T_ACCOUNT_KEY *account_key          = NULL;
static uint8_t gfps_account_key_num        = 5;  //sum of account key
static uint8_t gfps_account_key_table_size = 0;
/*Fast pair initialize*/
bool app_gfps_account_key_init(uint8_t key_num)
{
    if (key_num > GFPS_ACCOUNT_KEY_MAX)
    {
        key_num = GFPS_ACCOUNT_KEY_MAX;
    }
    gfps_account_key_num        = key_num;
    gfps_account_key_table_size = 4 + gfps_account_key_num * (GFPS_ACCOUNT_KEY_LENGTH +
                                                              GFPS_BD_ADDR_LENGTH);
    account_key = calloc(1, gfps_account_key_table_size);
    if (account_key)
    {
        uint32_t read_result = ftl_load_from_storage(account_key, ACCOUNT_KEY_FLASH_OFFSET,
                                                     gfps_account_key_table_size);
        if (read_result == ENOF)
        {
            memset(account_key, 0, gfps_account_key_table_size);

#if CONFIG_REALTEK_GFPS_FINDER_SUPPORT
            /*the Owner Account Key is defined as the first Account Key introduced to the Provider
            The Owner Account Key must not be removed when the Provider runs out of free Account Key slots.*/
            if (app_gfps_cfg.gfps_finder_support)
            {
                account_key->del = 1;
            }
#endif

            ftl_save_to_storage(account_key, ACCOUNT_KEY_FLASH_OFFSET, gfps_account_key_table_size);
        }
        else
        {
            for (uint8_t i = 0; i < account_key->num; i++)
            {
                APP_PRINT_INFO4("app_gfps_account_key_init: idx %d, key %b, bd_addr %b, del %d", i,
                                TRACE_BINARY(16, account_key->account_info[i].key),
                                TRACE_BDADDR(account_key->account_info[i].addr), account_key->del);
            }
        }
    }
    else
    {
        APP_PRINT_ERROR0("app_gfps_account_key_init: alloc account key memory fail");
        return false;
    }

    gfps_account_key_init(account_key, gfps_account_key_num);
    return true;
}

uint8_t app_gfps_account_key_get_table_size(void)
{
    return gfps_account_key_table_size;
}

T_ACCOUNT_KEY *app_gfps_account_key_get_table(void)
{
    return account_key;
}

/*Account key store */
bool app_gfps_account_key_store(uint8_t key[16], uint8_t *bd_addr)
{
    for (uint8_t i = 0; i < account_key->num; i++)
    {
        if (memcmp(bd_addr, account_key->account_info[i].addr, 6) == 0)
        {
            if (memcmp(key, account_key->account_info[i].key, 16) == 0)
            {
                /*bd_addr and key has already in account key table*/
                APP_PRINT_INFO1("app_gfps_account_key_store: key exist i %d", i);
                return false;
            }
            else
            {
                /*bd_addr has already stored in flash, just update key,
                 if finder support and owner key is valid, shall not update ownerkey(i==0)*/
                APP_PRINT_INFO3("app_gfps_account_key_store: bd_addr %b already exist, key update index i %d, key %b",
                                TRACE_BDADDR(bd_addr), i, TRACE_BINARY(16, account_key->account_info[i].key));
#if CONFIG_REALTEK_GFPS_FINDER_SUPPORT
                if (app_gfps_cfg.gfps_finder_support)
                {
                    bool owner_key_valid = gfps_get_owner_key_valid();

                    if ((i != 0) || (!owner_key_valid))
                    {
                        memcpy(account_key->account_info[i].key, key, 16);
                        ftl_save_to_storage(account_key, ACCOUNT_KEY_FLASH_OFFSET, gfps_account_key_table_size);
                        return true;
                    }
                    else
                    {
                        // if i==0 and owner_key_valid==1, shall not update owner key,
                        // the new key and bd addr shall stored in another location.
                    }
                }
                else
#endif
                {
                    memcpy(account_key->account_info[i].key, key, 16);
                    ftl_save_to_storage(account_key, ACCOUNT_KEY_FLASH_OFFSET, gfps_account_key_table_size);
                    return true;
                }
            }
        }
    }

    if (account_key->num < gfps_account_key_num)
    {
        memcpy(account_key->account_info[account_key->num].key, key, 16);
        memcpy(account_key->account_info[account_key->num].addr, bd_addr, 6);
        account_key->num++;
    }
    else if (account_key->num == gfps_account_key_num)
    {
        memcpy(account_key->account_info[account_key->del].key, key, 16);
        memcpy(account_key->account_info[account_key->del].addr, bd_addr, 6);
        account_key->del++;
        if (account_key->del == gfps_account_key_num)
        {
#if CONFIG_REALTEK_GFPS_FINDER_SUPPORT
            /*the Owner Account Key is defined as the first Account Key introduced to the Provider
            The Owner Account Key must not be removed when the Provider runs out of free Account Key slots.*/
            if (app_gfps_cfg.gfps_finder_support)
            {
                account_key->del = 1;
            }
            else
#endif
            {
                account_key->del = 0;
            }
        }
    }

    ftl_save_to_storage(account_key, ACCOUNT_KEY_FLASH_OFFSET, gfps_account_key_table_size);
    return true;
}

/*Clear Account key store */
void app_gfps_account_key_clear(void)
{
    APP_PRINT_WARN0("app_gfps_account_key_clear");
    memset(account_key, 0, gfps_account_key_table_size);

#if CONFIG_REALTEK_GFPS_FINDER_SUPPORT
    /*the Owner Account Key is defined as the first Account Key introduced to the Provider
    The Owner Account Key must not be removed when the Provider runs out of free Account Key slots.*/
    if (app_gfps_cfg.gfps_finder_support)
    {
        account_key->del = 1;
    }
#endif

    ftl_save_to_storage(account_key, ACCOUNT_KEY_FLASH_OFFSET, gfps_account_key_table_size);
}

/**
 * @brief print all account key info
 */
void app_gfps_account_key_table_print(void)
{
    for (uint8_t i = 0; i < account_key->num;  i++)
    {
        APP_PRINT_INFO3("app_gfps_account_key_table_print: idx %d, key %b, bd_addr %b", i,
                        TRACE_BINARY(16, account_key->account_info[i].key),
                        TRACE_BDADDR(account_key->account_info[i].addr));
    }
}

#if CONFIG_REALTEK_GFPS_FINDER_SUPPORT
bool app_gfps_account_key_update_owner_key(uint8_t key[16], T_ACCOUNT_KEY *p_table)
{
    memcpy(p_table->account_info[0].key, key, 16);
    if (p_table->num == 0)
    {
        p_table->num++;
    }
    return true;
}
#endif
bool app_gfps_account_key_add(uint8_t key[16], uint8_t *bd_addr, T_ACCOUNT_KEY *p_table)
{
    for (uint8_t i = 0; i < p_table->num; i++)
    {
        if (memcmp(bd_addr, p_table->account_info[i].addr, 6) == 0)
        {
            if (memcmp(key, p_table->account_info[i].key, 16) == 0)
            {
                /*bd_addr and key has already in p_table*/
                return false;
            }
            else
            {
                /*bd_addr has already in p_table, just update key
                 if finder support, shall not update owner key*/
                APP_PRINT_INFO1("app_gfps_account_key_add: key update i %d", i);

#if CONFIG_REALTEK_GFPS_FINDER_SUPPORT
                if (app_gfps_cfg.gfps_finder_support)
                {
                    if (i != 0)
                    {
                        memcpy(p_table->account_info[i].key, key, 16);
                        return true;
                    }
                }
                else
#endif
                {
                    memcpy(p_table->account_info[i].key, key, 16);
                    return true;
                }
            }
        }
    }

    if (p_table->num < gfps_account_key_num)
    {
        memcpy(p_table->account_info[p_table->num].key, key, 16);
        memcpy(p_table->account_info[p_table->num].addr, bd_addr, 6);
        p_table->num++;
    }
    else if (p_table->num == gfps_account_key_num)
    {
        memcpy(p_table->account_info[p_table->del].key, key, 16);
        memcpy(p_table->account_info[p_table->del].addr, bd_addr, 6);
        p_table->del++;
        if (p_table->del == gfps_account_key_num)
        {
#if CONFIG_REALTEK_GFPS_FINDER_SUPPORT
            /*the Owner Account Key is defined as the first Account Key introduced to the Provider
            The Owner Account Key must not be removed when the Provider runs out of free Account Key slots.*/
            if (app_gfps_cfg.gfps_finder_support)
            {
                p_table->del = 1;
            }
            else
#endif
            {
                p_table->del = 0;
            }
        }
    }

    return true;
}

#if CONFIG_REALTEK_GFPS_FINDER_SUPPORT
void app_gfps_account_key_save_ownerkey_valid(void)
{
    APP_PRINT_INFO1("app_gfps_account_key_save_ownerkey_valid: valid %d", account_key->owner_key_valid);
    ftl_save_to_storage(account_key, ACCOUNT_KEY_FLASH_OFFSET, gfps_account_key_table_size);
}
#endif
#endif
