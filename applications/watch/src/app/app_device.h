/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_DEVICE_H_
#define _APP_DEVICE_H_
#include <stdbool.h>
#include <stdint.h>
#include "rtl876x_wdg.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/** @defgroup APP_DEVICE App Device
  * @brief App Device
  * @{
  */
/*============================================================================*
 *                              Macros
 *============================================================================*/
/** @defgroup APP_DEVICE_Exported_Macros App Device Macros
    * @{
    */

/** End of APP_DEVICE_Exported_Macros
    * @}
    */

/*============================================================================*
 *                              Types
 *============================================================================*/
/** @defgroup APP_DEVICE_Exported_Types App Device Types
    * @{
    */
typedef enum
{
    APP_DEVICE_TIMER_POWER_OFF_RESET,
    APP_DEVICE_TIMER_TOTAL
} T_APP_DEVICE_TIMER;

typedef enum
{
    BUD_COUPLE_STATE_IDLE      = 0x00,
    BUD_COUPLE_STATE_START     = 0x01,
    BUD_COUPLE_STATE_CONNECTED = 0x02
} T_BUD_COUPLE_STATE;

typedef enum
{
    APP_DEVICE_STATE_OFF       = 0x00,
    APP_DEVICE_STATE_ON        = 0x01,
    APP_DEVICE_STATE_OFF_ING   = 0x02,
} T_APP_DEVICE_STATE;

typedef enum
{
    AUTO_POWER_OFF_MASK_POWER_ON = 0x01,
    AUTO_POWER_OFF_MASK_SOURCE_LINK = 0x02,
    AUTO_POWER_OFF_MASK_IN_BOX = 0x04,
    AUTO_POWER_OFF_MASK_BUD_COUPLING = 0x08,
    AUTO_POWER_OFF_MASK_KEY = 0x10,
    AUTO_POWER_OFF_MASK_ALREADY_POWER_OFF = 0x20,
    AUTO_POWER_OFF_MASK_PAIRING_MODE = 0x40,
    AUTO_POWER_OFF_MASK_ANC_APT_MODE = 0x80,
    AUTO_POWER_OFF_MASK_BLE_LINK_EXIST  = 0x00000100,
    AUTO_POWER_OFF_MASK_PLAYBACK_MODE   = 0x00000200,
} T_AUTO_POWER_OFF_MASK;


typedef enum
{
    APP_TONE_VP_STOP      = 0x00,
    APP_TONE_VP_STARTED     = 0x01,
} T_APP_TONE_VP_STATE;

typedef struct
{
    T_APP_TONE_VP_STATE state;
    uint8_t     index;
} T_APP_TONE_VP_STARTED;

/** End of APP_DEVICE_Exported_Types
    * @}
    */

/*============================================================================*
 *                              Functions
 *============================================================================*/
/** @defgroup APP_DEVICE_Exported_Functions App Device Functions
    * @{
    */

/** @} */ /* End of group APP_DEVICE_Exported_Functions */
/** End of APP_DEVICE
* @}
*/


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _APP_DEVICE_H_ */
