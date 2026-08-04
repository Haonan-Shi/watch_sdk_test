/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */


#include "sd_if.h"
#include "usb_if.h"
#include "module_adaptor.h"

#ifdef TARGET_RTL87X3EU
#define USB_SDHC_ID         SDHC_ID1
#else
#define USB_SDHC_ID         SDHC_ID0
#endif

static int usb_read_sd(uint32_t sector, uint32_t buf, uint16_t blk_size, uint16_t blk_num)
{
    return sd_if_read(USB_SDHC_ID, sector, buf, blk_size, blk_num);
}
static int usb_write_sd(uint32_t sector, uint32_t buf, uint16_t blk_size, uint16_t blk_num)
{
//    return 0;
    return sd_if_write(USB_SDHC_ID, sector, buf, blk_size, blk_num);
}
static uint32_t usb_sd_get_dev_block_num(void)
{
    return sd_if_get_dev_block_num(USB_SDHC_ID);
}
static const T_MSC_SD_CB sd_cb =
{
    .app_usb_read_sd = usb_read_sd,
    .app_usb_write_sd = usb_write_sd,
    .usb_get_sd_block_num = usb_sd_get_dev_block_num,
};
void usb_app_init(void)
{
    uint16_t product_string[] = L"RTL8763EW Watch";
    usb_set_string_in_descriptor(USB_PRODUCT_STRING, product_string, sizeof(product_string));
    usb_if_register_sd_cb(&sd_cb);
    usb_adp_interrupt_init();
}
