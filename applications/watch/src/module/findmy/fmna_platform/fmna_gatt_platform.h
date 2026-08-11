/*
 *      Copyright (C) 2020 Apple Inc. All Rights Reserved.
 *
 *      Find My Network ADK is licensed under Apple Inc's MFi Sample Code License Agreement,
 *      which is contained in the License.txt file distributed with the Find My Network ADK,
 *      and only to those who accept that license.
 */

#ifndef fmna_gatt_platform_h
#define fmna_gatt_platform_h

#include "fmna_gatt.h"
#include "fmna_constants.h"
#include "fmna_platform_includes.h"
#include "gap.h"
#include "gap_adv.h"
#include "gap_bond_le.h"
#include "profile_server.h"

#define BLE_CONN_HANDLE_INVALID 0xFF
#define FINDMY_TPS_VALUE 4

/*============================================================================*
 *                              Functions
 *============================================================================*/

void fmna_gatt_platform_init(void);
void fmna_gatt_platform_services_init(void);

void fmna_gatt_platform_send_authorized_write_reply(bool accept);
uint16_t fmna_gatt_platform_get_most_recent_conn_handle(void);
ret_code_t fmna_gatt_platform_send_indication(uint16_t conn_handle, FMNA_Service_Opcode_t *opcode,
                                              uint8_t *data, uint16_t length);
uint8_t fmna_gatt_platform_send_indication_busy(uint16_t conn_handle, FMNA_Service_Opcode_t opcode,
                                                void *data, uint16_t length);
void fmna_gatt_platform_reset_indication_queue(void);
uint8_t fmna_gatt_platform_get_next_command_response_index(void);
void fmna_gatt_platform_send_next_indication(void);
void fmna_gatt_indication_state_set(bool new_state);

void fmna_on_connect(uint8_t conn_id);
void fmna_on_disconnect(uint8_t conn_id);
bool fmna_link_by_conn_id(uint8_t conn_id);

#endif /* fmna_gatt_platform_h */
