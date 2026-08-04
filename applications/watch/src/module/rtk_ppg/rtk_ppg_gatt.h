/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _RTK_PPG_GATT_H_
#define _RTK_PPG_GATT_H_

#ifdef  __cplusplus
extern "C" {
#endif      /* __cplusplus */

#include "bt_gatt_svc.h"

extern T_SERVER_ID hrs_gatt_srv_id;

void rtk_ppg_gatt_init(void);

#ifdef  __cplusplus
}
#endif      /* __cplusplus */

#endif /* _RTK_PPG_GATT_H_ */