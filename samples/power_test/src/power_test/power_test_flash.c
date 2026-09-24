/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "rtl876x.h"
#include "trace.h"
#include "board.h"
#include "os_mem.h"
#include "app_msg.h"
#include "app_console_msg.h"
#include "power_test.h"
#include "power_test_flash.h"
#include "pm.h"
#include <zephyr/shell/shell.h>
#include "flash_map.h"
#include "fmc_api.h"
#include "wdg.h"
#include "section.h"
#include "cache.h"

#include "os_sync.h"

#ifdef USER_DATA1_ADDR
#define TEST_ADDR (USER_DATA1_ADDR)
#define TEST_SIZE (USER_DATA1_SIZE)
#else
#define TEST_ADDR (OTA_BANK1_ADDR)
#define TEST_SIZE (OTA_BANK1_SIZE)
#endif

#define TEST_READ_LEN (256)
#define TEST_WRITE_LEN (1024)
#define TEST_NUM    (200)

extern void power_test_send_msg(T_IO_CONSOLE subtype, void *param_buf);

int cmd_flash(const struct shell *sh, size_t argc, char **argv)
{
    uint16_t    action;
    void       *param_buf;

    if (!strcmp(argv[1], "write"))
    {
        action = POWER_TEST_FLASH_WRITE;
    }
    else if (!strcmp(argv[1], "read"))
    {
        action = POWER_TEST_FLASH_READ;
    }
    else if (!strcmp(argv[1], "erase"))
    {
        action = POWER_TEST_FLASH_ERASE;
    }
    else if (!strcmp(argv[1], "xip"))
    {
        action = POWER_TEST_FLASH_XIP;
    }
    else if (!strcmp(argv[1], "cache"))
    {
        action = POWER_TEST_FLASH_CACHE;
    }
    else if (!strcmp(argv[1], "half_cache"))
    {
        action = POWER_TEST_FLASH_HALF_CACHE;
    }
    else if (!strcmp(argv[1], "dma_read"))
    {
        action = POWER_TEST_FLASH_DMA_READ;
    }
    else if (!strcmp(argv[1], "write_prepare"))
    {
        action = POWER_TEST_FLASH_WRITE_PRE;
    }
    else if (!strcmp(argv[1], "erase_prepare"))
    {
        action = POWER_TEST_FLASH_ERASE_PRE;
    }
    else
    {
        goto err;
    }

    param_buf = malloc(3);

    if (param_buf != NULL)
    {
        uint8_t *p;

        p = param_buf;

        LE_UINT16_TO_STREAM(p, PLATFORM_ID);
        LE_UINT16_TO_STREAM(p, action);

        power_test_send_msg(IO_MSG_CONSOLE_STRING_RX, param_buf);
    }

    shell_print(sh, "Power test flash %s.", argv[1]);
    return 0;

err:
    shell_error(sh, "Invalid param %s (write read erase xip cache half_cache).", argv[1]);
    return -EINVAL;
}

RAM_TEXT_SECTION
void power_test_flash_write(void)
{
    uint32_t test_write_addr = TEST_ADDR;

    uint8_t test_write_data[TEST_WRITE_LEN];
    uint8_t write_data = 0x7F;
    memset(test_write_data, write_data, TEST_WRITE_LEN);
    uint8_t i = 6;

    while (true)
    {
        fmc_flash_nor_write(test_write_addr, test_write_data, TEST_READ_LEN);
        test_write_addr += TEST_READ_LEN;
        if (test_write_addr == (TEST_ADDR + TEST_SIZE))
        {
            test_write_addr = TEST_ADDR;
            if (i > 0)
            {
                i--;
                write_data &= (~BIT(i));
                memset(test_write_data, write_data, TEST_READ_LEN);
            }
            else
            {
                break;
            }
        }
    }
}

RAM_TEXT_SECTION
void power_test_flash_read(void)
{
    uint32_t test_read_addr = TEST_ADDR;
    uint8_t test_read_data[TEST_READ_LEN];

    while (true)
    {
        fmc_flash_nor_read(test_read_addr, &test_read_data, TEST_READ_LEN);
        test_read_addr += TEST_READ_LEN;

        if (test_read_addr >= (TEST_ADDR + TEST_SIZE))
        {
            test_read_addr = TEST_ADDR;
        }
    }
}

RAM_TEXT_SECTION
void power_test_flash_erase(void)
{
    uint32_t test_erase_addr = TEST_ADDR;

    while (true)
    {
        fmc_flash_nor_erase(test_erase_addr, FMC_FLASH_NOR_ERASE_SECTOR);
        test_erase_addr += 0x1000;
        if (test_erase_addr == (TEST_ADDR + TEST_SIZE))
        {
            test_erase_addr = TEST_ADDR;
        }
    }

}

RAM_TEXT_SECTION
void power_test_flash_xip(void)
{
    uint32_t test_xip_addr = TEST_ADDR;

    for (uint32_t erase_addr = test_xip_addr; erase_addr < TEST_ADDR + TEST_SIZE; erase_addr += 0x1000)
    {
        fmc_flash_nor_erase(erase_addr, FMC_FLASH_NOR_ERASE_SECTOR);
    }

    cache_disable();

    while (true)
    {
        if (*(uint32_t *)test_xip_addr != 0xFFFFFFFF)
        {
            APP_PRINT_INFO1("read data err.data:0x%x", *(uint32_t *)test_xip_addr);
        }
        test_xip_addr += 4;
        if (test_xip_addr == (TEST_ADDR + TEST_SIZE))
        {
            test_xip_addr = TEST_ADDR;
        }
    }
}

#ifdef CONFIG_SOC_SERIES_RTL87X3G
void ldr_cache(uint32_t tar_addr)
{
    /* Totally 100 ldr inst */
    __asm volatile
    (
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   ldr r1, [r0]                               \n"
        "   bx  lr                                     \n"
    );
}

#else
__asm volatile void ldr_cache(uint32_t tar_addr)
{
    /* Totally 100 ldr inst */
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar
    ldr     r1, [r0]     //cppcheck-suppress uninitvar

    bx      lr
}
#endif

void power_test_flash_cache(void)
{
    uint32_t test_cache_addr = TEST_ADDR;

    cache_hit_init(true);

    while (true)
    {
        ldr_cache(test_cache_addr);
    }
}

#define CYCLE_NUM (100)

RAM_TEXT_SECTION
void power_test_flash_half_cache(void)
{
    uint32_t test_cache_addr = TEST_ADDR;

    uint32_t num = CYCLE_NUM;
    cache_hit_init(true);
    while (true)
    {
        uint32_t s = os_lock();
        cache_disable();
        while (num)
        {
            ldr_cache(test_cache_addr);
            num--;
        }
        num = CYCLE_NUM;

        cache_enable();
        while (num)
        {
            ldr_cache(test_cache_addr);
            num --;
        }
        num = CYCLE_NUM;

        os_unlock(s);
    }
}

volatile uint32_t test_read_dma_addr = TEST_ADDR;
uint32_t test_dma_read_buf[TEST_READ_LEN];
RAM_TEXT_SECTION
void flash_nor_auto_dma_cb(void)
{
    test_read_dma_addr += TEST_READ_LEN;
    if (test_read_dma_addr >= (TEST_ADDR + TEST_SIZE))
    {
        test_read_dma_addr = TEST_ADDR;
    }

    fmc_flash_nor_auto_dma_read(test_read_dma_addr, (uint32_t)test_dma_read_buf, TEST_READ_LEN,
                                flash_nor_auto_dma_cb);
}

RAM_TEXT_SECTION
void power_test_flash_dma_read(void)
{
    for (uint32_t erase_addr = test_read_dma_addr; erase_addr < TEST_ADDR + TEST_SIZE;
         erase_addr += 0x1000)
    {
        fmc_flash_nor_erase(erase_addr, FMC_FLASH_NOR_ERASE_SECTOR);
    }

    fmc_flash_nor_auto_dma_read(test_read_dma_addr, (uint32_t)test_dma_read_buf, TEST_READ_LEN,
                                flash_nor_auto_dma_cb);

    while (1);
}

void power_test_flash_write_prepare(void)
{
    for (uint32_t test_addr = TEST_ADDR; test_addr < (TEST_ADDR + TEST_SIZE); test_addr += 0x1000)
    {
        fmc_flash_nor_erase(test_addr, FMC_FLASH_NOR_ERASE_SECTOR);
    }

    printk("Environment is ready\n");

}

void power_test_flash_erase_prepare(void)
{

    uint8_t *buf = malloc(TEST_WRITE_LEN);
    memset(buf, 0x5A, TEST_WRITE_LEN);
    for (uint32_t test_addr = TEST_ADDR; test_addr < (TEST_ADDR + TEST_SIZE);
         test_addr += TEST_WRITE_LEN)
    {
        fmc_flash_nor_write(test_addr, buf, TEST_WRITE_LEN);
    }

    free(buf);
    printk("Environment is ready\n");
}

void power_test_flash_action(T_POWER_TEST_FLASH_CMD action, uint8_t *buf)
{
    WDG_Disable();
    switch (action)
    {
    case POWER_TEST_FLASH_WRITE:
        power_test_flash_write();
        break;
    case POWER_TEST_FLASH_READ:
        power_test_flash_read();
        break;
    case POWER_TEST_FLASH_ERASE:
        power_test_flash_erase();
        break;
    case POWER_TEST_FLASH_XIP:
        power_test_flash_xip();
        break;
    case POWER_TEST_FLASH_CACHE:
        power_test_flash_cache();
        break;
    case POWER_TEST_FLASH_HALF_CACHE:
        power_test_flash_half_cache();
        break;
    case POWER_TEST_FLASH_DMA_READ:
        power_test_flash_dma_read();
        break;
    case POWER_TEST_FLASH_WRITE_PRE:
        power_test_flash_write_prepare();
        break;
    case POWER_TEST_FLASH_ERASE_PRE:
        power_test_flash_erase_prepare();
        break;
    default:
        break;
    }

    if (buf != NULL)
    {
        free(buf);
    }
}

