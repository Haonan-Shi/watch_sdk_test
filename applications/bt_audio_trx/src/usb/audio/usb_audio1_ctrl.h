/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef __USB_AUDIO1_CTRL_H_
#define __USB_AUDIO1_CTRL_H_
#include <stdint.h>

/** @defgroup APP_USB APP USB
  * @brief app usb module.
  * @{
  */

/** @defgroup USB_AUDIO_IF USB Audio
  * @brief USB Audio interface
  * @{
  */


typedef int32_t (*UAC_KITS_FUNC_INT)();

/**
 * usb_audio1_ctrl.h
 *
 * \brief   USB Audio 1.0 attribute type, refer to USB Audio1.0 spec
 *
 * \ingroup USB_AUDIO_IF
 */
#define UAC1_CUR                        0x1
#define UAC1_MIN                        0x2
#define UAC1_MAX                        0x3
#define UAC1_RES                        0x4
#define UAC1_MEM                        0x5

/**
 * usb_audio1_ctrl.h
 *
 * \brief   USB Audio 1.0 attribute value, refer to USB Audio1.0 spec
 *
 * \ingroup USB_AUDIO_IF
 */
typedef union ctrl_attr
{
    int32_t    data[5];
    struct
    {
        int32_t    cur;
        int32_t    min;
        int32_t    max;
        int32_t    res;
        int32_t    mem;
    } attr;
} T_CTRL_ATTR;

/**
 * usb_audio1_ctrl.h
 *
 * \brief   USB Audio 1.0 attribute get/set cbs
 *
 * \ingroup USB_AUDIO_IF
 */
typedef struct ctrl_func
{
    UAC_KITS_FUNC_INT   set;
    UAC_KITS_FUNC_INT   get;
} T_CTRL_FUNC;

/**
 * usb_audio1_ctrl.h
 *
 * \brief   USB Audio 1.0 attribute control structure
 *
 * \param name control name for debug
 * \param type control type defined in USB Audio1.0 spec
 * \param attr actual attribute value
 * \param func callbacks to get/set attribute
 *
 * \ingroup USB_AUDIO_IF
 */
typedef struct _usb_audio1_ctrl
{
    const char  *name;
    uint8_t     type;
    T_CTRL_ATTR *attr;
    T_CTRL_FUNC func;
} T_USB_AUDIO1_CTRL;

/**
 * usb_audio1_ctrl.h
 *
 * \brief   USB Audio 1.0 attribute control set
 *
 * \param id id of corresponding control uint
 * \param name control set name for debug
 * \param type control type defined in USB Audio1.0 spec
 * \param desc descriptor of corresponding control uint
 * \param ctrl \ref T_USB_AUDIO1_CTRL
 *
 * \ingroup USB_AUDIO_IF
 */
typedef struct _usb_audio1_ctrl_kits
{

    uint8_t             id;
    const char          *name;
    uint8_t             type;
    void                *desc;
    const T_USB_AUDIO1_CTRL    *ctrl[3];
} T_USB_AUDIO1_CTRL_KITS;

/** @}*/
/** End of USB_AUDIO_IF
*/

/** @}*/
/** End of APP_USB
*/
#endif
