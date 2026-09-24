/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*
 * Helpers for placing variables (or thread stacks) directly in a psram
 * memory-region instead of referencing the raw dts address. Each psram node in
 * this board carries compatible = "zephyr,memory-region" + a region name, which
 * generates a NOLOAD linker section; a symbol tagged with the matching macro
 * below lands in that region and shows up as used in the build memory report.
 *
 * Each macro is guarded with DT_NODE_EXISTS: if the board does not define the
 * node, the macro expands to nothing so the symbol falls back to default RAM and
 * the build still compiles.
 *
 * IMPORTANT: these sections are NOLOAD - they are NOT zeroed or copied at boot.
 * psram is only powered up in app_system_lower_init() during main(), so any
 * variable placed here must be initialised at runtime (after that point); do not
 * rely on a static initialiser or implicit zero-init. Do not target these
 * regions with zephyr_code_relocate() either (its boot-time copy/zero runs
 * before psram is up).
 */

#ifndef APP_PSRAM_SECTION_H_
#define APP_PSRAM_SECTION_H_

#include <zephyr/devicetree.h>
#include <zephyr/linker/devicetree_regions.h>

/* Cacheable psram0 region. */
#if DT_NODE_EXISTS(DT_NODELABEL(psram0_for_mcu))
#define SECTION_PSRAM0_MCU __attribute__((__section__(LINKER_DT_NODE_REGION_NAME(DT_NODELABEL(psram0_for_mcu)))))
#else
#define SECTION_PSRAM0_MCU
#endif

/* Cacheable psram1 region. */
#if DT_NODE_EXISTS(DT_NODELABEL(psram1_for_mcu))
#define SECTION_PSRAM1_MCU __attribute__((__section__(LINKER_DT_NODE_REGION_NAME(DT_NODELABEL(psram1_for_mcu)))))
#else
#define SECTION_PSRAM1_MCU
#endif

/* Non-cacheable psram0 region (MPU attr 0x44). Use for buffers shared across
 * bus masters / threads that need coherent reads/writes without cache
 * maintenance. */
#if DT_NODE_EXISTS(DT_NODELABEL(psram0_nc))
#define SECTION_PSRAM0_NC __attribute__((__section__(LINKER_DT_NODE_REGION_NAME(DT_NODELABEL(psram0_nc)))))
#else
#define SECTION_PSRAM0_NC
#endif

/* Non-cacheable psram1 region (MPU attr 0x44). */
#if DT_NODE_EXISTS(DT_NODELABEL(psram1_nc))
#define SECTION_PSRAM1_NC __attribute__((__section__(LINKER_DT_NODE_REGION_NAME(DT_NODELABEL(psram1_nc)))))
#else
#define SECTION_PSRAM1_NC
#endif

#endif /* APP_PSRAM_SECTION_H_ */
