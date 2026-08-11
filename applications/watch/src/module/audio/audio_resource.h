/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _RESOURCE_H_
#define _RESOURCE_H_

#include "flash_map.h"
#if (CONFIG_APP_NANDBOOT)
#define   MUSIC_NAME_BIN_ADDR                      PSRAM_APP_DEFINED_SECTION_ADDR
#else
#define   MUSIC_NAME_BIN_ADDR                      APP_DEFINED_SECTION_ADDR
#endif

/* Reduced music storage to allocate 12KB for recorder metadata (total still 60KB) */
#define   MUSIC_NAME_BIN_SIZE                      0x8000       // 32KB (was 40KB)
#define   MUSIC_HEADER_BIN_ADDR                    (MUSIC_NAME_BIN_ADDR + MUSIC_NAME_BIN_SIZE)
#define   MUSIC_HEADER_BIN_SIZE                     0x4000       // 16KB (was 20KB)


/* Recorder Playlist Metadata (12KB) - stored after music header */
#define   RECORD_NAME_BIN_ADDR                      (MUSIC_HEADER_BIN_ADDR + MUSIC_HEADER_BIN_SIZE)
#define   RECORD_NAME_BIN_SIZE                      0x2000       // 8KB 
#define   RECORD_HEADER_BIN_ADDR                    (RECORD_NAME_BIN_ADDR + RECORD_NAME_BIN_SIZE)
#define   RECORD_HEADER_BIN_SIZE                     0x1000       // 4KB 



#endif /* _RESOURCE_H_ */