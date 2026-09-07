/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if F_APP_USB_HID_SUPPORT
#include <stdint.h>
#include "trace.h"
#include "usb_hid_desc.h"
#include "usb_hid.h"
#include "app_usb_hid.h"
#include "string.h"

static void *hid_mmi_handle = NULL;

#if USB_HID_MOUSE_EN
void app_usb_mouse_handle_action(uint8_t action)
{
    USB_HID_MOUSE_INPUT_REPORT mouse_report = {0};

    int ret = 0;
    static int count_y = 0;

    if (hid_mmi_handle == NULL)
    {
        T_USB_HID_ATTR attr =
        {
            .zlp = 1,
            .high_throughput = 0,
            .congestion_ctrl = HID_CONGESTION_CTRL_DROP_CUR,
            .rsv = 0,
            .mtu = 64
        };
        hid_mmi_handle = usb_hid_data_pipe_open(HID_INT_IN_EP_1, attr, HID_MAX_PENDING_REQ_NUM, NULL);
    }

    APP_PRINT_INFO1("app_usb_mouse_handle_action, action %x", action);

    switch (action)
    {
    case MMI_MOUSE_UP:
        {
            mouse_report.y = 0xE0;   // highest bit 1 means up
        }
        break;

    case MMI_MOUSE_DWON:
        {
            mouse_report.y = 0x20;
        }
        break;

    case MMI_MOUSE_LEFT:
        {
            mouse_report.x = 0xE0;
        }
        break;

    case MMI_MOUSE_RIGHT:
        {
            mouse_report.x = 0x20;
        }
        break;

    default:
        {
            return;
        }
    }

    usb_hid_mouse_param_send(hid_mmi_handle, MOUSE_REPORT_ID, &mouse_report, 5);
}
#endif

static int8_t app_usb_hid_set_report_handle(T_HID_DRIVER_REPORT_REQ_VAL req_value, uint8_t *data,
                                            uint16_t length)
{
    APP_PRINT_TRACE2("app_usb_hid_set_report_handle id 0x%x, data0 0x%x", req_value.id, data[0]);
    // TO DO
    return 0;
}

static int8_t app_usb_hid_get_report_handle(T_HID_DRIVER_REPORT_REQ_VAL req_value, uint8_t *data,
                                            uint16_t *length)
{
    int8_t ret = 0;

    APP_PRINT_TRACE2("app_usb_hid_get_report_handle, id 0x%x, type 0x%x", req_value.id, req_value.type);

    // TO DO;

    return ret;
}

void app_usb_hid_init(void)
{
    T_HID_CBS cbs = {.get_report = (INT_IN_FUNC)app_usb_hid_get_report_handle, .set_report = (INT_OUT_FUNC)app_usb_hid_set_report_handle};
    usb_hid_ual_register(cbs);
}
#endif

