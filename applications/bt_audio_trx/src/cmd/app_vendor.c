/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <string.h>
#include <stdlib.h>
#include "gap.h"
#include "btm.h"
#include "trace.h"
#include "gap_vendor.h"
#include "app_vendor.h"
#include "app_cmd.h"

#define MP_CMD_HCI_OPCODE   0xFCEB

void app_vendor_rf_xtak_k(uint8_t channel, uint8_t upperbound, uint8_t lowerbound,
                          uint8_t offset)
{
    uint8_t params[6];

    params[0] = 0x06; //Module ID, MODULE_XTAL = 0x06
    params[1] = 0x00; //Subcmd, AUTO_K = 0x00

    /* cmd params */
    params[2] = channel;
    params[3] = upperbound;
    params[4] = lowerbound;
    params[5] = offset;

    gap_vendor_cmd_req(MP_CMD_HCI_OPCODE, sizeof(params), params);
}

void app_vendor_get_xtak_k_result(void)
{
    uint8_t params[4];

    params[0] = 0x06; //Module ID, MODULE_XTAL = 0x06
    params[1] = 0x01; //Subcmd, XTAL_VALUE = 0x01

    /* cmd params */
    params[2] = 0x0;  //Type, VALUE_GET = 0x0
    params[3] = 0x0;  //Value, this param would be ignored when type is get.

    gap_vendor_cmd_req(MP_CMD_HCI_OPCODE, sizeof(params), params);
}

void app_vendor_write_xtak_k_result(uint8_t xtal_val)
{
#if CONFIG_SOC_SERIES_RTL8773D || TARGET_RTL8773DFL
    /*Write to sys cfg*/
    uint8_t params[17];

    //Only For RTL87x3D
    params[0] = 0x00; //Module ID, PLATFORM
    params[1] = 0x02; //Subcmd, CFG_REVISE

    /* cmd params */
    params[2] = 0x02; //config number

    /*XTAL_SC_XI*/
    params[3] = 0x01; //Moudle ID L
    params[4] = 0x00; //Moudle ID H
    params[5] = 0x25; //offset L
    params[6] = 0x00; //offset H
    params[7] = 0x01; //len
    params[8] = xtal_val;//data
    params[9] = 0xFF; //mask

    /*XTAL_SC_XO*/
    params[10] = 0x01; //Moudle ID L
    params[11] = 0x00; //Moudle ID H
    params[12] = 0x26; //offset L
    params[13] = 0x00; //offset H
    params[14] = 0x01; //len
    params[15] = xtal_val;//data
    params[16] = 0xFF; //mask

    gap_vendor_cmd_req(MP_CMD_HCI_OPCODE, sizeof(params), params);
#endif
}
