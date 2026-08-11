/*
 *      Copyright (C) 2020 Apple Inc. All Rights Reserved.
 *
 *      Find My Network ADK is licensed under Apple Inc's MFi Sample Code License Agreement,
 *      which is contained in the License.txt file distributed with the Find My Network ADK,
 *      and only to those who accept that license.
 */
#include "fmna_platform_includes.h"
#include "fmna_paired_owner_control_point.h"
#include "fmna_gatt.h"
#include "fmna_state_machine.h"
#include "fmna_crypto.h"
#include "fmna_connection.h"
#include "fmna_malloc_platform.h"
#include "fmna_constants_platform.h"

//MARK: RX Packet Definitions

typedef struct
{
    FMNA_Service_Opcode_t get_current_primary_key_opcode;
} __attribute__((packed)) paired_owner_get_current_primary_key_packet_t;

typedef struct
{
    FMNA_Service_Opcode_t get_icloud_identifier_opcode;
} __attribute__((packed)) paired_owner_get_icloud_identifier_packet_t;

typedef struct
{
    FMNA_Service_Opcode_t get_serial_number_opcode;
} __attribute__((packed)) paired_owner_get_serial_number_packet_t;

/// RX Length Check managers for default and variable length cases.
static fmna_service_length_check_manager_t rx_length_check_managers[] = \
{
    /* RX Opcode                                                    Data Length */
    { FMNA_SERVICE_PAIRED_OWNER_OPCODE_GET_CURRENT_PRIMARY_KEY,     sizeof(paired_owner_get_current_primary_key_packet_t) },
    { FMNA_SERVICE_PAIRED_OWNER_OPCODE_GET_ICLOUD_IDENTIFIER,       sizeof(paired_owner_get_icloud_identifier_packet_t)   },
    { FMNA_SERVICE_PAIRED_OWNER_OPCODE_GET_SERIAL_NUMBER,           sizeof(paired_owner_get_serial_number_packet_t)       },
};

static FMNA_Response_Status_t rx_error_check(uint16_t conn_handle, FMNA_Service_Opcode_t opcode,
                                             uint8_t const *data, uint16_t length)
{
    return fmna_gatt_verify_control_point_opcode_and_length(opcode, length, rx_length_check_managers,
                                                            FMNA_SERVICE_LENGTH_CHECK_MANAGERS_SIZE(rx_length_check_managers));
}

const uint8_t Icloud_id_pairing_invalid[ICLOUD_IDENTIFIER_BLEN] = {0};
const uint8_t Primary_key_pairing_invalid[FMNA_PUBKEY_BLEN] = {0};

void fmna_paired_owner_rx_handler(uint16_t conn_handle, uint8_t const *data, uint16_t length)
{
    FMNA_Service_Opcode_t opcode = *(FMNA_Service_Opcode_t *)data;

    FMNA_Response_Status_t response_status = rx_error_check(conn_handle, opcode, data, length);
    if (response_status != RESPONSE_STATUS_SUCCESS)
    {
        FMNA_LOG_ERROR("fmna_paired_owner_rx_handler: Rx error 0x%x for opcode 0x%x, conn_handle 0x%x",
                       response_status, opcode, conn_handle);
        if (response_status == RESPONSE_STATUS_INVALID_COMMAND)
        {
            opcode = FMNA_SERVICE_OPCODE_NONE;
        }
        fmna_gatt_send_command_response(FMNA_SERVICE_PAIRED_OWNER_OPCODE_COMMAND_RESPONSE, conn_handle,
                                        opcode, response_status);
        return;
    }

    FMNA_LOG_INFO("fmna_paired_owner_rx_handler: Paired owner rx opcode = 0x%x", opcode);
    switch (opcode)
    {
    case FMNA_SERVICE_PAIRED_OWNER_OPCODE_GET_CURRENT_PRIMARY_KEY:
        if (fmna_connection_is_fmna_paired())
        {
            fmna_gatt_send_indication(conn_handle,
                                      FMNA_SERVICE_PAIRED_OWNER_OPCODE_GET_CURRENT_PRIMARY_KEY_RESPONSE,
                                      m_fmna_current_primary_key.public_key, FMNA_PUBKEY_BLEN);
        }
        else
        {
            fmna_gatt_send_indication(conn_handle,
                                      FMNA_SERVICE_PAIRED_OWNER_OPCODE_GET_CURRENT_PRIMARY_KEY_RESPONSE,
                                      (void *)Primary_key_pairing_invalid, FMNA_PUBKEY_BLEN);
        }
        break;

    case FMNA_SERVICE_PAIRED_OWNER_OPCODE_GET_ICLOUD_IDENTIFIER:
        FMNA_LOG_INFO("fmna_paired_owner_rx_handler: connect %d", fmna_connection_is_fmna_paired());
        if (fmna_connection_is_fmna_paired())
        {
            uint8_t icloud_id[ICLOUD_IDENTIFIER_BLEN] = {0};
            ftl_load_from_storage(icloud_id, FTL_SAVE_ICLOUD_ID_ADDR, FTL_SAVE_ICLOUD_ID_SIZE);
            fmna_gatt_send_indication(conn_handle,
                                      FMNA_SERVICE_PAIRED_OWNER_OPCODE_GET_ICLOUD_IDENTIFIER_RESPONSE,
                                      icloud_id, ICLOUD_IDENTIFIER_BLEN);
        }
        else
        {
            fmna_gatt_send_indication(conn_handle,
                                      FMNA_SERVICE_PAIRED_OWNER_OPCODE_GET_ICLOUD_IDENTIFIER_RESPONSE, (void *)Icloud_id_pairing_invalid,
                                      ICLOUD_IDENTIFIER_BLEN);
        }
        break;

    case FMNA_SERVICE_PAIRED_OWNER_OPCODE_GET_SERIAL_NUMBER:
        if (!fmna_connection_is_fmna_paired())
        {
            fmna_gatt_send_command_response(FMNA_SERVICE_PAIRED_OWNER_OPCODE_COMMAND_RESPONSE,
                                            conn_handle,
                                            FMNA_SERVICE_PAIRED_OWNER_OPCODE_GET_SERIAL_NUMBER,
                                            RESPONSE_STATUS_INVALID_STATE);
        }
        else
        {
            // Valid state to read serial number.
            if (serial_number_read_state == true)
            {
                fmna_gatt_send_indication(conn_handle,
                                          FMNA_SERVICE_PAIRED_OWNER_OPCODE_GET_SERIAL_NUMBER_RESPONSE,
                                          p_fmna_encrypted_serial_number_payload,
                                          ENCRYPTED_SERIAL_NUMBER_PAYLOAD_BLEN);
            }
            else
            {
                fmna_gatt_send_command_response(FMNA_SERVICE_PAIRED_OWNER_OPCODE_COMMAND_RESPONSE,
                                                conn_handle,
                                                FMNA_SERVICE_PAIRED_OWNER_OPCODE_GET_SERIAL_NUMBER,
                                                RESPONSE_STATUS_INVALID_COMMAND);
            }
        }

        break;

    default:
        FMNA_LOG_INFO("fmna_paired_owner_rx_handler: Invalid opcode for paired owner control point received");
    }

}
