/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "rtl876x.h"
#include "trace.h"
#include "board.h"
#include "os_mem.h"
#include "app_msg.h"
#include "power_test.h"
#include "gap_vendor.h"

#define HCI_CMD_OPCODE_PLATFORM     0xFCEB
#define HCI_CMD_MODULE_ID_PHY       0x05
#define HCI_CMD_SUBCMD_TX_POWER     0x00
#define HCI_CMD_SUBCMD_CONT_TX      0x08
#define HCI_CMD_SUBCMD_PACKET_RX    0x0A

typedef enum
{
    PACKET_TYPE_1DH1 = 0x00,
    PACKET_TYPE_1DH3 = 0x01,
    PACKET_TYPE_1DH5 = 0x02,
    PACKET_TYPE_2DH1 = 0x03,
    PACKET_TYPE_2DH3 = 0x04,
    PACKET_TYPE_2DH5 = 0x05,
    PACKET_TYPE_3DH1 = 0x06,
    PACKET_TYPE_3DH3 = 0x07,
    PACKET_TYPE_3DH5 = 0x08,
    PACKET_TYPE_LE1M = 0x09,
    PACKET_TYPE_NULL = 0x0A,
    PACKET_TYPE_LE2M = 0x0B,
    PACKET_TYPE_LR8  = 0x0C,
    PACKET_TYPE_LR2  = 0x0D,
    PACKET_TYPE_1DM1 = 0x0E,
    PACKET_TYPE_1DM3 = 0x0F,
    PACKET_TYPE_1DM5 = 0x10,
    PACKET_TYPE_AUX1 = 0x11
} CONT_TX_PACKET_TYPE;

typedef enum
{
    PAYLOAD_PATTERN_ALL_0 = 0x00,
    PAYLOAD_PATTERN_ALL_1 = 0x01,
    PAYLOAD_PATTERN_REPEAT_01010101 = 0x02,
    PAYLOAD_PATTERN_REPEAT_10101010 = 0x03,
    PAYLOAD_PATTERN_REPEAT_0_TO_F = 0x04,
    PAYLOAD_PATTERN_REPEAT_00001111 = 0x05,
    PAYLOAD_PATTERN_REPEAT_11110000 = 0x06,
    PAYLOAD_PATTERN_PRBS9 = 0x07
} CONT_TX_PAYLOAD_PATTERN;
typedef struct t_hci_cmd_tx_power
{
    uint8_t module_id;            /* HCI_CMD_MODULE_ID_PHY */
    uint8_t subcmd;               /* HCI_CMD_SUBCMD_TX_POWER */
    uint8_t type;                 /* VALUE_GET = 0x00; VALUE_SET = 0x01 */
    uint8_t power_type;           /* TXGAIN_INDEX_TYPE = 0x00; TX_POWER_TYPE = 0x01 */
    uint8_t max_tx_power_br_1M;   /* unit 0.5dbm */
    uint8_t max_tx_power_edr_2M;  /* unit 0.5dbm */
    uint8_t max_tx_power_edr_3M;  /* unit 0.5dbm */
    uint8_t max_tx_power_le_1M;   /* unit 0.5dbm */
    uint8_t max_tx_power_le_2M;   /* unit 0.5dbm */
} T_HCI_CMD_TX_POWER;

typedef struct t_hci_cmd_cont_tx
{
    uint8_t module_id;             /* HCI_CMD_MODULE_ID_PHY */
    uint8_t subcmd;                /* HCI_CMD_SUBCMD_CONT_TX */
    uint8_t type;                  /* POUTER_STOP = 0x00; POUTER_START = 0x01 */
    uint8_t sync_word_1;           /* BT address */
    uint8_t sync_word_2;
    uint8_t sync_word_3;
    uint8_t sync_word_4;
    uint8_t sync_word_5;
    uint8_t sync_word_6;
    uint8_t sync_word_7;
    uint8_t sync_word_8;
    uint8_t rf_channel;            /* 0x00~0x4E */
    uint8_t power_type;            /* TX_POWER_TYPE = 0x01; TXGAIN_LEVEL_TYPE = 0x02 */
    uint8_t tx_power;
    uint8_t packet_type;           /* CONT_TX_PACKET_TYPE */
    uint8_t payload_pattern;       /* CONT_TX_PAYLOAD_PATTERN */
    uint8_t tx_packet_count_1;     /* tx packet count */
    uint8_t tx_packet_count_2;
    uint8_t tx_packet_count_3;
    uint8_t tx_packet_count_4;
    uint8_t is_whiten;             /* Whitening or not */
    uint8_t whitening_coeff;       /* If is_whiten disable, set 0x80, otherwise set 0x7f*/
} T_HCI_CMD_CONT_TX;

typedef struct t_hci_cmd_packet_rx
{
    uint8_t module_id;             /* HCI_CMD_MODULE_ID_PHY */
    uint8_t subcmd;                /* HCI_CMD_SUBCMD_PACKET_RX */
    uint8_t type;                  /* POUTER_STOP = 0x00; POUTER_START = 0x01 */
    uint8_t sync_word_1;           /* BT address */
    uint8_t sync_word_2;
    uint8_t sync_word_3;
    uint8_t sync_word_4;
    uint8_t sync_word_5;
    uint8_t sync_word_6;
    uint8_t sync_word_7;
    uint8_t sync_word_8;
    uint8_t rf_channel;            /* 0x00~0x4E */
    uint8_t power_type;            /* Ignored */
    uint8_t tx_power;              /* Ignored */
    uint8_t packet_type;           /* CONT_TX_PACKET_TYPE */
    uint8_t payload_pattern;       /* CONT_TX_PAYLOAD_PATTERN */
    uint8_t tx_packet_count_1;     /* Ignored */
    uint8_t tx_packet_count_2;
    uint8_t tx_packet_count_3;
    uint8_t tx_packet_count_4;
    uint8_t is_whiten;             /* Whitening or not */
    uint8_t whitening_coeff;       /* If is_whiten disable, set 0x80, otherwise set 0x7f*/
} T_HCI_CMD_PACKET_RX;

void power_test_set_tx_power(uint8_t br_1M, uint8_t edr_2M, uint8_t edr_3M, uint8_t le_1M,
                             uint8_t le_2M, uint8_t *buf)
{
    APP_PRINT_INFO5("power_test_set_tx_power %d %d %d %d %d", br_1M, edr_2M, edr_3M, le_1M, le_2M);
    T_HCI_CMD_TX_POWER tx_power = {0};

    tx_power.module_id = HCI_CMD_MODULE_ID_PHY;
    tx_power.subcmd = HCI_CMD_SUBCMD_TX_POWER;
    tx_power.type = 0x01;
    tx_power.power_type = 0x01;
    tx_power.max_tx_power_br_1M = br_1M;
    tx_power.max_tx_power_edr_2M = edr_2M;
    tx_power.max_tx_power_edr_3M = edr_3M;
    tx_power.max_tx_power_le_1M = le_1M;
    tx_power.max_tx_power_le_2M = le_2M;

    if (gap_vendor_cmd_req(HCI_CMD_OPCODE_PLATFORM, 9, (uint8_t *)(&tx_power)))
    {
        APP_PRINT_INFO0("tx power hci cmd success");
    }
    else
    {
        APP_PRINT_INFO0("tx power hci cmd failed");
    }

    if (buf != NULL)
    {
        free(buf);
    }
}

void power_test_cont_tx(uint8_t tx_power, uint8_t packet_type, uint8_t *buf)
{
    APP_PRINT_INFO2("power_test_cont_tx %d %d", tx_power, packet_type);
    T_HCI_CMD_CONT_TX cont_tx = {0};

    cont_tx.module_id = HCI_CMD_MODULE_ID_PHY;
    cont_tx.subcmd = HCI_CMD_SUBCMD_CONT_TX;
    cont_tx.type = 0x01;
    cont_tx.sync_word_1 = 0xCC;
    cont_tx.sync_word_2 = 0xB7;
    cont_tx.sync_word_3 = 0x99;
    cont_tx.sync_word_4 = 0x2D;
    cont_tx.sync_word_5 = 0x9A;
    cont_tx.sync_word_6 = 0x9F;
    cont_tx.sync_word_7 = 0xF2;
    cont_tx.sync_word_8 = 0x58;
    cont_tx.rf_channel = 0x00;
    cont_tx.power_type = 0x01;
    /* NOTE: this paramter only work when tx power control enabled
       TX_POWER_TYPE: 0x00~0xFF Signed integer, Unit: 0.5dbm
       TXGAIN_LEVEL_TYPE: 0x00~0x07 Unsigned integer */
    cont_tx.tx_power = tx_power;
    cont_tx.packet_type = packet_type;
    cont_tx.payload_pattern = PAYLOAD_PATTERN_ALL_1;
    cont_tx.tx_packet_count_1 = 0xFF;
    cont_tx.tx_packet_count_2 = 0xFF;
    cont_tx.tx_packet_count_3 = 0xFF;
    cont_tx.tx_packet_count_4 = 0xFF;
    cont_tx.is_whiten = 0x00;
    cont_tx.whitening_coeff = 0x80;

    if (gap_vendor_cmd_req(HCI_CMD_OPCODE_PLATFORM, 0x16, (uint8_t *)(&cont_tx)))
    {
        APP_PRINT_INFO0("cont tx hci cmd success");
    }
    else
    {
        APP_PRINT_INFO0("cont tx hci cmd failed");
    }

    if (buf != NULL)
    {
        free(buf);
    }
}

void power_test_packet_rx(uint8_t packet_type, uint8_t *buf)
{
    APP_PRINT_INFO1("power_test_packet_rx %d %d", packet_type);
    T_HCI_CMD_PACKET_RX packet_rx = {0};

    packet_rx.module_id = HCI_CMD_MODULE_ID_PHY;
    packet_rx.subcmd = HCI_CMD_SUBCMD_PACKET_RX;
    packet_rx.type = 0x01;
    packet_rx.sync_word_1 = 0xCC;
    packet_rx.sync_word_2 = 0xB7;
    packet_rx.sync_word_3 = 0x99;
    packet_rx.sync_word_4 = 0x2D;
    packet_rx.sync_word_5 = 0x9A;
    packet_rx.sync_word_6 = 0x9F;
    packet_rx.sync_word_7 = 0xF2;
    packet_rx.sync_word_8 = 0x58;
    packet_rx.rf_channel = 0x00;
    packet_rx.power_type = 0x01;
    packet_rx.tx_power = 12;
    packet_rx.packet_type = packet_type;
    packet_rx.payload_pattern = PAYLOAD_PATTERN_ALL_1;
    packet_rx.tx_packet_count_1 = 0xFF;
    packet_rx.tx_packet_count_2 = 0xFF;
    packet_rx.tx_packet_count_3 = 0xFF;
    packet_rx.tx_packet_count_4 = 0xFF;
    packet_rx.is_whiten = 0x00;
    packet_rx.whitening_coeff = 0x80;

    if (gap_vendor_cmd_req(HCI_CMD_OPCODE_PLATFORM, 0x16, (uint8_t *)(&packet_rx)))
    {
        APP_PRINT_INFO0("packet rx hci cmd success");
    }
    else
    {
        APP_PRINT_INFO0("packet rx hci cmd failed");
    }

    if (buf != NULL)
    {
        free(buf);
    }
}

