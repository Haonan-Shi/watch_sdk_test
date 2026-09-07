/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include "usbh_msc_driver.h"
#include "usbh_mgr.h"
#include "ff.h"
#include "trace.h"
#include "app_io_msg.h"
#include "app_usbh_msc.h"

#define APP_USBH_MSC_MSG_DEV_READY  0x01

#define MBR_PARTITION_TYPE_NONE             (0x00)
#define MBR_PARTITION_TYPE_FAT12            (0x01)
#define MBR_PARTITION_TYPE_FAT16            (0x04)
#define MBR_PARTITION_TYPE_exFAT            (0x07)
#define MBR_PARTITION_TYPE_FAT32_CHS        (0x0B)
#define MBR_PARTITION_TYPE_FAT32_LBA        (0x0C)

typedef struct __attribute__((packed))
{
    uint8_t  boot_flag;
    uint8_t  start_chs[3];
    uint8_t  partition_type;
    uint8_t  end_chs[3];
    uint32_t start_lba;
    uint32_t sector_count;
} T_PARTITION_ENTRY;

typedef struct __attribute__((packed))
{
    uint8_t          bootstrap[440];
    uint32_t         disk_signature;
    uint16_t         reserved;
    T_PARTITION_ENTRY   partitions[4];
    uint16_t         boot_signature;
} T_MBR;

static T_MBR *g_mbr;
static FATFS g_fatfs;
static void *test_read_buffer = NULL;

static int app_usbh_msc_mbr_read_complete(T_USBH_MSC_OP_PARAM param, T_USBH_MSC_OP_STATUS status)
{
    T_MBR *mbr = NULL;
    if (status.success)
    {
        USB_PRINT_INFO2("app_usbh_msc_mbr_read_complete 0x%x 0x%x", g_mbr->partitions[0].partition_type,
                        g_mbr->boot_signature);
    }
    return 0;
}

static int app_usbh_mgr_cb(T_USBH_MGR_EVT evt, T_USBH_MGR_EVT_PARAM *param)
{
    if (evt == USBH_MGR_EVT_DEV_INFO_INFORM)
    {
        T_IO_MSG io_msg =
        {
            .type = IO_MSG_TYPE_USBH_MSC,
            .subtype = APP_USBH_MSC_MSG_DEV_READY,
        };
        app_io_msg_send(&io_msg);

    }
    return 0;
}

void app_usbh_msg_handle(T_IO_MSG *msg)
{
    if (msg->type == IO_MSG_TYPE_USBH_MSC)
    {
        if (msg->subtype == USBH_MGR_EVT_DEV_INFO_INFORM)
        {
            if (f_mount(&g_fatfs, L"1:", 0) != FR_OK)
            {
                // MKFS_PARM param = {.fmt = FM_FAT32, .n_fat = 0, .align = 0, .n_root = 0, .au_size = 0};
                // f_mkfs(L"1:", &param, NULL, 0);

            }
            FIL file;
            FRESULT res = f_open(&file, L"1:/test.txt", FA_CREATE_ALWAYS | FA_READ | FA_WRITE);
            if (res == FR_OK)
            {
                const char *str_test = "USB MSC hello world";
                f_write(&file, str_test, strlen(str_test) + 1, NULL);
                f_sync(&file);
            }
            f_rewind(&file);
            USB_PRINT_INFO1("f_open result: %d", res);
            res = f_read(&file, (void *)test_read_buffer, 512, NULL);
            if (res == FR_OK)
            {
                USB_PRINT_INFO1("f_read result: %s", TRACE_STRING(test_read_buffer));
                //DBG_DIRECT("f_read: 0x%x-0x%x-0x%x-0x%x", buf[0], buf[1], buf[2], buf[3]);
            }
            usbh_msc_driver_data_read(0, (uint8_t *)g_mbr, 0, 1, app_usbh_msc_mbr_read_complete, 0xffffffff);
        }
    }
    else
    {
        USB_PRINT_ERROR1("Unknown USB message type: %d", msg->type);
    }
}

void app_usbh_msc_init(void)
{
    T_USBH_MGR_EVT_MSK msk = {.d32 = 0};
    msk.b.dev_info_inform = 1;
    usbh_mgr_cb_register(msk, app_usbh_mgr_cb);
    usbh_msc_driver_init();

    test_read_buffer = malloc(1024);
    g_mbr = (T_MBR *)test_read_buffer;
    memset(test_read_buffer, 0, 1024);
}
