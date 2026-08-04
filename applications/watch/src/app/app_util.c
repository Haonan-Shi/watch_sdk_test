/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <zephyr/kernel.h>
#include "trace.h"
#include "app_util.h"

uint8_t app_util_calc_checksum(uint8_t *dataPtr, uint16_t len)
{
    uint8_t check_sum;

    check_sum = 0;
    while (len)
    {
        check_sum += *dataPtr;
        dataPtr++;
        len--;
    }
    return (uint8_t)(0xff - check_sum + 1); //((~check_sum)+1);
}

void app_util_print_all_tasks_info(void)
{
    struct k_thread *thread;
    const char *task_name_str = "Task Name";
    const char *priority_str = "Priority";
    const char *stack_size_str = "Stack Size";
    const char *state_str = "State";

    APP_PRINT_TRACE4("\n%-20s %-10s %-10s %-10s\n",
                     TRACE_STRING(task_name_str), TRACE_STRING(priority_str), TRACE_STRING(stack_size_str),
                     TRACE_STRING(state_str));
    APP_PRINT_TRACE0("--------------------------------------------\n");

    for (thread = _kernel.threads; thread; thread = thread->next_thread)
    {
        APP_PRINT_TRACE4("%-20s %-10d %-10zu %-10d\n",
                         TRACE_STRING(k_thread_name_get(thread)),
                         thread->base.prio,
                         thread->stack_info.size,
                         thread->base.thread_state);
    }
}
