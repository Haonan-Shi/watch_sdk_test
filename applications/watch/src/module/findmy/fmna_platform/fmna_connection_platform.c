/*
 *      Copyright (C) 2020 Apple Inc. All Rights Reserved.
 *
 *      Find My Network ADK is licensed under Apple Inc's MFi Sample Code License Agreement,
 *      which is contained in the License.txt file distributed with the Find My Network ADK,
 *      and only to those who accept that license.
 */
#include <string.h>
#include "fmna_connection_platform.h"
#include "fmna_platform_includes.h"
#include "fmna_constants.h"
#include "fmna_connection.h"
#include "fmna_gatt_platform.h"
#include "fmna_state_machine.h"
#include "fmna_peer_manager.h"
#include <gap_conn_le.h>
#include "fmc_api.h"
#include "app_findmy_ble.h"
#include "app_findmy.h"
#include "bt_bond_le.h"
#include "system_status_api.h"

fmna_ret_code_t fmna_connection_platform_disconnect(uint16_t conn_handle)
{
    bool ret = false;
    if (app_findmy_is_findmy_link(conn_handle))
    {
        ret = le_disconnect((uint8_t)conn_handle);
    }
    return ret;
}

void fmna_connection_platform_gap_params_init(void)
{

}


void fmna_connection_platform_conn_params_init(void)
{
//    fmna_ret_code_t ret_code;

}

bool fmna_handle_ble_evt(T_FMNA_BLE_EVT_TYPE evt_type, uint8_t conn_id)
{
    bool ret = false;
    switch (evt_type)
    {
    case FMNA_CONNECTED:
        {
            APP_PRINT_INFO1("fmna_handle_ble_evt: FMNA_CONNECTED conn_id %d", conn_id);
            uint16_t conn_interval;
            le_get_conn_param(GAP_PARAM_CONN_INTERVAL, &conn_interval, conn_id);
            fmna_connection_connected_handler(conn_id, conn_interval);
            fmna_on_connect(conn_id);
            ret = true;
        }
        break;

    case FMNA_DISCONNECTED:
        {
            if (fmna_connection_is_valid_connection(conn_id))
            {
                APP_PRINT_INFO1("fmna_handle_ble_evt: FMNA_DISCONNECTED conn_id %d", conn_id);

                fmna_connection_disconnected_handler(conn_id);
                fmna_on_disconnect(conn_id);
                fmna_connection_update_connection_info(conn_id, FMNA_MULTI_STATUS_ENCRYPTED, false);
                ret = true;
            }
        }
        break;

    case FMNA_CONN_PARAM_UPDATE:
        {
            if (fmna_connection_is_valid_connection(conn_id))
            {
                APP_PRINT_INFO1("fmna_handle_ble_evt: FMNA_CONN_PARAM_UPDATE conn_id %d", conn_id);
                uint16_t conn_interval;
                le_get_conn_param(GAP_PARAM_CONN_INTERVAL, &conn_interval, conn_id);
                fmna_connection_conn_param_update_handler(conn_id, conn_interval);
                ret = true;
            }
        }
        break;

    case FMNA_AUTHEN_SUCCESS:
        {
            if (fmna_connection_is_valid_connection(conn_id))
            {
                APP_PRINT_INFO1("fmna_handle_ble_evt: FMNA_AUTHEN_SUCCESS conn_id %d", conn_id);

                if (app_global_data.app_bond_idx[FINDMY_APP] != 0xFF)
                {
                    APP_PRINT_INFO1("fmna_handle_ble_evt: FMNA_AUTHEN_SUCCESS app_bond_idx 0x%x",
                                    app_global_data.app_bond_idx[FINDMY_APP]);
                    T_LE_BOND_ENTRY *findmy_key_entry = NULL;
                    findmy_key_entry = bt_le_find_key_entry_by_idx(app_global_data.app_bond_idx[FINDMY_APP]);
                    bt_le_set_high_priority_bond(findmy_key_entry);
                }

                fmna_pm_conn_sec_handle(conn_id);
                ret = true;
            }
        }
        break;

    default:
        APP_PRINT_ERROR1("fmna_handle_ble_evt: unknown evt_type %d", evt_type);
        break;
    }
    return ret;
}

void fmna_connection_platform_log_token_help(void *auth_token, uint16_t token_size, void *auth_uuid,
                                             uint16_t uuid_size)
{
    APP_PRINT_INFO3("fmna_connection_platform_log_token_help: MFi token UUID %b, uuid_size %d, token_size %d",
                    TRACE_BINARY(uuid_size, auth_uuid), uuid_size, token_size);
}

#define MFI_TOKEN_MAX_LOG_CHUNK 64
void fmna_connection_platform_log_token(void *auth_token, uint16_t token_size)
{
    uint16_t token_remaining = token_size;
    uint8_t *p_temp = auth_token;
    uint16_t to_print;

    // cppcheck-suppress syntaxError
    FMNA_LOG_INFO("fmna_connection_platform_log_token: MFi Token");
    while (token_remaining)
    {
        if (token_remaining > MFI_TOKEN_MAX_LOG_CHUNK)
        {
            to_print = MFI_TOKEN_MAX_LOG_CHUNK;
        }
        else
        {
            to_print = token_remaining;
        }
        FMNA_LOG_HEXDUMP_INFO(p_temp, to_print);
        token_remaining -= to_print;
        p_temp += to_print;
    }
}


char num_to_char(uint8_t nibble)
{
    if (nibble < 10)
    {
        return (('0' + nibble));
    }

    return (('a' + nibble - 10));
}
//#include "system_rtl876x.h"
//TODO: confirm spec, any SN can be accepted?
void fmna_connection_platform_get_serial_number(uint8_t *pSN, uint8_t length)
{
    uint8_t temp[8];
    uint16_t remaining = length;
    int i = 0;
    uint8_t bt_addr[8];
    gap_get_param(GAP_PARAM_BD_ADDR, bt_addr);
    uint8_t chip_id = sys_hall_read_chip_id();
    // xor device id and bd addr to identify the device
    *((uint32_t *)temp) =  chip_id;
    *((uint32_t *)temp) ^= *((uint32_t *)bt_addr);

    *((uint32_t *)(temp + 4)) =  chip_id;
    *((uint32_t *)(temp + 4)) ^= *((uint32_t *)(bt_addr + 4));

    // Convert to a character string
    for (; i < 8 && remaining; ++i)
    {
        pSN[2 * i] = num_to_char((temp[i] & 0x0f));
        remaining--;
        if (remaining)
        {
            pSN[2 * i + 1] = num_to_char(((temp[i] >> 4) & 0x0f));
            remaining--;
        }
    }

    // Pad remaining with 'f'
    if (remaining)
    {
        pSN[i] = 'f';
        remaining--;
        i++;
    }
}

// TODO remove / replace this with POR storage
// Using fstorage as temp storage for Token
// Please note this implementation is not 100% safe
// There is a window after erase and before write completes
// that there is no Token stored. A reset in this window could
// cause the device to become unusable.
#define CHECK_FLASH_RET(ret, _label_) if (!ret) goto _label_;

bool m_new_token_stored = false;

void fmna_connection_update_mfi_token_storage(mfi_info_t *p_data, uint16_t data_size)
{
    uint16_t auth_total_len = SOFTWARE_AUTH_TOKEN_BLEN + SOFTWARE_AUTH_UUID_BLEN;
    uint8_t m_temp_auth_buffer[auth_total_len];
    uint8_t *p_uuid = p_data->m_software_auth_uuid;
    uint8_t *p_token = p_data->p_software_auth_token;
    bool ret = false;

    if (data_size != auth_total_len)
    {
        APP_PRINT_ERROR0("fmna_connection_update_mfi_token_storage: token length not accepted");
        goto error;
    }

    /* Backup token in flash*/
    ret = fmc_flash_nor_read(APPLE_AUTH_TOKEN_SAVE_ADDRESS,
                             (uint8_t *)m_temp_auth_buffer, auth_total_len);
    CHECK_FLASH_RET(ret, error);

    ret = fmc_flash_nor_erase(APPLE_AUTH_TOKEN_BACKUP_ADDRESS, FMC_FLASH_NOR_ERASE_SECTOR);
    CHECK_FLASH_RET(ret, error);

    ret = fmc_flash_nor_write(APPLE_AUTH_TOKEN_BACKUP_ADDRESS,
                              (uint8_t *)m_temp_auth_buffer, auth_total_len);
    CHECK_FLASH_RET(ret, error);

    if (memcmp((const void *)APPLE_AUTH_TOKEN_BACKUP_ADDRESS,
               (const void *)m_temp_auth_buffer,
               auth_total_len))
    {
        APP_PRINT_ERROR0("fmna_connection_update_mfi_token_storage: token backup region not matched");
        goto error;
    }

    /* Update token */
    APP_PRINT_INFO0("fmna_connection_update_mfi_token_storage: token update");
    memcpy(m_temp_auth_buffer, p_uuid, SOFTWARE_AUTH_UUID_BLEN);
    memcpy(m_temp_auth_buffer + SOFTWARE_AUTH_UUID_BLEN, p_token, SOFTWARE_AUTH_TOKEN_BLEN);

    ret = fmc_flash_nor_erase(APPLE_AUTH_TOKEN_SAVE_ADDRESS, FMC_FLASH_NOR_ERASE_SECTOR);
    CHECK_FLASH_RET(ret, error);

    ret = fmc_flash_nor_write(APPLE_AUTH_TOKEN_SAVE_ADDRESS,
                              (uint8_t *)m_temp_auth_buffer, auth_total_len);
    CHECK_FLASH_RET(ret, error);

    if (memcmp((const void *)APPLE_AUTH_TOKEN_SAVE_ADDRESS,
               (const void *)m_temp_auth_buffer,
               auth_total_len))
    {
        APP_PRINT_ERROR0("fmna_connection_update_mfi_token_storage: token region not matched");
        goto error;
    }

    m_new_token_stored = true;
    fmna_state_machine_dispatch_event(FMNA_SM_EVENT_FMNA_PAIRING_MFITOKEN);
    return;

error:
    APP_PRINT_ERROR0("fmna_connection_update_mfi_token_storage: failed!");
    m_new_token_stored = false;
    return;
}

bool fmna_connection_mfi_token_stored(void)
{
    return m_new_token_stored;
}
