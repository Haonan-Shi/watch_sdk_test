/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*
 * Demo 1: psram used for static/global variables.
 *
 * The variables below are placed in the PSRAM0_MCU / PSRAM1_MCU memory-regions
 * via the section macros. Those regions are NOLOAD, so the C runtime does NOT
 * zero them at boot and psram is only powered up in app_system_lower_init().
 * To get the usual "zero initial value" of a static variable we therefore clear
 * them explicitly at runtime (after psram init) instead of relying on implicit
 * zero-init.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <zephyr/devicetree.h>

#include "psram_section.h"
#include "psram_demo.h"

#define PSRAM_STATIC_ARRAY_LEN 256

static uint8_t  psram0_static_buf[PSRAM_STATIC_ARRAY_LEN] SECTION_PSRAM0_MCU;
static uint32_t psram0_static_counter                     SECTION_PSRAM0_MCU;
static uint8_t  psram1_static_buf[PSRAM_STATIC_ARRAY_LEN] SECTION_PSRAM1_MCU;

static int in_range(const void *p, uintptr_t base, size_t size)
{
    uintptr_t a = (uintptr_t)p;

    return (a >= base) && (a < base + size);
}

int psram_static_var_demo(void)
{
    int fail = 0;

    uintptr_t p0 = DT_REG_ADDR(DT_NODELABEL(psram0_for_mcu));
    size_t    s0 = DT_REG_SIZE(DT_NODELABEL(psram0_for_mcu));
    uintptr_t p1 = DT_REG_ADDR(DT_NODELABEL(psram1_for_mcu));
    size_t    s1 = DT_REG_SIZE(DT_NODELABEL(psram1_for_mcu));

    printf("\n[1] ---- PSRAM static-variable demo ----\n");
    printf("    psram0_static_buf @ %p  (PSRAM0_MCU 0x%08lx size 0x%x)\n",
           (void *)psram0_static_buf, (unsigned long)p0, (unsigned int)s0);
    printf("    psram1_static_buf @ %p  (PSRAM1_MCU 0x%08lx size 0x%x)\n",
           (void *)psram1_static_buf, (unsigned long)p1, (unsigned int)s1);

    /* the linker must have placed the variables inside the psram regions */
    if (!in_range(psram0_static_buf, p0, s0))
    {
        printf("    [FAIL] psram0_static_buf not in PSRAM0_MCU region\n");
        fail++;
    }
    if (!in_range(psram1_static_buf, p1, s1))
    {
        printf("    [FAIL] psram1_static_buf not in PSRAM1_MCU region\n");
        fail++;
    }

    /* NOLOAD region -> clear at runtime to obtain the "zero initial value" */
    memset(psram0_static_buf, 0, sizeof(psram0_static_buf));
    memset(psram1_static_buf, 0, sizeof(psram1_static_buf));
    psram0_static_counter = 0;

    /* write a pattern, read it back */
    for (size_t i = 0; i < PSRAM_STATIC_ARRAY_LEN; i++)
    {
        psram0_static_buf[i] = (uint8_t)(i & 0xFF);
        psram1_static_buf[i] = (uint8_t)(~i & 0xFF);
    }
    for (size_t i = 0; i < PSRAM_STATIC_ARRAY_LEN; i++)
    {
        if (psram0_static_buf[i] != (uint8_t)(i & 0xFF) ||
            psram1_static_buf[i] != (uint8_t)(~i & 0xFF))
        {
            printf("    [FAIL] readback mismatch at %u\n", (unsigned int)i);
            fail++;
            break;
        }
        psram0_static_counter++;
    }

    printf("    counter=%d (expected %d)\n",
           psram0_static_counter, PSRAM_STATIC_ARRAY_LEN);
    if (psram0_static_counter != PSRAM_STATIC_ARRAY_LEN)
    {
        fail++;
    }

    printf("    [%s] static-variable demo\n", fail ? "FAIL" : "PASS");
    return fail;
}
