/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*
 * tinyml_psram.h - shared external-PSRAM bring-up for the combined tinyml
 * sample.
 *
 * Every application routes ts_malloc()/ts_free() (ts_mem.c) to the heap-4
 * allocator in psRam_heap.c, whose backing ucHeap[] lives in an external
 * PSRAM memory-region (see the board overlay + psram_section.h).  Those
 * regions are only usable once the SPIC controllers are initialised at
 * runtime, so each app's main() must call tinyml_psram_init() BEFORE the
 * first allocation (i.e. before spawning its worker thread).
 */

#ifndef TINYML_PSRAM_H_
#define TINYML_PSRAM_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Bring up the external PSRAM controllers (FMC_SPIC_ID_1 -> psram0,
 * FMC_SPIC_ID_3 -> psram1).  Each controller is gated on its devicetree node
 * being "okay" in the board overlay; missing nodes are skipped. */
void tinyml_psram_init(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* TINYML_PSRAM_H_ */
