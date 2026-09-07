/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef fmna_version_h
#define fmna_version_h

#define FW_VERSION_MAJOR_NUMBER     1
#define FW_VERSION_MINOR_NUMBER     0
#define FW_VERSION_REVISION_NUMBER  1

void fmna_version_init(void);
uint32_t fmna_version_get_fw_version(void);

#endif /* fmna_version_h */
