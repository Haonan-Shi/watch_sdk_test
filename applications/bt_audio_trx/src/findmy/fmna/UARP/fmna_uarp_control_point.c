/*
 *      Copyright (C) 2020 Apple Inc. All Rights Reserved.
 *
 *      Find My Network ADK is licensed under Apple Inc.MFi Sample Code License Agreement,
 *      which is contained in the License.txt file distributed with the Find My Network ADK,
 *      and only to those who accept that license.
 */

#if CONFIG_REALTEK_FINDMY_USE_UARP
#include "fmna_platform_includes.h"
#include "fmna_gatt.h"
#include "fmna_gatt_platform.h"
#include "fmna_state_machine.h"
#include "fmna_connection.h"
#include "fmna_crypto.h"
#include "fmna_version.h"
#include "fmna_uarp_control_point.h"

#define UARP_MAX_LEN kFMNASamplePayloadWindowSize

#include "FMNASampleUARP.h"

//MARK: RX Packet Definitions

// General buffer to hold the incoming Pairing characteristic Write data
typedef struct
{
    uint16_t len;
    uint8_t data[UARP_MAX_LEN];
} uarp_buffer_t;

static uarp_buffer_t uarp_rx_buffer = {};

// Only allow one controller
static uint16_t uarp_conn_handle = CONN_HANDLE_INVALID;

static struct FMNASampleAccessory uarpAccessory = {};
static struct FMNASampleController uarpController = {};
static struct UARPVersion activeFW = {};
static struct UARPVersion stagedFW = {};
static struct FMNASampleCallbacks uarpCBs = {};

// Hold info for sending messages

char dummySerial[SERIAL_NUMBER_RAW_BLEN + 1];

static void fmna_uarp_dispatch_send_packet_complete_handler(void *p_event_data,
                                                            uint16_t event_size)
{
    uint32_t ret_code;
    FMNA_LOG_INFO("fmna_uarp_dispatch_send_packet_complete_handler: %x, %x", &uarpAccessory,
                  &uarpController);
    ret_code = FMNASampleSendMessageComplete(&uarpAccessory,
                                             &uarpController);
    FMNA_LOG_INFO("fmna_uarp_dispatch_send_packet_complete_handler: returned %x", ret_code);
}

void fmna_uarp_packet_sent(void)
{
    app_findmy_sched_event_put(NULL, 0, fmna_uarp_dispatch_send_packet_complete_handler);
}

static void fmna_uarp_dispatch_rx_handler(void *p_event_data, uint16_t event_size)
{
    // Call into UARP handler
    uint32_t ret_val;
    ret_val = FMNASampleRecvMessage(&uarpAccessory,
                                    &uarpController,
                                    uarp_rx_buffer.data,
                                    uarp_rx_buffer.len);
    FMNA_LOG_INFO("fmna_uarp_dispatch_rx_handler: length %x status %x", uarp_rx_buffer.len, ret_val);
    memset(&uarp_rx_buffer, 0, sizeof(uarp_rx_buffer));
    fmna_gatt_platform_send_authorized_write_reply(true);
}

void fmna_uarp_control_point_handle_rx(void)
{
    FMNA_LOG_INFO("fmna_uarp_control_point_handle_rx");
    app_findmy_sched_event_put(NULL, 0, fmna_uarp_dispatch_rx_handler);
}


uint32_t fmna_uarp_send_msg(void *pAccessoryDelegate, void *pControllerDelegate, uint8_t *pBuffer,
                            uint32_t length)
{
    FMNA_LOG_INFO("fmna_uarp_send_msg 0x%x", pBuffer);
    fmna_gatt_send_indication(uarp_conn_handle, FMNA_SERVICE_OPCODE_INTERNAL_UARP, pBuffer, length);
    return kUARPStatusSuccess;
}

fmna_ret_code_t fmna_uarp_controller_reset(void)
{
    FMNA_LOG_INFO("fmna_uarp_controller_reset");
    if (uarp_conn_handle != CONN_HANDLE_INVALID)
    {
        fmna_uarp_disconnect(uarp_conn_handle);
    }
    return FMNA_SUCCESS;
}

fmna_ret_code_t fmna_uarp_connect(uint16_t conn_handle)
{
    FMNA_LOG_INFO("fmna_uarp_connect");
    if (uarp_conn_handle == CONN_HANDLE_INVALID)
    {
        uarp_conn_handle = conn_handle;
        memset(&uarp_rx_buffer, 0, sizeof(uarp_rx_buffer));
        FMNASampleControllerAdd(&uarpAccessory, &uarpController);
    }
    else
    {
        FMNA_LOG_INFO("fmna_uarp_connect: UARP controller already established current handle %x, new conn %x",
                      uarp_conn_handle, conn_handle);
    }
    return FMNA_SUCCESS;
}

fmna_ret_code_t fmna_uarp_disconnect(uint16_t conn_handle)
{
    FMNA_LOG_INFO("fmna_uarp_disconnect %d", conn_handle);
    if (conn_handle == uarp_conn_handle)
    {
        FMNA_LOG_INFO("fmna_uarp_disconnect: removing controller for handle %d", conn_handle);
        uarp_conn_handle = CONN_HANDLE_INVALID;
        memset(&uarp_rx_buffer, 0, sizeof(uarp_rx_buffer));
        FMNASampleControllerRemove(&uarpAccessory, &uarpController);
    }
    else
    {
        FMNA_LOG_INFO("fmna_uarp_disconnect: Not Removing controller, different handle uarp_handle %x conn_handle %d",
                      uarp_conn_handle, conn_handle);
    }
    return FMNA_SUCCESS;
}

void fmna_uarp_control_point_init(void)
{
    FMNA_LOG_INFO("fmna_uarp_control_point_init");
    uint32_t ret_code  = 0;
    // The FW version code needs to verfied
    activeFW.build = 0;
    activeFW.major = FW_VERSION_MAJOR_NUMBER;
    activeFW.minor = FW_VERSION_MINOR_NUMBER;
    activeFW.release = FW_VERSION_REVISION_NUMBER;
    uarpCBs.fSendMessage = fmna_uarp_send_msg;

    memcpy(dummySerial, fmna_crypto_get_serial_number_raw(), SERIAL_NUMBER_RAW_BLEN);
    dummySerial[SERIAL_NUMBER_RAW_BLEN] = 0;
    ret_code = FMNASampleInit(&uarpAccessory,
                              FMNA_MANUFACTURER_NAME,
                              FMNA_MODEL_NAME,
                              (const char *)dummySerial,
                              FMNA_HARDWARE_VERSION,
                              &activeFW,
                              &stagedFW,
                              &uarpCBs);
    FMNA_LOG_INFO("fmna_uarp_control_point_init: returned %x", ret_code);
}

fmna_ret_code_t fmna_uarp_control_point_append_to_rx_buffer(uint8_t const *data, uint16_t length)
{
    FMNA_LOG_INFO("fmna_uarp_control_point_append_to_rx_buffer");
    if ((uarp_rx_buffer.len + length) > UARP_MAX_LEN)
    {
        FMNA_LOG_ERROR("fmna_uarp_control_point_append_to_rx_buffer: Too much UARP data in packet");
        memset(&uarp_rx_buffer, 0, sizeof(uarp_rx_buffer));
        return FMNA_ERROR_INVALID_LENGTH;
    }

    memcpy(&(uarp_rx_buffer.data[uarp_rx_buffer.len]), data, length);
    uarp_rx_buffer.len += length;

    return FMNA_SUCCESS;
}

fmna_ret_code_t fmna_uarp_authorized_rx_handler(uint16_t conn_handle, uint8_t const *data,
                                                uint16_t length)
{
    fmna_ret_code_t ret_code = FMNA_SUCCESS;
    FMNA_LOG_INFO("fmna_uarp_authorized_rx_handler: Write request of UARP Characteristic flags %x",
                  data[FRAGMENTED_FLAG_INDEX]);

    if (fmna_connection_is_fmna_paired() != true)
    {
        FMNA_LOG_ERROR("fmna_uarp_authorized_rx_handler: Not paired");
        fmna_gatt_platform_send_authorized_write_reply(false);
        return FMNA_ERROR_INVALID_STATE;
    }

    if (uarp_conn_handle != conn_handle)
    {
        // drop the packet not the correct connection handle
        FMNA_LOG_ERROR("fmna_uarp_authorized_rx_handler: Not current Controller, Dropping");
        fmna_gatt_platform_send_authorized_write_reply(true);
        return FMNA_ERROR_INVALID_STATE;
    }

    ret_code = fmna_uarp_control_point_append_to_rx_buffer(&data[FRAGMENTED_FLAG_LENGTH],
                                                           length - FRAGMENTED_FLAG_LENGTH);
    if (ret_code != FMNA_SUCCESS)
    {
        // TODO send an error?
        FMNA_LOG_ERROR("fmna_uarp_authorized_rx_handler: fmna_uarp_control_point_append_to_rx_buffer error %x",
                       ret_code);
        return ret_code;
    }

    if (data[FRAGMENTED_FLAG_INDEX] & FRAGMENTED_FLAG_FINAL)
    {
        // packet reassembled so give to UARP
        // Delay allowing gatt writes until the packet is processes so the rx buffer is not overwritten
        fmna_uarp_control_point_handle_rx();
    }
    else
    {
        FMNA_LOG_ERROR("fmna_uarp_authorized_rx_handler: fmna_uarp_control_point_append_to_rx_buffer fragment not final");
        fmna_gatt_platform_send_authorized_write_reply(true);
    }

    return ret_code;
}
#endif
