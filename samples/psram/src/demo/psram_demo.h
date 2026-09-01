/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef PSRAM_DEMO_H_
#define PSRAM_DEMO_H_

#include <stdbool.h>
#include <stdint.h>
#include "mem_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Each demo returns the number of failed checks (0 == PASS) and prints its own
 * [PASS]/[FAIL] lines. Call them only after app_system_lower_init() (psram
 * powered up) and psram_heap_init() (psram heaps registered). */
int  psram_static_var_demo(void);
int  psram_heap_demo(void);
int  psram_cache_demo(void);

/* Thread-stack demos (psram_stack.c): a dynamically created task with a
 * dynamically allocated psram stack, and one with a statically defined psram
 * stack. */
int  psram_thread_stack_demo(void);

/* Print size / free / used bytes of the PSRAM0/PSRAM1 heaps (heap "static
 * check"). The free bytes come from os_mem_peek(), the size from the devicetree
 * heap node. */
void psram_heap_peek(void);

/*
 * osif-style task create/delete whose thread stack AND control block are
 * allocated from a psram heap (implemented in psram_stack.c). Mirrors
 * os_task_create()/os_task_delete() (osif_zephyr.c os_task_create_zephyr) but
 * takes the psram RAM_TYPE to allocate from:
 *   ram_type : RAM_TYPE_PSRAM0 or RAM_TYPE_PSRAM1
 *   priority : osif priority (higher = more urgent), mapped to a Zephyr
 *              priority the same way osif does (CONFIG_ZEPHYR_PRI_MAX - prio).
 *
 * Returns true and stores an opaque handle in *pp_handle on success.
 */
bool psram_task_create(void **pp_handle, const char *p_name, void (*p_routine)(void *),
                       void *p_param, uint16_t stack_size, uint16_t priority,
                       RAM_TYPE ram_type);

/* Abort the task and free its psram stack + control block. */
bool psram_task_delete(void *p_handle);

#ifdef __cplusplus
}
#endif

#endif /* PSRAM_DEMO_H_ */
