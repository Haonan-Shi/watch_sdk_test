/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef fmna_debug_control_point_h
#define fmna_debug_control_point_h

#include "fmna_gatt.h"

#ifdef DEBUG

/// Function for handling the different Debug opcodes.
/// @param data Buffer of data of debug opcode and possible operands
void fmna_debug_control_point_rx_handler(uint16_t conn_handle, uint8_t const *data,
                                         uint16_t length);

#endif // DEBUG

#endif /* fmna_debug_control_point_h */


