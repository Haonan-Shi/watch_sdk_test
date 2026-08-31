/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/logging/log.h>
#include <stdlib.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/display.h>
#include <ctype.h>
#include <stdio.h>

#ifdef CONFIG_ARCH_POSIX
#include <unistd.h>
#else
#include <zephyr/posix/unistd.h>
#endif

LOG_MODULE_REGISTER(app);

#include <trace.h>
#include <zephyr/device.h>

#include "fmc_api.h"
#include "fmc_api_ext.h"
#include "system_status_api.h"
#include "pm.h"
#include "gui_server.h"
#include "app_lower_init.h"


int main(void)
{
    printf("Hello HoneyGUI! I am in task[%s] \n", k_thread_name_get(k_current_get()));
    app_system_lower_init();
    gui_server_init();
    return 0;
}



