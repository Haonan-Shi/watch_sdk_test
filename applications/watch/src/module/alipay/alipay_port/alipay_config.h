/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _ALIPAY_CONFIG_H_
#define _ALIPAY_CONFIG_H_

#if CONFIG_ALIPAY


/*************************************
*          Log Config
*************************************/
#include "trace.h"
#include "alipay_mem.h"
#define AliPay_LOG(format, ...)  DBG_DIRECT(format, ##__VA_ARGS__)

/*************************************
*          HSC32L1 Config
*************************************/

#define ALIPAY_I2Cx   I2C2

#define  I2C_SLAVE_ADDR  (0xC8>>1)

#define  I2C_RESET_PIN  P3_0

#define  I2C_CLK_PIN    P1_0
#define  I2C_SDA_PIN    P1_1

/*************************************
*          PSRAM Heap Config
*************************************/
//#define  ALIPAY_PSRAM_ADDR  (PSRAM_GUI_HEAP_ADDR + PSRAM_GUI_HEAP_SIZE)

//#define  ALIPAY_PSRAM_SIZE    (1024 * 1024)

/*************************************
*          Mbedtls Memory Config
*************************************/
//#include "iotsec.h"
//#define mbedtls_free             csi_free
//#define mbedtls_calloc           csi_calloc

/*************************************
*          QuickJS Memory Config
*************************************/
//#include "iotsec.h"
#define quickjs_getUsedSize      csi_getUsedSize
#define quickjs_free             csi_free
#define quickjs_realloc          csi_realloc
#define quickjs_malloc           csi_malloc

/*************************************
*          Alipay BT Address
*************************************/

#define ALIPAY_BT_MAC_RAM_ARRD     (0x20004644)

#else


/*************************************
*          Mbedtls memory Config
*************************************/

#define mbedtls_free             //csi_free
#define mbedtls_calloc           //csi_calloc

/*************************************
*          QuickJS Memory Config
*************************************/
#include "iotsec.h"
#define quickjs_getUsedSize      //csi_getUsedSize
#define quickjs_free             //csi_free
#define quickjs_realloc          //csi_realloc
#define quickjs_malloc           //csi_malloc

#endif // CONFIG_ALIPAY

#endif // _ALIPAY_CONFIG_H_
