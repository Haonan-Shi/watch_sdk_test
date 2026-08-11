/*
 *      Copyright (C) 2020 Apple Inc. All Rights Reserved.
 *
 *      Find My Network ADK is licensed under Apple Inc's MFi Sample Code License Agreement,
 *      which is contained in the License.txt file distributed with the Find My Network ADK,
 *      and only to those who accept that license.
 */

#include "fmna_platform_includes.h"
#include "fmna_peer_manager.h"
#include "fmna_state_machine.h"
#include "fmna_connection.h"
#include "app_ble_gap.h"
#include "bt_bond_le.h"
#include "app_findmy.h"

uint8_t fmna_pm_peer_count(void)
{
    return bt_le_get_bond_dev_num();
}

void fmna_pm_delete_bonds(void)
{
    APP_PRINT_INFO0("fmna_pm_delete_bonds: Erase findmy bond!");
    T_LE_BOND_ENTRY *findmy_key_entry = NULL;
    findmy_key_entry = bt_le_find_key_entry_by_idx(app_global_data.app_bond_idx[FINDMY_APP]);
    bt_le_delete_bond(findmy_key_entry);
}

void fmna_pm_conn_sec_handle(uint16_t conn_handle)
{
    APP_PRINT_INFO1("fmna_pm_conn_sec_handle: conn_handle 0x%x.", conn_handle);

    // mark as encrypted in the connection record
    fmna_connection_update_connection_info(conn_handle, FMNA_MULTI_STATUS_ENCRYPTED, true);

    // BT pairing completed successfully/ link was encrypted. Send BONDED event to state machine.
    fmna_evt_handler(FMNA_SM_EVENT_BONDED, NULL);
}

void peer_manager_init(void)
{
}

