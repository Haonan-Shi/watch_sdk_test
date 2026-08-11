/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _WALKIE_TALKIE_SCAN_H_
#define _WALKIE_TALKIE_SCAN_H_

#include "rtl876x.h"

bool walkie_talkie_receiver_working(void);
bool walkie_talkie_scan_start(void);
bool walkie_talkie_scan_stop(void);
T_GAP_CAUSE walkie_talkie_scan_connect(uint8_t *bd_addr);

#endif /* _WALKIE_TALKIE_SCAN_H_ */
