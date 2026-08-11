/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if CONFIG_REALTEK_GFPS_FEATURE_SUPPORT
/*============================================================================*
 *               Define to prevent recursive inclusion
 *============================================================================*/
#ifndef APP_ECC_H
#define APP_ECC_H

#ifdef __cplusplus
extern "C"
{
#endif

#define EVENT_BUS_TOPIC_ECC_MSG_RELAY   "ecc/relay"

/*============================================================================*
 *                              Functions
 *============================================================================*/
/** @defgroup APP_ECC_Exported_Functions
    * @{
    */

void app_ecc_handle_msg(void);

/** @} */ /* End of group APP_ECC_Exported_Functions */

#ifdef __cplusplus
}
#endif

#endif /* GAP_MSG_H */
#endif
