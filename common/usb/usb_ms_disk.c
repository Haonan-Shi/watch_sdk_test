/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if F_APP_USB_MSC_SUPPORT

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#if defined(CONFIG_ZEPHYR_REALTEK_APP_MODULE)
#include <zephyr/storage/disk_access.h>
#include "usb_ms_driver.h"
#include "os_mem.h"
#include <string.h>
#include "rtk_errno.h"
#include "trace.h"

#define DISK_DRIVE_NAME  CONFIG_MMC_VOLUME_NAME //CONFIG_SDMMC_VOLUME_NAME
#else
#include "errno.h"
#include "usb_ms_driver.h"
#include "sd.h"
#include "os_mem.h"
#include <string.h>
#include "trace.h"

#ifdef CONFIG_SOC_SERIES_RTL87X3G
#define USB_MS_DISK_SDHC_ID         SDHC_ID0
#else
#define USB_MS_DISK_SDHC_ID         SDHC_ID0
#endif

#endif

#define USB_MS_DISK_BLK_SIZE        (0x200)

static int usb_ms_disk_format(void)
{
    return ESUCCESS;
}

void *usb_ms_disk_buffer_alloc(uint32_t blk_num)
{
#if defined(CONFIG_ZEPHYR_REALTEK_APP_MODULE)
    uint32_t blk_len;

    if (disk_access_ioctl(DISK_DRIVE_NAME, DISK_IOCTL_GET_ERASE_BLOCK_SZ, &blk_len) != 0) { return NULL; }

    return os_mem_zalloc(OS_MEM_TYPE_DATA, blk_num * blk_len);
#else
#if CONFIG_SOC_SERIES_RTL8763E
    return os_mem_zalloc(OS_MEM_TYPE_BUFFER, blk_num * USB_MS_DISK_BLK_SIZE);
#else
    return os_mem_zalloc(OS_MEM_TYPE_DATA, blk_num * USB_MS_DISK_BLK_SIZE);
#endif
#endif
}

int usb_ms_disk_buffer_free(void *buf)
{
    os_mem_free(buf);

    return ESUCCESS;
}

static int usb_ms_disk_read(uint32_t lba, uint32_t blk_num, uint8_t *data)
{
    int ret = ESUCCESS;

#if defined(CONFIG_ZEPHYR_REALTEK_APP_MODULE)
    if (disk_access_read(DISK_DRIVE_NAME, data, lba, blk_num) != 0)
    {
        ret = -EIO;
    }

#else
    if (sd_read(USB_MS_DISK_SDHC_ID, lba, (uint32_t)data, USB_MS_DISK_BLK_SIZE, blk_num) != 0)
    {
        ret = -EIO;
    }
#endif

    return ret;
}

static int usb_ms_disk_write(uint32_t lba, uint32_t blk_num, uint8_t *data)
{
    int ret = ESUCCESS;

#if defined(CONFIG_ZEPHYR_REALTEK_APP_MODULE)
    if (disk_access_write(DISK_DRIVE_NAME, data, lba, blk_num) != 0)
    {
        ret = -EIO;
    }
#else
    if (sd_write(USB_MS_DISK_SDHC_ID, lba, (uint32_t)data, USB_MS_DISK_BLK_SIZE, blk_num) != 0)
    {
        ret = -EIO;
    }
#endif

    return ret;
}

static bool usb_ms_disk_is_ready(void)
{
    return true;
}

static int usb_ms_disk_capacity_get(uint32_t *max_lba, uint32_t *blk_len)
{

#if defined(CONFIG_ZEPHYR_REALTEK_APP_MODULE)
    if (disk_access_ioctl(DISK_DRIVE_NAME, DISK_IOCTL_GET_SECTOR_COUNT, max_lba) != 0) { return -EIO; }
    if (disk_access_ioctl(DISK_DRIVE_NAME, DISK_IOCTL_GET_ERASE_BLOCK_SZ, blk_len) != 0) { return -EIO; }
    *max_lba = (*max_lba) - 1;
#else
//    *max_lba =  sd_if_get_dev_block_num(USB_MS_DISK_SDHC_ID);
    *blk_len = USB_MS_DISK_BLK_SIZE;
    *max_lba =  sd_get_dev_capacity(USB_MS_DISK_SDHC_ID) / (*blk_len);
#endif

    return ESUCCESS;
}

static T_DISK_DRIVER usb_ms_disk_driver =
{
    .type = 0,
    .blk_size = USB_MS_DISK_BLK_SIZE,
#if CONFIG_SOC_SERIES_RTL8763E
    .max_blk_per_access = 1,
#else
    .max_blk_per_access = 32,
#endif
    .format = usb_ms_disk_format,
    .read = usb_ms_disk_read,
    .write = usb_ms_disk_write,
    .is_ready = usb_ms_disk_is_ready,
    .remove = NULL,
    .capacity_get = usb_ms_disk_capacity_get,
    .buffer_alloc = usb_ms_disk_buffer_alloc,
    .buffer_free = usb_ms_disk_buffer_free,
};

int usb_ms_disk_init(void)
{
#if defined(CONFIG_ZEPHYR_REALTEK_APP_MODULE)
    if (disk_access_ioctl(DISK_DRIVE_NAME, DISK_IOCTL_GET_ERASE_BLOCK_SZ,
                          &usb_ms_disk_driver.blk_size) != 0) { return -EIO; }
#endif

    usb_ms_driver_disk_register((T_DISK_DRIVER *)&usb_ms_disk_driver);
    return 0;
}

int usb_ms_disk_deinit(void)
{
    usb_ms_driver_disk_unregister((T_DISK_DRIVER *)&usb_ms_disk_driver);
    // usb_dm_cb_unregister(usb_ms_disk_dm_cb);
    return 0;
}
#endif
