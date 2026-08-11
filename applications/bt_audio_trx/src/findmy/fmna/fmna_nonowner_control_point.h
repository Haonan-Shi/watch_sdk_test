/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef fmna_nonowner_control_point_h
#define fmna_nonowner_control_point_h

#include "fmna_gatt.h"

extern bool m_aggressive_ut_adv_enabled;

/// Function for handling the different Non-owner opcodes.
/// @param data Buffer of data of non-owner opcode and possible operands
void fmna_nonowner_rx_handler(uint16_t conn_handle, uint8_t const *data, uint16_t length);

#endif /* fmna_nonowner_control_point_h */
