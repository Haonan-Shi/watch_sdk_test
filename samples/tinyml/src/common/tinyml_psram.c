/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*
 * tinyml_psram.c - shared external-PSRAM bring-up for the combined tinyml
 * sample.  Consolidates the identical edge_psram_init()/ic_psram_init()
 * sequences the standalone samples used to carry, and adds it for the KWS /
 * motion apps (which now also allocate from PSRAM via the unified ts_mem.c).
 *
 * Mirrors the sequence in applications/bt_audio_trx app_lower_init.c:
 *   FMC_SPIC_ID_1 -> psram0, FMC_SPIC_ID_3 -> psram1.
 * Each region is gated on its devicetree node being "okay" (see the board
 * overlay).
 */

#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/printk.h>

#include "fmc_api.h"
#include "fmc_api_ext.h"
#include "tinyml_psram.h"

void tinyml_psram_init(void)
{
#if DT_NODE_HAS_STATUS(DT_NODELABEL(psram0), okay)
    if (fmc_psram_winbond_opi_init(FMC_SPIC_ID_1))
    {
        printk("WB OPI psram0 init success!\n");
    }
    else
    {
        printk("WB OPI psram0 init fail!\n");
    }
    fmc_set_spic_slpck_enable(FMC_SPIC_ID_1, true);

    uint32_t psram0_actual_mhz = 0;
    fmc_psram_clock_switch(FMC_SPIC_ID_1, 280, &psram0_actual_mhz);
    printk("psram0 clock switch: actual mhz = %d\n", psram0_actual_mhz);
#endif

#if DT_NODE_HAS_STATUS(DT_NODELABEL(psram1), okay)
    if (fmc_psram_winbond_opi_init(FMC_SPIC_ID_3))
    {
        printk("WB OPI psram1 init success!\n");
    }
    else
    {
        printk("WB OPI psram1 init fail!\n");
    }
    fmc_set_spic_slpck_enable(FMC_SPIC_ID_3, true);

    uint32_t psram1_actual_mhz = 0;
    fmc_psram_clock_switch(FMC_SPIC_ID_3, 280, &psram1_actual_mhz);
    printk("psram1 clock switch: actual mhz = %d\n", psram1_actual_mhz);
#endif
}
