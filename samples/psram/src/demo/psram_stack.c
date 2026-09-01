/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*
 * Demo 3: psram used as a thread stack. Two variants, both creating the task at
 * runtime:
 *   [3.1] dynamic task + dynamic stack - psram_task_create() allocates the task
 *         control block AND the thread stack from a psram heap.
 *   [3.2] dynamic task + static stack  - the struct k_thread is allocated from a
 *         psram heap, the stack is a build-time array placed in the PSRAM0_MCU
 *         memory-region (Z_KERNEL_STACK_DEFINE_IN).
 *
 * A psram thread stack must be set up at runtime, after psram is powered up in
 * app_system_lower_init(). A compile-time thread (K_THREAD_DEFINE) cannot use
 * one: the kernel writes a static thread's initial stack frame during boot
 * (z_init_static_threads()), before psram is available.
 *
 * This file also implements the osif-style psram_task_create()/
 * psram_task_delete() used by [3.1]. They mirror os_task_create_zephyr()/
 * os_task_delete_zephyr() in osif_zephyr.c, except that both the stack and the
 * task control block come from a psram heap instead of RAM_TYPE_DATA_ON. The
 * stack is 8-byte aligned like the osif path; HW stack protection on this
 * Cortex-M55 uses PSPLIM, so that alignment is sufficient.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <zephyr/kernel.h>

#include "mem_types.h"
#include "os_mem.h"
#include "osif_zephyr.h"
#include "psram_section.h"
#include "psram_demo.h"

#define PSRAM_STACK_SIZE     2048
#define WORKER_LOCAL_LEN     512
#define WORKER_EXPECT        65280u  /* deterministic result of psram_stack_worker */

/* static psram stack (build-time, PSRAM0_MCU) used by demo 3.2 */
Z_KERNEL_STACK_DEFINE_IN(task_static_stack, PSRAM_STACK_SIZE, SECTION_PSRAM0_MCU);

static volatile uint32_t worker_result;
static K_SEM_DEFINE(worker_sem, 0, 1);

/* thread entry: writes and sums a local array on its (psram) stack, records the
 * result, and signals completion */
static void psram_stack_worker(void *a, void *b, void *c)
{
    volatile uint8_t local[WORKER_LOCAL_LEN];
    uint32_t sum = 0;

    ARG_UNUSED(a);
    ARG_UNUSED(b);
    ARG_UNUSED(c);

    for (size_t i = 0; i < WORKER_LOCAL_LEN; i++)
    {
        local[i] = (uint8_t)(i & 0xFF);
    }
    for (size_t i = 0; i < WORKER_LOCAL_LEN; i++)
    {
        sum += local[i];
    }
    worker_result = sum;
    printf("        worker ran, local stack @ %p sum=%u\n", (void *)&local[0], sum);
    k_sem_give(&worker_sem);
}

/*
 * osif-style task whose control block and stack are both allocated from a psram
 * heap. ram_type selects RAM_TYPE_PSRAM0 or RAM_TYPE_PSRAM1; priority is an osif
 * priority (higher = more urgent) and is mapped to a Zephyr priority the same way
 * osif does. Returns true and stores an opaque handle in *pp_handle on success.
 */
struct psram_task
{
    struct k_thread zthread;
    void *stack_start;
};

bool psram_task_create(void **pp_handle, const char *p_name, void (*p_routine)(void *),
                       void *p_param, uint16_t stack_size, uint16_t priority,
                       RAM_TYPE ram_type)
{
    if (pp_handle == NULL)
    {
        return false;
    }
    *pp_handle = NULL;

    /* Only the psram heaps are valid here. Reject anything else up front:
     * os_mem_* would otherwise index its per-ram-type heap array directly, so a
     * garbage/out-of-range value is an out-of-bounds access, and a valid but
     * non-psram type (e.g. RAM_TYPE_DATA_ON) would silently put the stack in
     * sram - breaking this function's contract. */
    if ((ram_type != RAM_TYPE_PSRAM0) && (ram_type != RAM_TYPE_PSRAM1))
    {
        printk("psram_task_create: ram_type %d is not a psram heap\n", ram_type);
        return false;
    }

    /* task control block from psram */
    struct psram_task *task = os_mem_zalloc(ram_type, sizeof(struct psram_task));

    if (task == NULL)
    {
        return false;
    }

    /* thread stack from psram (8-byte aligned, same as osif os_task_create) */
    k_thread_stack_t *stack = os_mem_aligned_alloc(ram_type, stack_size, 8);

    if (stack == NULL)
    {
        os_mem_free(task);
        return false;
    }
    memset(stack, 0, stack_size);
    task->stack_start = stack;

    /* osif priority (higher = more urgent) -> zephyr priority, as osif does */
    int switch_priority = CONFIG_ZEPHYR_PRI_MAX - priority;

    k_tid_t tid = k_thread_create(&task->zthread, stack, stack_size,
                                  (k_thread_entry_t)p_routine, p_param, NULL, NULL,
                                  switch_priority, 0, K_NO_WAIT);
    if (tid == NULL)
    {
        os_mem_aligned_free(stack);
        os_mem_free(task);
        return false;
    }
    k_thread_name_set(tid, p_name);

    *pp_handle = task;
    return true;
}

bool psram_task_delete(void *p_handle)
{
    struct psram_task *task = (struct psram_task *)p_handle;

    if (task == NULL)
    {
        return false;
    }

    k_thread_abort(&task->zthread);
    os_mem_aligned_free(task->stack_start);
    os_mem_free(task);
    return true;
}

/* osif task routines take a single void* arg; adapt the 3-arg worker */
static void osif_worker(void *arg)
{
    psram_stack_worker(arg, NULL, NULL);
}

/* [3.1] dynamic task + dynamic stack: psram_task_create() takes both from the
 * PSRAM1 heap (RAM_TYPE_PSRAM0 works the same way). An osif task is not joined,
 * so wait on the worker's completion sem and then delete it. */
static int demo_dyntask_dynstack(void)
{
    const char *tag = "task_dynstack";
    void *handle = NULL;
    int fail = 0;

    printf("  [3.1] dynamic task + dynamic psram stack (both from the PSRAM1 heap)\n");

    worker_result = 0;
    k_sem_reset(&worker_sem);

    if (!psram_task_create(&handle, tag, osif_worker, NULL,
                           PSRAM_STACK_SIZE, 3 /* osif priority */, RAM_TYPE_PSRAM1))
    {
        printf("      [FAIL] %s: psram_task_create\n", tag);
        return 1;
    }

    if (k_sem_take(&worker_sem, K_MSEC(1000)) != 0 || worker_result != WORKER_EXPECT)
    {
        printf("      [FAIL] %s: worker did not finish correctly\n", tag);
        fail = 1;
    }
    else
    {
        printf("      [PASS] %s\n", tag);
    }

    k_msleep(10);   /* let the worker fully exit before freeing its stack */
    psram_task_delete(handle);
    return fail;
}

/* [3.2] dynamic task + static stack: the struct k_thread is allocated from the
 * PSRAM0 heap, the stack is the build-time array in the PSRAM0_MCU region. Here
 * the k_thread object is ours, so the thread can simply be joined. */
static int demo_dyntask_staticstack(void)
{
    const char *tag = "task_staticstack";
    int fail = 0;

    printf("  [3.2] dynamic task + static psram stack (PSRAM0_MCU section)\n");

    struct k_thread *t = os_mem_zalloc(OS_MEM_TYPE_PSRAM0, sizeof(struct k_thread));

    if (t == NULL)
    {
        printf("      [FAIL] %s: alloc k_thread from psram\n", tag);
        return 1;
    }

    worker_result = 0;
    k_sem_reset(&worker_sem);

    k_tid_t tid = k_thread_create(t, task_static_stack,
                                  K_THREAD_STACK_SIZEOF(task_static_stack),
                                  psram_stack_worker, NULL, NULL, NULL,
                                  5 /* zephyr priority */, 0, K_NO_WAIT);
    k_thread_name_set(tid, tag);

    if (k_thread_join(t, K_MSEC(1000)) != 0)
    {
        printf("      [FAIL] %s: join timeout\n", tag);
        fail = 1;
    }
    else if (worker_result != WORKER_EXPECT)
    {
        printf("      [FAIL] %s: bad worker result\n", tag);
        fail = 1;
    }
    else
    {
        printf("      [PASS] %s (task@%p stack@%p)\n", tag,
               (void *)t, (void *)task_static_stack);
    }

    os_mem_free(t);
    return fail;
}

int psram_thread_stack_demo(void)
{
    int fail = 0;

    printf("\n[3] ---- PSRAM thread-stack demos ----\n");
    fail += demo_dyntask_dynstack();
    fail += demo_dyntask_staticstack();
    printf("    [%s] thread-stack demos (both variants)\n", fail ? "FAIL" : "PASS");
    return fail;
}
