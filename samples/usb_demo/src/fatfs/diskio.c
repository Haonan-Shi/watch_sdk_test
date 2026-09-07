/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2013        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control module to the FatFs module with a defined API.        */
/*-----------------------------------------------------------------------*/

#include "diskio.h"     /* FatFs lower layer API */
#include "usbh_msc_driver.h"
#include "os_sync.h"
#include "trace.h"


T_USB_DISK_CAPACITY g_disk_capacity = {0};

DSTATUS disk_initialize(
    BYTE pdrv               /* Physical drive nmuber (0..) */
)
{
    DSTATUS stat = RES_OK;
    g_disk_capacity = usbh_msc_driver_capacity_get(0);

    return stat;
}

DSTATUS disk_status(
    BYTE pdrv       /* Physical drive nmuber (0..) */
)
{
    return 0;
}

static int app_usbh_msc_data_read_complete(T_USBH_MSC_OP_PARAM param, T_USBH_MSC_OP_STATUS status)
{
    USB_PRINT_INFO4("app_usbh_msc_data_read_complete success:%d, skey:0x%x, asc:0x%x %b",
                    status.success,
                    status.skey, status.asc, TRACE_BINARY(param.rw.blk_actual, param.rw.buf));
    return 0;
}

static int app_usbh_msc_data_write_complete(T_USBH_MSC_OP_PARAM param, T_USBH_MSC_OP_STATUS status)
{
    USB_PRINT_INFO3("app_usbh_msc_data_write_complete success:%d, skey:0x%x, asc:0x%x", status.success,
                    status.skey, status.asc);
    return 0;
}

DRESULT disk_read(
    BYTE pdrv,      /* Physical drive nmuber (0..) */
    BYTE *buff,     /* Data buffer to store read data */
    DWORD sector,   /* Sector address (LBA) */
    UINT count      /* Number of sectors to read (1..128) */
)
{
    DRESULT res = RES_OK;
    USB_PRINT_INFO2("disk_read, pdrv:%d, sector %d", pdrv, sector);
    usbh_msc_driver_data_read(0, buff, sector, count, app_usbh_msc_data_read_complete, 0xffffffff);

    return res;
}

DRESULT disk_write(
    BYTE pdrv,          /* Physical drive nmuber (0..) */
    const BYTE *buff,   /* Data to be written */
    DWORD sector,       /* Sector address (LBA) */
    UINT count          /* Number of sectors to write (1..128) */
)
{
    USB_PRINT_INFO1("disk_write, pdrv:%d", pdrv);
    DRESULT res = RES_OK;;

    usbh_msc_driver_data_write(0, (uint8_t *)buff, sector, count, app_usbh_msc_data_write_complete,
                               0xffffffff);

    return res;

}



DRESULT disk_ioctl(
    BYTE pdrv,      /* Physical drive nmuber (0..) */
    BYTE cmd,       /* Control code */
    void *buff      /* Buffer to send/receive control data */
)
{
    DRESULT res = RES_OK;

    switch (cmd)
    {
    case GET_SECTOR_SIZE:
        *(DWORD *)buff = g_disk_capacity.blk_len;
        break;
    case GET_BLOCK_SIZE:
        *(DWORD *)buff = g_disk_capacity.blk_len;
        break;
    case GET_SECTOR_COUNT:
        *(DWORD *)buff = g_disk_capacity.lba;
        break;
    default:
        res = RES_PARERR;  /* Invalid command */
        break;
    }
    return res;
}

DWORD get_fattime(void)
{
    return 0;
}

