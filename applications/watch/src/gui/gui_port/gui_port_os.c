/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "guidef.h"
#include "gui_port.h"
#include "os_timer.h"
#include <os_msg.h>
#include <os_task.h>
#include <os_sync.h>
#include "platform_utils.h"
#include "trace.h"
#include "stdarg.h"
#include "os_sched.h"
#include "string.h"
#include "trace.h"
#include "wdg.h"
#include "gui_server.h"
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include "psram_section.h"
#include "gui_common.h"
LOG_MODULE_REGISTER(GUI_MODULE, LOG_LEVEL_INF);

#define GUI_FB_SKIP_FRAME   1

extern bool gui_port_dc_lcd_is_work(void);

static bool ble_bond_new_device_flag = false;
static uint32_t gui_task_delay_timer_timeout_ms = 3000;
static void *gui_delay_timer;
static uint32_t fb_skip_count;

void *port_thread_create(const char *name, void (*entry)(void *param), void *param,
                         uint32_t stack_size, uint8_t priority)
{
    void *handle = NULL;
    if (os_task_create(&handle, name, entry, 0, stack_size, 1))
    {
        return handle;
    }
    else
    {
        return NULL;
    }
}
bool port_thread_delete(void *handle)
{
    return os_task_delete(handle);
}

bool port_thread_suspend(void *handle)
{
    return os_task_suspend(handle);
}

bool port_thread_resume(void *handle)
{
    return os_task_resume(handle);
}

bool port_thread_mdelay(uint32_t ms)
{
    // platform_delay_ms(ms);
    os_delay(ms);  //if open, when tab was slid, IDLE stack will overflow
    return true;
}

uint32_t port_thread_ms_get(void)
{
    static uint32_t time_ms_record = 0;
    static uint32_t time_ms_last = 0;
    uint32_t time_ms = sys_timestamp_get(); // max: 0xFFFFFFFF / 1000 ms
    if (time_ms_last > time_ms)
    {
        time_ms_record += 0xFFFFFFFF / 1000; // overflow
    }
    time_ms_last = time_ms;
    return time_ms_record + time_ms;
}

uint32_t port_thread_us_get(void)
{
    /*sys_timestamp_get_us() and sys_timestamp_get() should be called carefully*/
    /*overflow will occur after 1.193 hour*/

    return sys_timestamp_get_us();

}

bool port_thread_ctx_malloc(uint32_t size)
{
    return true;
}

#include "stdlib.h"
#include "stdio.h"
#include "gui_api.h"
#include "trace.h"

static void port_log(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    char buf[256];
    vsnprintf(buf, sizeof(buf), format, args);

    //APP_PRINT_INFO1("[GUI MODULE]%s", TRACE_STRING(buf));
    LOG_INF("%s", buf);

    va_end(args);
}

static bool port_mq_create(void *handle, const char *name, uint32_t msg_size, uint32_t max_msgs)
{
    return os_msg_queue_create(handle, name, max_msgs, msg_size);
}

static bool port_mq_send(void *handle, void *buffer, uint32_t size, uint32_t timeout)
{
    return os_msg_send(handle, buffer, timeout);
}

static bool port_mq_recv(void *handle, void *buffer, uint32_t size, uint32_t timeout)
{
    return os_msg_recv(handle, buffer, timeout);
}

static void port_gui_sleep(void)
{
    gui_display_off();
}

#define GUI_HEAP_SIZE                                           (30 * 1024)

__attribute__((aligned(4))) static uint8_t port_mem_heap[GUI_HEAP_SIZE] = {0};

/* GUI lower heap lives in the PSRAM1_MCU region. Defined as a variable in the
 * PSRAM1_MCU linker section (NOLOAD) instead of referencing the raw dts
 * address, so the region shows up as used in the build memory report and the
 * placement is validated at link time. The section is not zeroed/copied at
 * boot; the GUI allocator initialises the pool at runtime, which only runs
 * after psram is powered up. Sized to the whole psram1_for_mcu region. */
static uint8_t gui_lower_heap[DT_REG_SIZE(DT_NODELABEL(psram1_for_mcu))] SECTION_PSRAM1_MCU;

static struct gui_os_api os_api =
{
    .name = "rtk_osif",
    .thread_create = port_thread_create,
    .thread_delete = port_thread_delete,
    .thread_suspend = port_thread_suspend,
    .thread_resume = port_thread_resume,
    .thread_mdelay = port_thread_mdelay,
    .thread_ms_get = port_thread_ms_get,
    .thread_us_get = port_thread_us_get,
    .mq_create = port_mq_create,
    .mq_send = port_mq_send,
    .mq_recv = port_mq_recv,
    .f_malloc = NULL,
    .f_free = NULL,
    .f_realloc = NULL,
    .gui_sleep_cb = port_gui_sleep,

    /*mem_size and lower_mem_size should be large than tlsf_size 3188 byte*/
    .mem_addr = (void *)port_mem_heap,
    .mem_size = GUI_HEAP_SIZE,

#include "address_map.h"
    .lower_mem_addr = (void *)gui_lower_heap,
    .lower_mem_size = sizeof(gui_lower_heap),

    .mem_threshold_size = 10 * 1024,

    /*enable this if use printf*/
    // .log = (log_func_t)printf,
    .log = port_log,
};

static void port_gui_task_delay_enable(uint32_t delay_ms)
{
    APP_PRINT_TRACE0("ble_bond_new_device_flag enable!!");
    ble_bond_new_device_flag = true;
    os_timer_restart(&gui_delay_timer, delay_ms);
}
static void port_gui_task_delay_disable(void)
{
    ble_bond_new_device_flag = false;
}

static void gui_delay_timer_callback(void *pxTimer)
{
    APP_PRINT_TRACE0("gui fb skip stop timeout!");
    port_gui_task_delay_disable();
}

void gui_port_os_add_exe_to_gui_task()
{
    // APP_PRINT_INFO0("feed watchdog");
    wdg_kick();

    if (ble_bond_new_device_flag)
    {
        fb_skip_count++;
        if (fb_skip_count % (GUI_FB_SKIP_FRAME + 1))
        {
            APP_PRINT_TRACE0("gui fb skip !!");
            os_delay(20);
        }
    }
}

void gui_port_os_init(void)
{
    gui_os_api_register(&os_api);
    gui_task_ext_execution_sethook(gui_port_os_add_exe_to_gui_task);

    gui_task_delay_set_callbacks(port_gui_task_delay_enable, port_gui_task_delay_disable);
    os_timer_create(&gui_delay_timer, "gui_delay_timer", 1, gui_task_delay_timer_timeout_ms,
                    false, gui_delay_timer_callback);

}

