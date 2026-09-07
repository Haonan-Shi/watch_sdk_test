/*
 *      Copyright (C) 2020 Apple Inc. All Rights Reserved.
 *
 *      Find My Network ADK is licensed under Apple Inc.MFi Sample Code License Agreement,
 *      which is contained in the License.txt file distributed with the Find My Network ADK,
 *      and only to those who accept that license.
 */

#ifndef fmna_uarp_control_point_h
#define fmna_uarp_control_point_h

#include "fmna_gatt.h"

/// Function for handling the different UARP messages.
/// @param data Buffer of UARP data
fmna_ret_code_t fmna_uarp_authorized_rx_handler(uint16_t conn_handle, uint8_t const *data,
                                                uint16_t length);

/// Function for indicating an encrypted link has connected
/// @param conn_handle the connection handle for the encrypted connection
fmna_ret_code_t fmna_uarp_connect(uint16_t conn_handle);

/// Function for indicating a link has disconnected
/// @param conn_handle the connection handle for the disconnected link
fmna_ret_code_t fmna_uarp_disconnect(uint16_t conn_handle);

fmna_ret_code_t fmna_uarp_controller_reset(void);

void fmna_uarp_packet_sent(void);

void fmna_uarp_control_point_init(void);

#endif /* fmna_uarp_control_point_h */
