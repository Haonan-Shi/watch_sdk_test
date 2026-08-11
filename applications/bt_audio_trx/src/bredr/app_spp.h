/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_SPP_H_
#define _APP_SPP_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum
{
    SPP_SEND_OK             = 0x00,
    SPP_PROFILE_NOT_CONN    = 0x01,
    SPP_NO_RFC_CREDIT       = 0x02,
    SPP_SEND_FAIL           = 0x03,
};

/** @defgroup APP_SPP App Spp
  * @brief App Spp
  * @{
  */
/**
    * @brief  spp send transfer data
    * @param  app_idx,data ptr, data len
    * @return send result
    */
uint8_t app_spp_transfer(uint8_t app_idx, uint8_t *data, uint32_t len);

/**
    * @brief  spp module init
    * @param  void
    * @return void
    */
void app_spp_init(void);


/** End of APP_SPP
* @}
*/


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _APP_SPP_H_ */
