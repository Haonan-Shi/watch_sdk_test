/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "crc16btx.h"
#include "uart_packet_parser.h"
#include "app_msg.h"
#include "ts_queue.h"
#include "trace.h"
#include "tinyml_main.h"
#include "io_uart.h"
#include "ts_mem.h"
#include "psRam_heap.h"
#include "os_sched.h"
#include "os_timer.h"
#include "model_process.h"
#include "utils.h"

/*============================================================================*
 *                              Variables
 *============================================================================*/
typedef struct
{
    uart_rx_state_t state;
    uint8_t buf[UART_RX_BUF_SIZE];
    uint16_t index;
    uint16_t length;
    uint16_t crc_calc;
    uint16_t crc_recv;
    uint16_t seq; // 2-byte sequence number
    uint8_t cmd;  // command field

    // additional members
    uint8_t *model_buf;      // pointer to received model data buffer
    uint8_t *data_buf;       // pointer to received data buffer
    uint16_t model_crc;      // CRC of the received model data
    uint16_t data_crc;       // CRC of the received data
    uint32_t model_len;      // model data length
    uint32_t data_len;       // data length
} uart_rx_parser_t;

static uart_rx_parser_t uart_rx_parser;

uart_tx_parser_t uart_tx_parser = {0}; // initialize global struct to prevent wild pointers

void tf_quant_param_buffer_send_all(void);
void tf_profiler_buffer_send_all(void);

/*============================================================================*
 *                              Functions
 *============================================================================*/
static bool uart_send_ack(uint16_t seq, uint8_t cmd, const uint8_t *data, uint16_t data_len) ;

void uart_timeout_cb(void *timer)
{
    DBG_DIRECT("uart_timeout_cb called, uart_rx_parser.state %d", uart_rx_parser.state);
    // Timeout handling logic
    uart_rx_parser.state = UART_RX_STATE_IDLE; // reset state machine
    uart_rx_parser.index = 0; // reset index
    uart_rx_parser.length = 0; // reset length
    uart_rx_parser.crc_calc = 0; // reset CRC calculated value
    uart_rx_parser.crc_recv = 0; // reset received CRC value
    uart_rx_parser.seq = 0; // reset sequence number
    uart_rx_parser.cmd = 0; // reset command
    // Clear buffer
    memset(uart_rx_parser.buf, 0, sizeof(uart_rx_parser.buf));
    // Clear other added members
    if (uart_rx_parser.model_buf != NULL)
    {
        ts_free(uart_rx_parser.model_buf); // free model data buffer
        uart_rx_parser.model_buf = NULL; // clear model data buffer pointer
    }
    if (uart_rx_parser.data_buf != NULL)
    {
        ts_free(uart_rx_parser.data_buf); // free data buffer
        uart_rx_parser.data_buf = NULL; // clear data buffer pointer
    }
    uart_rx_parser.model_crc = 0; // clear model CRC
    uart_rx_parser.data_crc = 0; // clear data CRC
    uart_rx_parser.model_len = 0; // clear model data length
    uart_rx_parser.data_len = 0; // clear data length
    // Reset parser
    // Additional states or variables can be reset here if needed
    // For example, if there is a waiting-ACK state, reset it here
    //memset(&uart_rx_parser, 0, sizeof(uart_rx_parser)); // Ensure allocated memory is freed first to avoid memory leaks

}

// CRC description:
// CRC range: length field (2 bytes) + sequence number (2 bytes) + command (1 byte) + data area (data_len bytes),
// i.e. starting from out_buf[2], total length = len_field + 2 bytes.
// Excludes frame header, CRC itself, and frame tail.

// Replace original CRC implementation with btxfcs
static inline uint16_t uart_crc16(const uint8_t *data, uint16_t len)
{
    return btxfcs(BTXFCS_INIT, (uint8_t *)data, len);
}

// Single-byte parse function
void uart_rx_byte_parse(uint8_t byte)
{
    //DBG_DIRECT("uart_rx_parser.state %d, byte %8x", uart_rx_parser.state, byte) ;
    ts_timer_start(uart_timeout_cb);
    switch (uart_rx_parser.state)
    {
    case UART_RX_STATE_IDLE:
        if (byte == UART_FRAME_HEAD1)
        {
            uart_rx_parser.state = UART_RX_STATE_HEAD1;
            uart_rx_parser.index = 0;
        }
        break;
    case UART_RX_STATE_HEAD1:
        if (byte == UART_FRAME_HEAD2)
        {
            uart_rx_parser.state = UART_RX_STATE_HEAD2;
        }
        else
        {
            uart_rx_parser.state = UART_RX_STATE_IDLE;
        }
        break;
    case UART_RX_STATE_HEAD2:
        uart_rx_parser.length = byte; // low byte
        uart_rx_parser.state = UART_RX_STATE_LEN;
        break;
    case UART_RX_STATE_LEN:

        uart_rx_parser.length |= ((uint16_t)byte << 8); // high byte
        DBG_DIRECT("uart_rx_parser.length %d", uart_rx_parser.length);
        uart_rx_parser.buf[0] = uart_rx_parser.length & 0xFF;      // length low byte
        uart_rx_parser.buf[1] = (uart_rx_parser.length >> 8) & 0xFF; // length high byte
        uart_rx_parser.index = 2;
        uart_rx_parser.state = UART_RX_STATE_SEQ1; // next state: sequence number low byte
        break;
    case UART_RX_STATE_SEQ1:
        uart_rx_parser.seq = byte; // sequence number low byte
        uart_rx_parser.buf[uart_rx_parser.index++] = byte;
        uart_rx_parser.state = UART_RX_STATE_SEQ2;
        break;
    case UART_RX_STATE_SEQ2:
        uart_rx_parser.seq |= ((uint16_t)byte << 8); // sequence number high byte
        uart_rx_parser.buf[uart_rx_parser.index++] = byte;
        uart_rx_parser.state = UART_RX_STATE_DATA;
        break;
    case UART_RX_STATE_DATA:
        uart_rx_parser.buf[uart_rx_parser.index++] = byte;
        //DBG_DIRECT("uart_rx_parser.index %d", uart_rx_parser.index);
        // get command field (command byte at index==5)
        if (uart_rx_parser.index == 5)
        {
            uart_rx_parser.cmd = byte;
        }
        // length check: length field only covers seq+cmd+data, not CRC
        // when index reaches length+2, data area ends and we enter CRC
        if (uart_rx_parser.index >= uart_rx_parser.length + 2)   // +2 for the length field itself
        {
            uart_rx_parser.state = UART_RX_STATE_CRC1;
        }
        break;
    case UART_RX_STATE_CRC1:
        uart_rx_parser.crc_recv = byte << 8;
        uart_rx_parser.state = UART_RX_STATE_CRC2;
        break;
    case UART_RX_STATE_CRC2:
        uart_rx_parser.crc_recv |= byte;
        uart_rx_parser.state = UART_RX_STATE_TAIL;
        break;
    case UART_RX_STATE_TAIL:
        if (byte == UART_FRAME_TAIL)
        {
            // CRC check: range covers length+seq+cmd+data (excludes header, CRC, tail)
            uart_rx_parser.crc_calc = uart_crc16(uart_rx_parser.buf, uart_rx_parser.length + 2);
            if (uart_rx_parser.crc_calc == uart_rx_parser.crc_recv)
            {
                DBG_DIRECT("CRC check succeed, cmd %x", uart_rx_parser.cmd);
                switch (uart_rx_parser.cmd)
                {
                case UART_CMD_ACK:
                    DBG_DIRECT("Received ACK packet, seq=%u", uart_rx_parser.seq);
                    // handle ACK packet
                    handle_ack_packet(uart_rx_parser.seq);
                    break;
                case UART_CMD_RETRANS:
                    DBG_DIRECT("Received RETRANS packet, seq=%u", uart_rx_parser.seq);
                    // handle retransmit packet
                    handle_retrans_packet(uart_rx_parser.seq);
                    break;
                default:
                    DBG_DIRECT("Received user command packet, cmd=0x%02x, seq=%d", uart_rx_parser.cmd,
                               uart_rx_parser.seq);
                    // normal / custom command packet
                    // data area starts at buf[5], length = uart_rx_parser.length - 3
                    // buf[0,1]: length, [2,3]: seq, [4]: cmd, [5...]: data
                    handle_user_command(
                        uart_rx_parser.cmd,
                        uart_rx_parser.seq,
                        &uart_rx_parser.buf[5],
                        uart_rx_parser.length - 3
                    );
                    // only send ACK for normal data packets
                    //uart_send_ack(uart_rx_parser.seq, UART_CMD_ACK, NULL, 0);
                    break;
                }
            }
            else
            {
                T_IO_MSG uart_msg = {.type = IO_MSG_TYPE_UART, .subtype = IO_MSG_UART_DATA_PARSER_FAILED, .u.buf = NULL};
                tinyml_send_msg_to_task(&uart_msg);
                DBG_DIRECT("crc failed, uart_rx_parser.crc_calc %x, uart_rx_parser.crc_recv %x",
                           uart_rx_parser.crc_calc, uart_rx_parser.crc_recv);

                // only send retrans request for normal packets
                if (uart_rx_parser.cmd != UART_CMD_ACK && uart_rx_parser.cmd != UART_CMD_RETRANS)
                {
                    //uart_send_ack(uart_rx_parser.seq, UART_CMD_RETRANS);
                }
            }
        }
        // regardless of CRC result, reset state
        uart_rx_parser.state = UART_RX_STATE_IDLE;
        break;
    default:
        uart_rx_parser.state = UART_RX_STATE_IDLE;
        break;
    }
}

// Batch data parse function
void uart_rx_data_parse(const uint8_t *data, uint16_t len)
{
    for (uint16_t i = 0; i < len; ++i)
    {
        uart_rx_byte_parse(data[i]);
    }
}

/*
UART Protocol Packet Format:

| Byte Index   | Field Name        | Length (bytes) | Description
|:------------:|:-----------------|:--------------|:-------------------------------------|
| 0            | Frame Header 1   | 1             | UART_FRAME_HEAD1
| 1            | Frame Header 2   | 1             | UART_FRAME_HEAD2
| 2            | Length Low Byte  | 1             | cmd+data+seq length, low byte
| 3            | Length High Byte | 1             | cmd+data+seq length, high byte
| 4            | Seq Low Byte     | 1             | Sequence number low byte
| 5            | Seq High Byte    | 1             | Sequence number high byte
| 6            | Command          | 1             | cmd
| 7 ~ N        | Data Area        | N-6           | data (length = data_len)
| N+1          | CRC High Byte    | 1             | CRC16 high byte (over length+seq+cmd+data)
| N+2          | CRC Low Byte     | 1             | CRC16 low byte
| N+3          | Frame Tail       | 1             | UART_FRAME_TAIL

- Length field = cmd(1 byte) + data length(data_len) + seq(2 bytes), 2 bytes, little-endian.
- CRC range: length field(2 bytes) + seq(2 bytes) + cmd(1 byte) + data area(data_len bytes), total len_field+2 bytes.
- Total packet length = 2(header) + 2(length) + 2(seq) + 1(cmd) + data_len + 2(CRC) + 1(tail).

Example:
[HEAD1][HEAD2]--[LEN_L][LEN_H]--[SEQ_L][SEQ_H]--[CMD]--[DATA...]--[CRC_H][CRC_L]--[TAIL]
*/

/**
* @brief  Build a UART protocol packet
* @param out_buf      Output buffer
* @param seq          Sequence number (2 bytes)
* @param cmd          Command byte
* @param data         Data area pointer
* @param data_len     Data area length
* @return Total packet length in bytes; 0 if it exceeds the maximum buffer size
*/
uint16_t uart_encode_packet(uint8_t *out_buf, uint16_t seq, uint8_t cmd, const uint8_t *data,
                            uint16_t data_len)
{
    uint16_t total_len = 2 /*head*/ + 2 /*len*/ + 2 /*seq*/ + 1 /*cmd*/ + data_len + 2 /*crc*/ +
                         1 /*tail*/;
    if (total_len > UART_RX_BUF_SIZE) { return 0; }

    uint16_t idx = 0;
    // frame header
    out_buf[idx++] = UART_FRAME_HEAD1;
    out_buf[idx++] = UART_FRAME_HEAD2;
    // length field (seq+cmd+data), little-endian
    uint16_t len_field = 2 /*seq*/ + 1 /*cmd*/ + data_len;
    out_buf[idx++] = len_field & 0xFF;
    out_buf[idx++] = (len_field >> 8) & 0xFF;
    // sequence number
    out_buf[idx++] = seq & 0xFF;
    out_buf[idx++] = (seq >> 8) & 0xFF;
    // command
    out_buf[idx++] = cmd;
    // data
    if (data_len > 0 && data != NULL)
    {
        memcpy(&out_buf[idx], data, data_len);
        idx += data_len;
    }
    // CRC (over length+seq+cmd+data, excludes header, CRC, tail)
    uint16_t crc = uart_crc16(&out_buf[2], len_field + 2);
    out_buf[idx++] = (crc >> 8) & 0xFF;
    out_buf[idx++] = crc & 0xFF;
    // frame tail
    out_buf[idx++] = UART_FRAME_TAIL;
    return idx;
}

// ACK packet send function
// Usage in parsing:
// if (uart_rx_parser.cmd == UART_CMD_ACK) { /* this is an ACK packet */ }
// else if (uart_rx_parser.cmd == UART_CMD_RETRANS) { /* this is a retransmit packet */ }
bool uart_send_ack(uint16_t seq, uint8_t cmd, const uint8_t *data, uint16_t data_len)
{
    uint8_t *ack_buf = ts_malloc(2 /*head*/ + 2 /*len*/ + 2 /*seq*/ + 1 /*cmd*/ + data_len + 2 /*crc*/ +
                                 1 /*tail*/);
    if (ack_buf == NULL)
    {
        DBG_DIRECT("uart_send_ack: ts_malloc failed");
        return false;
    }
    uint16_t ack_len = uart_encode_packet(ack_buf, seq, cmd, data, data_len);
    if (ack_len == 0)
    {
        DBG_DIRECT("ACK packet too large");
        ts_free(ack_buf);
        return false;
    }
    DBG_DIRECT("uart_send_ack: uart_senddata, status %d", data[0]);
    uart_senddata(ack_buf, ack_len);
    ts_free(ack_buf);
    return true;
}

// Send ACK packet
// uart_send_ack(seq, UART_CMD_ACK);

// Send retransmit packet
// uart_send_ack(seq, UART_CMD_RETRANS);

static volatile bool ack_received = false;
static volatile uint16_t ack_seq = 0;

void handle_ack_packet(uint16_t seq)
{
    ack_received = true;
    ack_seq = seq;
}

// Actively send data and wait for ACK, supports retransmission
bool uart_send_with_ack(uint16_t seq, uint8_t cmd, const uint8_t *data, uint16_t data_len,
                        uint32_t timeout_ms, uint8_t max_retry)
{
    // fill TX struct
    uart_tx_parser.seq = seq;
    uart_tx_parser.cmd = cmd;
    uart_tx_parser.data = data;
    uart_tx_parser.data_len = data_len;
    uart_tx_parser.retry_count = 0;
    uart_tx_parser.max_retry = max_retry;
    uart_tx_parser.timeout_ms = timeout_ms;
    uart_tx_parser.waiting_ack = true;

    uint8_t *send_buf = ts_malloc(data_len + 10);
    if (!send_buf) { return false; }
    uint16_t send_len = uart_encode_packet(send_buf, seq, cmd, data, data_len);

    for (uint8_t retry = 0; retry < max_retry; ++retry)
    {
        ack_received = false;
        ack_seq = 0;

        uart_senddata(send_buf, send_len);

        // uint32_t tick_start = os_sys_tick_get();
        // while (((os_sys_tick_get() - tick_start) * 1000 / os_sys_tick_rate_get()) < timeout_ms) {
        //     if (ack_received && ack_seq == seq) {
        //         break;
        //     }
        //     platform_delay_ms(1); // 1ms
        // }
    }
    ts_free(send_buf);
    return true;
}

void handle_retrans_packet(uint16_t seq)
{
    // User-defined retransmission handling logic
}


void handle_user_command(uint8_t cmd, uint16_t seq, const uint8_t *data, uint16_t data_len)
{
    uint8_t ack_status = 0; // 0: OK, non-zero: error code
    uint8_t ack_buf[32] = {0}; // used to return status + data
    uint16_t ack_data_len = 1; // default returns status only

    switch (cmd)
    {
    case UART_CMD_GET_DEVICE_NAME:
        {
            DBG_DIRECT("Handle: Get Device Name (0x40), seq=%d", seq);
            const char *dev_name = "RTL87x2G-Device";
            ack_status = 0;
            ack_buf[0] = ack_status;
            // Append payload data
            size_t name_len = strlen(dev_name);
            if (name_len > sizeof(ack_buf) - 1) { name_len = sizeof(ack_buf) - 1; }
            memcpy(&ack_buf[1], dev_name, name_len);
            ack_data_len = 1 + name_len;
            uart_send_ack(seq, UART_CMD_ACK, ack_buf, ack_data_len);
            break;
        }
    case UART_CMD_GET_MEM_SIZE:
        {
            DBG_DIRECT("Handle: Get Memory Size (0x41), seq=%d", seq);
            uint32_t mem_size = configTOTAL_psRAM_HEAP_SIZE; // e.g. 1MB
            ack_status = 0;
            ack_buf[0] = ack_status;
            memcpy(&ack_buf[1], &mem_size, sizeof(mem_size));
            ack_data_len = 1 + sizeof(mem_size);
            uart_send_ack(seq, UART_CMD_ACK, ack_buf, ack_data_len);
            break;
        }
    case UART_CMD_LOAD_MODEL:
        DBG_DIRECT("Handle: Load Model (0x42), seq=%d, data_len=%d", seq, data_len);
        // TODO: handle model loading
        ack_status = 0;
        ack_buf[0] = ack_status;
        ack_data_len = 1;
        uart_send_ack(uart_rx_parser.seq, UART_CMD_ACK, ack_buf, ack_data_len);
        break;
    case UART_CMD_LOAD_DATA:
        DBG_DIRECT("Handle: Load Data (0x43), seq=%d, data_len=%d", seq, data_len);
        // TODO: handle data loading
        ack_status = 0;
        ack_buf[0] = ack_status;
        ack_data_len = 1;
        uart_send_ack(uart_rx_parser.seq, UART_CMD_ACK, ack_buf, ack_data_len);
        break;
    case UART_CMD_CONFIG_PARAM:
        DBG_DIRECT("Handle: Config Param (0x44), seq=%d, data_len=%d", seq, data_len);
        ack_status = 0;
        // [0x44][modelSize(4)][modelCRC(2)][dataSize(4)][dataCRC(2)] (all little-endian)
        if (data_len >= 12)
        {
            // Parse parameters
            uart_rx_parser.model_len = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
            uart_rx_parser.model_crc = data[4] | (data[5] << 8);
            uart_rx_parser.data_len  = data[6] | (data[7] << 8) | (data[8] << 16) | (data[9] << 24);
            uart_rx_parser.data_crc  = data[10] | (data[11] << 8);

            DBG_DIRECT("Model size: 0x%x, Model CRC: 0x%x", uart_rx_parser.model_len, uart_rx_parser.model_crc);
            DBG_DIRECT("Data size: 0x%x, Data CRC: 0x%x", uart_rx_parser.data_len, uart_rx_parser.data_crc);

            // Release old buffers (if any)
            if (uart_rx_parser.model_buf)
            {
                ts_free(uart_rx_parser.model_buf);
                uart_rx_parser.model_buf = NULL;
            }
            if (uart_rx_parser.data_buf)
            {
                ts_free(uart_rx_parser.data_buf);
                uart_rx_parser.data_buf = NULL;
            }
            // Dynamically allocate new buffers
            bool alloc_fail = false;
            if (uart_rx_parser.model_len > 0)
            {
                uart_rx_parser.model_buf = (uint8_t *)ts_malloc(uart_rx_parser.model_len);
                if (!uart_rx_parser.model_buf)
                {
                    DBG_DIRECT("model_buf malloc failed!");
                    alloc_fail = true;
                }
            }
            if (uart_rx_parser.data_len > 0)
            {
                uart_rx_parser.data_buf = (uint8_t *)ts_malloc(uart_rx_parser.data_len);
                if (!uart_rx_parser.data_buf)
                {
                    DBG_DIRECT("data_buf malloc failed!");
                    alloc_fail = true;
                }
            }
            if (alloc_fail)
            {
                /* Partial-alloc failure: if one buffer succeeded and the other
                 * failed, free the successful one so there is no leak. */
                if (uart_rx_parser.model_buf)
                {
                    ts_free(uart_rx_parser.model_buf);
                    uart_rx_parser.model_buf = NULL;
                }
                if (uart_rx_parser.data_buf)
                {
                    ts_free(uart_rx_parser.data_buf);
                    uart_rx_parser.data_buf = NULL;
                }
                ack_status = 1; // 1: malloc failed
                const char *err_msg = "malloc failed";
                ack_buf[0] = ack_status;
                size_t err_len = strlen(err_msg);
                if (err_len > sizeof(ack_buf) - 1) { err_len = sizeof(ack_buf) - 1; }
                memcpy(&ack_buf[1], err_msg, err_len);
                ack_data_len = 1 + err_len;
            }
            else
            {
                ack_buf[0] = ack_status;
                ack_data_len = 1;
            }
        }
        else
        {
            ack_status = 2; // 2: param too small
            const char *err_msg = "param too small";
            ack_buf[0] = ack_status;
            size_t err_len = strlen(err_msg);
            if (err_len > sizeof(ack_buf) - 1) { err_len = sizeof(ack_buf) - 1; }
            memcpy(&ack_buf[1], err_msg, err_len);
            ack_data_len = 1 + err_len;
            DBG_DIRECT("Config Param data_len too small: %d", data_len);
        }
        uart_send_ack(uart_rx_parser.seq, UART_CMD_ACK, ack_buf, ack_data_len);
        break;
    case UART_CMD_DOWNLOAD_MODEL_FILE:
        DBG_DIRECT("Handle: Download Model File (0x46), seq=%d, data_len=%d", seq, data_len);
        ack_status = 0;
        if (uart_rx_parser.model_buf && uart_rx_parser.model_len > 0 && data && data_len > 0)
        {
            // Assume packets are received in order; data_len cannot exceed model_len
            static uint32_t model_offset = 0;
            if (model_offset + data_len <= uart_rx_parser.model_len)
            {
                memcpy(uart_rx_parser.model_buf + model_offset, data, data_len);
                model_offset += data_len;
                ack_status = 0;
                // If reception is complete, CRC can be checked here
                if (model_offset == uart_rx_parser.model_len)
                {
                    // Validate CRC
                    uint16_t crc = uart_crc16(uart_rx_parser.model_buf, uart_rx_parser.model_len);
                    if (crc != uart_rx_parser.model_crc)
                    {
                        ack_status = 2; // CRC error
                        DBG_DIRECT("Model CRC check failed! calc=0x%x, expect=0x%x", crc, uart_rx_parser.model_crc);
                    }
                    else
                    {
                        DBG_DIRECT("Model CRC check OK!");
                    }
                    model_offset = 0; // reset offset
                }
            }
            else
            {
                ack_status = 1; // overflow
                DBG_DIRECT("Model file write overflow!");
                model_offset = 0; // reset offset
            }
        }
        else
        {
            ack_status = 3; // buffer not allocated
            DBG_DIRECT("Model buf not allocated or invalid param!");
        }
        ack_buf[0] = ack_status;
        ack_data_len = 1;
        uart_send_ack(seq, UART_CMD_ACK, ack_buf, ack_data_len);
        break;
    case UART_CMD_DOWNLOAD_DATA_FILE:
        DBG_DIRECT("Handle: Download Data File (0x47), seq=%d, data_len=%d", seq, data_len);
        ack_status = 0;
        if (uart_rx_parser.data_buf && uart_rx_parser.data_len > 0 && data && data_len > 0)
        {
            // Assume packets are received in order; data_len cannot exceed configured data length
            static uint32_t data_offset = 0;
            if (data_offset + data_len <= uart_rx_parser.data_len)
            {
                memcpy(uart_rx_parser.data_buf + data_offset, data, data_len);
                data_offset += data_len;
                ack_status = 0;
                // If reception is complete, CRC can be checked here
                if (data_offset == uart_rx_parser.data_len)
                {
                    uint16_t crc = uart_crc16(uart_rx_parser.data_buf, uart_rx_parser.data_len);
                    if (crc != uart_rx_parser.data_crc)
                    {
                        ack_status = 2; // CRC error
                        DBG_DIRECT("Data CRC check failed! calc=0x%x, expect=0x%x", crc, uart_rx_parser.data_crc);
                    }
                    else
                    {
                        DBG_DIRECT("Data CRC check OK!");
                    }
                    data_offset = 0; // reset offset
                }
            }
            else
            {
                ack_status = 1; // overflow
                DBG_DIRECT("Data file write overflow!");
                data_offset = 0; // reset offset
            }
        }
        else
        {
            ack_status = 3; // buffer not allocated
            DBG_DIRECT("Data buf not allocated or invalid param!");
        }
        ack_buf[0] = ack_status;
        ack_data_len = 1;
        uart_send_ack(seq, UART_CMD_ACK, ack_buf, ack_data_len);
        break;
    case UART_CMD_MODEL_INFER:
        DBG_DIRECT("Handle: Model Infer (0x45), seq=%d", seq);
        ack_status = 0;
        ack_buf[0] = ack_status;
        ack_data_len = 1;
        uart_send_ack(uart_rx_parser.seq, UART_CMD_ACK, ack_buf, ack_data_len);
        // Call inference function and get return value
        // Use model_len as the model length parameter if needed
        // Example: MinimalInferenceWithTime(uart_rx_parser.model_buf, uart_rx_parser.data_buf, uart_rx_parser.data_len);
        // If model length is needed, use uart_rx_parser.model_len here


        int infer_ret = MinimalInferenceWithTime(uart_rx_parser.model_buf/*get_model_pointer()*/,
                                                 uart_rx_parser.model_len,
                                                 uart_rx_parser.data_buf/*(float*)input_data*/,
                                                 uart_rx_parser.data_len/*input_data_len*/);
        (void)infer_ret;

        // uart_rx_parser.model_len can be used for model-length related handling
        // Example: DBG_DIRECT("Model buffer length: %u", uart_rx_parser.model_len);
        // ack_status = (uint8_t)infer_ret;
        // ack_buf[0] = ack_status;
        // ack_data_len = 1;
        // uart_send_ack(uart_rx_parser.seq, UART_CMD_ACK, ack_buf, ack_data_len);

        // Send inference result
        extern void tf_result_buffer_send_all(void);
        tf_result_buffer_send_all();

        PrintUniqueOperators(uart_rx_parser.model_buf);
        extern void tf_operator_buffer_send_all(void);
        tf_operator_buffer_send_all();

        void tf_used_ram_event_upload();
        tf_used_ram_event_upload();

        // Example call: get Config_info string and upload
        char quant_info_buf[192];
        extern void GetQuantConfigInfoString(const char *model_buf, char *out_buf, size_t bufsize);
        GetQuantConfigInfoString((const char *)uart_rx_parser.model_buf, quant_info_buf,
                                 sizeof(quant_info_buf));
        tf_quant_param_buffer_send_all();

        tf_profiler_event_upload(uart_rx_parser.model_buf/*get_model_pointer()*/,
                                 uart_rx_parser.data_buf/*(float*)input_data*/,
                                 uart_rx_parser.data_len/*input_data_len*/);
        tf_profiler_buffer_send_all();
        break;
    default:
        DBG_DIRECT("Unknown user command: 0x%02x, seq=%d", cmd, seq);
        ack_status = 0x00; // unknown command
        ack_buf[0] = ack_status;
        ack_data_len = 1;
        uart_send_ack(seq, UART_CMD_ACK, ack_buf, ack_data_len);
        break;
    }
}


void tf_result_buffer_add(const char *str)
{
    if (!str) { return; }
    // Free previous result to avoid double-free
    if (uart_tx_parser.result_buffer)
    {
        DBG_DIRECT("tf_result_buffer_add: free old buffer");
        ts_free(uart_tx_parser.result_buffer);
        uart_tx_parser.result_buffer = NULL;
        uart_tx_parser.result_buf_size = 0;
        uart_tx_parser.result_count = 0;
    }
    uint16_t str_len = strlen(str) + 1;
    // Limit max length to prevent out-of-bounds
    if (str_len > 256) { str_len = 256; }
    uart_tx_parser.result_buffer = (char *)ts_malloc(str_len);
    if (!uart_tx_parser.result_buffer)
    {
        DBG_DIRECT("tf_result_buffer_add: ts_malloc failed");
        return;
    }
    memcpy(uart_tx_parser.result_buffer, str, str_len);
    uart_tx_parser.result_buf_size = str_len;
    uart_tx_parser.result_count = 1;
    DBG_DIRECT("tf_result_buffer_add: stored result, len=%d", str_len);
}

void tf_result_buffer_send_all(void)
{
    if (!uart_tx_parser.result_buffer || uart_tx_parser.result_buf_size == 0)
    {
        uart_tx_parser.result_count = 0;
        return;
    }
    DBG_DIRECT("tf_result_buffer_send_all: send result %s", uart_tx_parser.result_buffer);
    uart_send_with_ack(uart_tx_parser.seq++, UART_EVENT_REPORT_INFER_RESULT,
                       (const uint8_t *)uart_tx_parser.result_buffer,
                       strlen(uart_tx_parser.result_buffer), 1000, 1);

    // Added: upload inference time string
    if (uart_tx_parser.inference_time_buffer && uart_tx_parser.inference_time_buf_size > 0)
    {
        uart_send_with_ack(uart_tx_parser.seq++, UART_EVENT_REPORT_COST_TIME,
                           (const uint8_t *)uart_tx_parser.inference_time_buffer,
                           strlen(uart_tx_parser.inference_time_buffer), 1000, 1);
        ts_free(uart_tx_parser.inference_time_buffer);
        uart_tx_parser.inference_time_buffer = NULL;
        uart_tx_parser.inference_time_buf_size = 0;
    }

    ts_free(uart_tx_parser.result_buffer);
    uart_tx_parser.result_buffer = NULL;
    uart_tx_parser.result_buf_size = 0;
    uart_tx_parser.result_count = 0;
}

void tf_operator_buffer_add(const char *str)
{
    if (!str) { return; }
    // Free previous result to avoid double-free
    if (uart_tx_parser.operator_buffer)
    {
        ts_free(uart_tx_parser.operator_buffer);
        uart_tx_parser.operator_buffer = NULL;
        uart_tx_parser.operator_buf_size = 0;
    }
    uint16_t str_len = strlen(str) + 1;
    if (str_len > 256) { str_len = 256; }
    uart_tx_parser.operator_buffer = (char *)ts_malloc(str_len);
    if (!uart_tx_parser.operator_buffer) { return; }
    memcpy(uart_tx_parser.operator_buffer, str, str_len);
    uart_tx_parser.operator_buf_size = str_len;
}

void tf_operator_buffer_send_all(void)
{
    if (!uart_tx_parser.operator_buffer || uart_tx_parser.operator_buf_size == 0)
    {
        return;
    }
    uart_send_with_ack(uart_tx_parser.seq++, UART_EVENT_REPORT_OPERATOR,
                       (const uint8_t *)uart_tx_parser.operator_buffer,
                       strlen(uart_tx_parser.operator_buffer), 1000, 1);

    ts_free(uart_tx_parser.operator_buffer);
    uart_tx_parser.operator_buffer = NULL;
    uart_tx_parser.operator_buf_size = 0;
}

void tf_quant_param_buffer_add(const char *str)
{
    if (!str) { return; }
    // Free previous result to avoid double-free
    if (uart_tx_parser.quant_param_buffer)
    {
        ts_free(uart_tx_parser.quant_param_buffer);
        uart_tx_parser.quant_param_buffer = NULL;
        uart_tx_parser.quant_param_buf_size = 0;
    }
    uint16_t str_len = strlen(str) + 1;
    if (str_len > 256) { str_len = 256; }
    uart_tx_parser.quant_param_buffer = (char *)ts_malloc(str_len);
    if (!uart_tx_parser.quant_param_buffer) { return; }
    memcpy(uart_tx_parser.quant_param_buffer, str, str_len);
    uart_tx_parser.quant_param_buf_size = str_len;
}

void tf_quant_param_buffer_send_all(void)
{
    if (!uart_tx_parser.quant_param_buffer || uart_tx_parser.quant_param_buf_size == 0)
    {
        return;
    }
    uart_send_with_ack(uart_tx_parser.seq++, UART_EVENT_REPORT_QUANT_PARAM,
                       (const uint8_t *)uart_tx_parser.quant_param_buffer,
                       strlen(uart_tx_parser.quant_param_buffer), 1000, 1);

    ts_free(uart_tx_parser.quant_param_buffer);
    uart_tx_parser.quant_param_buffer = NULL;
    uart_tx_parser.quant_param_buf_size = 0;
}

void tf_used_ram_event_upload()
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%d Bytes", uart_tx_parser.arena_used_bytes);

    uart_send_with_ack(uart_tx_parser.seq++, UART_EVENT_REPORT_USED_RAM,
                       (const uint8_t *)buf, strlen(buf), 1000, 1);
}


void tf_profiler_buffer_add(const char *str)
{
    if (!str) { return; }
    if (uart_tx_parser.profiler_buffer)
    {
        ts_free(uart_tx_parser.profiler_buffer);
        uart_tx_parser.profiler_buffer = NULL;
        uart_tx_parser.profiler_buf_size = 0;
    }
    uint16_t str_len = strlen(str) + 1;
    if (str_len > 10240) { str_len = 10240; } // limit maximum length to 10240 bytes
    if (str_len == 0) { return; } // ignore empty string
    uart_tx_parser.profiler_buffer = (char *)ts_malloc(str_len);
    if (!uart_tx_parser.profiler_buffer) { return; }
    memcpy(uart_tx_parser.profiler_buffer, str, str_len);
    uart_tx_parser.profiler_buf_size = str_len;
}

void tf_profiler_buffer_send_all(void)
{
    if (!uart_tx_parser.profiler_buffer || uart_tx_parser.profiler_buf_size == 0)
    {
        return;
    }
    uart_send_with_ack(uart_tx_parser.seq++, UART_EVENT_REPORT_PROFILER,
                       (const uint8_t *)uart_tx_parser.profiler_buffer,
                       strlen(uart_tx_parser.profiler_buffer), 1000, 1);
    ts_free(uart_tx_parser.profiler_buffer);
    uart_tx_parser.profiler_buffer = NULL;
    uart_tx_parser.profiler_buf_size = 0;
}


