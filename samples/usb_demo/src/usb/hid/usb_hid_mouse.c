/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if F_APP_USB_HID_SUPPORT
#include <string.h>
#include <stdlib.h>
#include "os_queue.h"
#include "os_sync.h"
#include "usb_hid_desc.h"
#include "usb_spec20.h"
#include "trace.h"
#include "usb_hid_spec.h"
#include "usb_hid.h"
#include "usb_pipe.h"
#include "errno.h"

typedef struct _t_hid_ual
{
    struct _t_hid_ual *p_next;
    T_HID_CBS cbs;
} T_HID_UAL;

T_OS_QUEUE ual_list;
int hid_protocol = HID_PROTOCOL_REPORT;
uint8_t mouse_tx_buf[12] = {0};
int8_t mouse_x_diff = 0;
int8_t mouse_y_diff = 0;

#define UNDEFINED_REPORT_ID 0
#define BOOT_MOUSE_TX_LEN   3

#define HID_REPORT_DESC_MOUSE  \
    0x05, 0x01,                         /* Usage Page (Generic Desktop)            */ \
          0x09, 0x02,                         /* Usage (Mouse)         */ \
          0xA1, 0x01,                         /* Collection (Application)         */ \
          0x85, MOUSE_REPORT_ID, /* REPORT_ID        (0xee) */ \
          0x09, 0x01,  /* Usage (pointer)                    */ \
          0xA1, 0x00,                         /* Collection (Physical)         */ \
          0x05, 0x09,  /* Usage page(Button)                    */ \
          0x19, 0x01,                         /* Usage Minimum (1)              */ \
          0x29, 0x03,                         /* Usage Maximum (3)              */ \
          0x15, 0x00,                         /* Logical Minimum (0)              */ \
          0x25, 0x01,                         /* Logical Maximum (1)              */ \
          0x95, 0x08,                         /* Report Count (8)                 */ \
          0x75, 0x01,                         /* Report Size (1)                  */ \
          0x81, 0x02,                         /* Input 3bits(Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)*/ \
          0x05, 0x01,  /* Usage page(Generic Desktop)                    */ \
          0x09, 0x30,                         /* Usage (X)              */ \
          0x09, 0x31,                         /* Usage (Y)              */ \
          0x09, 0x38,                         /* Usage (M)              */ \
          0x15, 0x81,                         /* Logical Minimum (127)              */ \
          0x25, 0x7F,                         /* Logical Maximum (-127)              */ \
          0x75, 0x08,                         /* Report Size (8)                  */ \
          0x95, 0x03,                         /* Report Count (3)                 */ \
          0x81, 0x06,                         /* Input 2position bytes(X&Y)(Data,Arr)*/ \
          0xC0,                               /* END_COLLECTION                    */ \
          0xC0,                               /* END_COLLECTION                    */

static const char report_descs[] =
{
    HID_REPORT_DESC_MOUSE
};

static T_USB_INTERFACE_DESC hid_std_if_desc =
{
    .bLength            = sizeof(T_USB_INTERFACE_DESC),
    .bDescriptorType    = USB_DESC_TYPE_INTERFACE,
    .bInterfaceNumber   = 0,
    .bAlternateSetting  = 0,
    .bNumEndpoints      = 1,
    .bInterfaceClass    = USB_CLASS_CODE_HID,
    .bInterfaceSubClass = 1,  // boot interface
    .bInterfaceProtocol = 2,  // 0 none, 1 keyboard, 2 mouse
    .iInterface         = 0,
};

static const T_HID_CS_IF_DESC  hid_cs_if_desc =
{
    .bLength            = sizeof(T_HID_CS_IF_DESC),
    .bDescriptorType    = DESC_TYPE_HID,
    .bcdHID             = 0x0110,
    .bCountryCode       = 0,
    .bNumDescriptors    = 1,
    .desc[0]            =
    {
        .bDescriptorType = DESC_TYPE_REPORT,
        .wDescriptorLength = sizeof(report_descs),
    },

};

static const T_USB_ENDPOINT_DESC int_in_ep_desc_fs =
{
    .bLength           = sizeof(T_USB_ENDPOINT_DESC),
    .bDescriptorType   = USB_DESC_TYPE_ENDPOINT,
    .bEndpointAddress  = USB_DIR_IN | 0x01,
    .bmAttributes      = USB_EP_TYPE_INT,
    .wMaxPacketSize    = 0x40,
    .bInterval         = 1,
};

static const T_USB_ENDPOINT_DESC int_in_ep_desc_hs =
{
    .bLength           = sizeof(T_USB_ENDPOINT_DESC),
    .bDescriptorType   = USB_DESC_TYPE_ENDPOINT,
    .bEndpointAddress  = USB_DIR_IN | 0x01,
    .bmAttributes      = USB_EP_TYPE_INT,
    .wMaxPacketSize    = 0x40,
    .bInterval         = 4,
};

static const void *hid_if_descs_fs[] =
{
    (void *) &hid_std_if_desc,
    (void *) &hid_cs_if_desc,
    (void *) &int_in_ep_desc_fs,
    NULL,
};

static const void *hid_if_descs_hs[] =
{
    (void *) &hid_std_if_desc,
    (void *) &hid_cs_if_desc,
    (void *) &int_in_ep_desc_hs,
    NULL,
};

void *usb_hid_data_pipe_open(uint8_t ep_addr, T_USB_HID_ATTR attr, uint8_t pending_req_num,
                             USB_HID_DATA_PIPE_CB cb)
{
    T_USB_HID_DRIVER_ATTR driver_attr;
    memcpy(&driver_attr, &attr, sizeof(T_USB_HID_DRIVER_ATTR));
    return usb_hid_driver_data_pipe_open(ep_addr, driver_attr, pending_req_num, cb);
}

int usb_hid_data_pipe_close(void *handle)
{
    return usb_hid_driver_data_pipe_close(handle);
}

bool usb_hid_mouse_param_send(void *handle, uint32_t report_id,
                              USB_HID_MOUSE_INPUT_REPORT *mouse_param,
                              uint32_t len)
{
    mouse_param->x += mouse_x_diff;
    mouse_param->y += mouse_y_diff;
    if (hid_protocol == HID_PROTOCOL_REPORT)
    {
        if (report_id == UNDEFINED_REPORT_ID)
        {
            memcpy(mouse_tx_buf, mouse_param, len);
        }
        else
        {
            mouse_tx_buf[0] = report_id;
            memcpy(mouse_tx_buf + 1, mouse_param, len - 1);
        }
    }
    else
    {
        memcpy(mouse_tx_buf, mouse_param, BOOT_MOUSE_TX_LEN);
        len = BOOT_MOUSE_TX_LEN;
    }

    if (usb_hid_driver_data_pipe_send(handle, mouse_tx_buf, len) == 0)
    {
        mouse_x_diff = 0;
        mouse_y_diff = 0;
        return true;
    }
    else
    {
        mouse_x_diff = mouse_param->x;
        mouse_y_diff = mouse_param->y;
    }
    return false;
}

int usb_hid_ual_register(T_HID_CBS cbs)
{
    T_HID_UAL *ual_node = malloc(sizeof(T_HID_UAL));
    memcpy(&ual_node->cbs, &cbs, sizeof(T_HID_CBS));
    os_queue_in(&ual_list, ual_node);
    return 0;
}

int32_t usb_hid_get_report(T_HID_DRIVER_REPORT_REQ_VAL req_value, void *buf, uint16_t *len)
{
    uint32_t ret = 0;
    T_HID_UAL *ual = (T_HID_UAL *)ual_list.p_first;
    while (ual)
    {
        if (ual->cbs.get_report)
        {
            ret += ual->cbs.get_report(req_value, buf, len);
        }
        ual = ual->p_next;
    }
    return ret;
}

int32_t usb_hid_set_report(T_HID_DRIVER_REPORT_REQ_VAL req_value, void *buf, uint16_t len)
{
    uint32_t ret = 0;
    T_HID_UAL *ual = (T_HID_UAL *)ual_list.p_first;
    while (ual)
    {
        if (ual->cbs.set_report)
        {
            ret += ual->cbs.set_report(req_value, buf, len);
        }
        ual = ual->p_next;
    }
    return ret;
}

int32_t usb_hid_protocol_change(uint8_t protocol)
{
    hid_protocol = protocol;
    return 0;
}

void usb_hid_init(void)
{
    void *inst = usb_hid_driver_inst_alloc();
    os_queue_init(&ual_list);
    usb_hid_driver_if_desc_register(inst, (void *)hid_if_descs_hs, (void *)hid_if_descs_fs,
                                    (void *)report_descs);

    T_USB_HID_DRIVER_CBS cbs = {0};
    cbs.get_report = usb_hid_get_report;
    cbs.set_report = usb_hid_set_report;
    cbs.protocol_change = usb_hid_protocol_change;
    usb_hid_driver_cbs_register(inst, &cbs);
    usb_hid_driver_init();
}
#endif
