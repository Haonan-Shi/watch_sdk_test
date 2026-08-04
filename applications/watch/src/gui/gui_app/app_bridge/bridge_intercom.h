/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *               Define to prevent recursive inclusion
 *============================================================================*/
#ifndef _BRIDGE_INTERCOM_H_
#define _BRIDGE_INTERCOM_H_

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include <stdint.h>
#include <stdbool.h>

/*============================================================================*
 *                         Macros
 *============================================================================*/

/*gui topic*/

/*app topic*/

#define EVENT_BUG_TOPIC_INTERCOM_ALL_TOPIC             "intc/*"
#define EVENT_BUS_TOPIC_INTERCOM_GUI_ON                "intc/gui/on"
#define EVENT_BUS_TOPIC_INTERCOM_GUI_OFF               "intc/gui/off"
#define EVENT_BUS_TOPIC_INTERCOM_GUI_CONNECT_DEV1      "intc/gui/conn_dev1"
#define EVENT_BUS_TOPIC_INTERCOM_GUI_CONNECT_DEV2      "intc/gui/conn_dev2"
#define EVENT_BUS_TOPIC_INTERCOM_GUI_CONNECT_DEV3      "intc/gui/conn_dev3"
#define EVENT_BUS_TOPIC_INTERCOM_GUI_DISCONNECT        "intc/gui/disconn"
#define EVENT_BUS_TOPIC_INTERCOM_TRANSMIT_START        "intc/gui/tran_start"
#define EVENT_BUS_TOPIC_INTERCOM_TRANSMIT_STOP         "intc/gui/tran_stop"


/*============================================================================*
 *                         Types
 *============================================================================*/


/*============================================================================*
 *                         Constants
 *============================================================================*/


/*============================================================================*
 *                         Variables
 *============================================================================*/


/*============================================================================*
 *                         Functions
 *============================================================================*/

/**
* @brief Send recording command or data from GUI to App.
*
* @param  topic The topic of the message.
* @param  data Pointer to the data to be sent.
* @param  size Size of the data to be sent.
* @return true if the message was sent successfully, false otherwise.
*/

bool intercom_gui_to_app(const char *topic, void *data, uint32_t size);
/**
 * @brief Initialize the recording bridge, including creating topics and subscribing to events.
 *        This function should be called in bridge_module_init().
 */
void bridge_intercom_init(void);

#ifdef __cplusplus
}
#endif

#endif /* _BRIDGE_INTERCOM_H_ */