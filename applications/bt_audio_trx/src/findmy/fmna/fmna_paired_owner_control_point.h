/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef fmna_paired_owner_control_point_h
#define fmna_paired_owner_control_point_h

#include "fmna_gatt.h"

/// Function for handling the different Paired Owner opcodes.
/// @param data Buffer of data of Paired Owner opcode and possible operands
void fmna_paired_owner_rx_handler(uint16_t conn_handle, uint8_t const *data, uint16_t length);

#endif /* fmna_paired_owner_control_point_h */


