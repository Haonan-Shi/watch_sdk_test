/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*
 * Zephyr replacement for the Realtek ROM crc16btx.h.  btxfcs() is the table-
 * driven HDLC/PPP FCS-16 accumulation (reflected CRC-16-CCITT, poly 0x8408).
 *
 * IMPORTANT: the Realtek ROM crc16btx.h seeds the FCS with 0x0000 (BTXFCS_INIT
 * = BTXFCS_GOOD = 0x0000), NOT the standard PPP 0xFFFF.  The vendor firmware and
 * the host-side protocol tool both compute btxfcs(0x0000, ...), so this shim
 * MUST match that seed or every end-to-end CRC check fails.  (A 0xFFFF seed
 * here was the cause of the "crc failed" flood against the unmodified host.)
 */

#ifndef TINYML_EDGE_CRC16BTX_SHIM_H
#define TINYML_EDGE_CRC16BTX_SHIM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BTXFCS_INIT   0x0000u  /* Realtek ROM seed; matches vendor host tool */

/* Incrementally update the FCS-16 over len bytes at cp, starting from fcs. */
uint16_t btxfcs(uint16_t fcs, uint8_t *cp, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* TINYML_EDGE_CRC16BTX_SHIM_H */
