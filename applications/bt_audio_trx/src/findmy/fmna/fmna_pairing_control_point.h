/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef fmna_pairing_control_point_h
#define fmna_pairing_control_point_h

void fmna_pairing_control_point_unpair(void);
fmna_ret_code_t fmna_pairing_control_point_append_to_rx_buffer(uint8_t const *data,
                                                               uint16_t length);
void fmna_pairing_control_point_handle_rx(void);

#endif /* fmna_pairing_control_point_h */
