/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef UART_PACKET_PARSER_H
#define UART_PACKET_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define UART_FRAME_HEAD1 0xAA
#define UART_FRAME_HEAD2 0xBB
#define UART_FRAME_TAIL  0xCC
#define UART_RX_BUF_SIZE 4096

#define UART_CMD_ACK      0x80  // ACK command byte
#define UART_CMD_RETRANS  0x81  // Retransmit command byte

// Standard command definitions
#define UART_CMD_GET_DEVICE_NAME   0x40
#define UART_CMD_GET_MEM_SIZE      0x41
#define UART_CMD_LOAD_MODEL        0x42
#define UART_CMD_LOAD_DATA         0x43
#define UART_CMD_CONFIG_PARAM      0x44
#define UART_CMD_MODEL_INFER       0x45
#define UART_CMD_DOWNLOAD_MODEL_FILE     0x46
#define UART_CMD_DOWNLOAD_DATA_FILE      0x47

#define UART_EVENT_REPORT_USED_RAM       0x50
#define UART_EVENT_REPORT_COST_TIME      0x51
#define UART_EVENT_REPORT_OPERATOR       0x52
#define UART_EVENT_REPORT_QUANT_PARAM    0x53
#define UART_EVENT_REPORT_MODEL_INFO     0x54
#define UART_EVENT_REPORT_INFER_RESULT   0x55
#define UART_EVENT_REPORT_PROFILER       0x56

typedef enum
{
    UART_RX_STATE_IDLE,
    UART_RX_STATE_HEAD1,
    UART_RX_STATE_HEAD2,
    UART_RX_STATE_LEN,
    UART_RX_STATE_SEQ1,
    UART_RX_STATE_SEQ2,
    UART_RX_STATE_DATA,
    UART_RX_STATE_CRC1,
    UART_RX_STATE_CRC2,
    UART_RX_STATE_TAIL
} uart_rx_state_t;

typedef struct
{
    uint16_t seq;           // TX sequence number
    uint8_t cmd;            // TX command
    const uint8_t *data;    // TX data pointer
    uint16_t data_len;      // TX data length
    uint8_t retry_count;    // Retry count so far
    uint8_t max_retry;      // Maximum retry count
    uint32_t timeout_ms;    // ACK timeout in ms
    bool waiting_ack;       // Whether waiting for ACK
    char *result_buffer;    // Dynamic inference result buffer
    uint16_t result_buf_size; // Current buffer size
    uint16_t result_count;    // Number of results currently stored
    char *operator_buffer;     // Dedicated buffer for operator upload
    uint16_t operator_buf_size;// operator buffer size
    char *quant_param_buffer;     // Dedicated buffer for quant param upload
    uint16_t quant_param_buf_size;// quant param buffer size
    char *profiler_buffer;     // Dedicated buffer for profiler upload
    uint16_t profiler_buf_size;// profiler buffer size
    int arena_used_bytes;      // stores arena_used_bytes
    char *inference_time_buffer; // inference time string buffer
    uint16_t inference_time_buf_size;

} uart_tx_parser_t;

extern uart_tx_parser_t uart_tx_parser;

void uart_rx_byte_parse(uint8_t byte);
void uart_rx_data_parse(const uint8_t *data, uint16_t len);
uint16_t uart_encode_packet(uint8_t *out_buf, uint16_t seq, uint8_t cmd, const uint8_t *data,
                            uint16_t data_len);

// User must implement this callback to handle a complete packet
void process_uart_packet(const uint8_t *buf, uint16_t len);
void handle_ack_packet(uint16_t seq);
void handle_retrans_packet(uint16_t seq);
void handle_user_command(uint8_t cmd, uint16_t seq, const uint8_t *data, uint16_t data_len);
// Actively send data and wait for ACK, supports retransmission; returns true when ACK is received
bool uart_send_with_ack(uint16_t seq, uint8_t cmd, const uint8_t *data, uint16_t data_len,
                        uint32_t timeout_ms, uint8_t max_retry);

#ifdef __cplusplus
}
#endif

#endif // UART_PACKET_PARSER_H
