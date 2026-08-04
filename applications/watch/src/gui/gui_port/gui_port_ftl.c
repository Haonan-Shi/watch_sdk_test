/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "gui_api_ftl.h"
#include "gui_api_os.h"
#include "gui_port.h"
#include <stdio.h>
#include "fmc_api_ext.h"

int port_ftl_read(uintptr_t addr, uint8_t *buf, size_t len)
{
#if CONFIG_APP_NANDBOOT
    int ret = fmc_flash_nand_read(addr, buf, len);
    if (ret)
    {
        return 0;
    }
    else
    {
        gui_log("port_ftl_read fail! %d", ret);
        return -1;
    }
#else
    gui_log("port_ftl_write not support");
    return -1;
#endif
}

int port_ftl_write(uintptr_t addr, const uint8_t *buf, size_t len)
{
    gui_log("port_ftl_write not support");
    return -1;
}

int port_ftl_erase(uintptr_t addr, size_t len)
{
    gui_log("port_ftl_erase not support");
    return -1;
}

static struct gui_ftl ftl_port =
{
    .read      = port_ftl_read,
    .write     = port_ftl_write,
    .erase     = port_ftl_erase,
};

extern void gui_ftl_info_register(struct gui_ftl *info);

void gui_port_ftl_init(void)
{
    gui_ftl_info_register(&ftl_port);
}
