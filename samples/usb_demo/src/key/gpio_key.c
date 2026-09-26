/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if GPIO_KEY_EN
/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/dt-bindings/gpio/realtek-rtl87x3g-gpio.h>
#include "trace.h"
#include "app_usb_hid.h"
#include "app_io_msg.h"

/** @defgroup  GPIO_INT_DEMO  GPIO INTERRUPT DEMO
    * @brief  Gpio interrupt implementation demo code
    * @{
    */

/*============================================================================*
 *                              Macros
 *============================================================================*/
/** @defgroup Gpio_Interrupt_Exported_Macros Gpio Interrupt Exported Macros
  * @brief
  * @{
  */
#if CONFIG_SOC_SERIES_RTL8773D
#define KEY_HIGH_ACTIVE_EN  1
#else
#define KEY_HIGH_ACTIVE_EN  0
#endif

/* Demo key: P1_0, which is pad 8 = GPIOA7 (see hal pin_def.h) */
#define KEY_GPIO_NODE                             DT_NODELABEL(gpioa)
#define KEY_GPIO_PIN                              7
#define KEY_DEBOUNCE_MS                           30

#if KEY_HIGH_ACTIVE_EN
#define KEY_GPIO_FLAGS      (GPIO_ACTIVE_HIGH | GPIO_PULL_DOWN)
#else
#define KEY_GPIO_FLAGS      (GPIO_ACTIVE_LOW | GPIO_PULL_UP)
#endif

typedef enum
{
    KEY_CLICK_0           = 0,
    KEY_CLICK_1,
    KEY_CLICK_2,
    KEY_CLICK_3,
    KEY_CLICK_4,
} T_KEY_CNT_NUM;


/** @} */ /* End of group Gpio_Interrupt_Exported_Macros */

/*============================================================================*
 *                              Variables
 *============================================================================*/
static const struct gpio_dt_spec key_input =
{
    .port = DEVICE_DT_GET(KEY_GPIO_NODE),
    .pin  = KEY_GPIO_PIN,
    .dt_flags = KEY_GPIO_FLAGS | RTL87X3G_GPIO_INPUT_DEBOUNCE_MS(KEY_DEBOUNCE_MS),
};

static struct gpio_callback key_gpio_cb;

/*============================================================================*
 *                              Functions
 *============================================================================*/
/** @defgroup Gpio_Interrupt_Exported_Functions Gpio Interrupt Exported Functions
  * @brief
  * @{
  */

void app_key_send_msg(uint32_t param)
{
    T_IO_MSG button_msg;

    button_msg.type = IO_MSG_TYPE_GPIO;
    button_msg.subtype = IO_MSG_GPIO_KEY;
    button_msg.u.param = param;

    APP_PRINT_TRACE1("app_key_send_msg param:0x%x", param);
    app_io_msg_send(&button_msg);
}

void app_key_gpio_press(void)
{
    static uint8_t s_key_cnt = KEY_CLICK_1;
    APP_PRINT_TRACE1("app_key_gpio_press, cnt:%d", s_key_cnt);

    switch (s_key_cnt)
    {
    case KEY_CLICK_1:
    case KEY_CLICK_2:
    case KEY_CLICK_3:
        {
            app_key_send_msg(s_key_cnt);
            s_key_cnt++;
        }
        break;
    case KEY_CLICK_4:
        {
            app_key_send_msg(s_key_cnt);
            s_key_cnt = KEY_CLICK_1;
        }
        break;
    default:
        s_key_cnt = KEY_CLICK_1;
        break;
    }
}

#if USB_HID_KEYBOARD_EN
void app_key_handle_msg(T_IO_MSG *io_driver_msg_recv)
{
    uint8_t key_num = 0;

    key_num = io_driver_msg_recv->u.param & 0xFF;
    APP_PRINT_TRACE1("app_key_handle_msg key_num:%d", key_num);
    extern void app_usb_keyboard_input_demo(void);
    app_usb_keyboard_input_demo();
}
#elif USB_HID_MOUSE_EN
void app_key_handle_msg(T_IO_MSG *io_driver_msg_recv)
{
    uint8_t key_num = 0;

    key_num = io_driver_msg_recv->u.param & 0xFF;
    APP_PRINT_TRACE1("app_key_handle_msg key_num:%d", key_num);
    if (key_num == KEY_CLICK_1)
    {
        app_usb_mouse_handle_action(MMI_MOUSE_UP);
    }
    else if (key_num == KEY_CLICK_2)
    {
        app_usb_mouse_handle_action(MMI_MOUSE_DWON);
    }
    else if (key_num == KEY_CLICK_3)
    {
        app_usb_mouse_handle_action(MMI_MOUSE_LEFT);
    }
    else if (key_num == KEY_CLICK_4)
    {
        app_usb_mouse_handle_action(MMI_MOUSE_RIGHT);
    }
}
#endif

static void gpio_isr_cb(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    ARG_UNUSED(dev);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);

    /* logical level: 1 means active (pressed), the active polarity is taken
     * from key_input.dt_flags
     */
    int pressed = gpio_pin_get_dt(&key_input);

    IO_PRINT_INFO2("gpio_isr_cb: pin %d, pressed %d", key_input.pin, pressed);

    /* The rtl87x3g gpio driver has no both edge trigger, so keep following the
     * current level with a single edge trigger.
     */
    gpio_flags_t next_trig = (pressed > 0) ? GPIO_INT_EDGE_TO_INACTIVE
                             : GPIO_INT_EDGE_TO_ACTIVE;

    gpio_pin_interrupt_configure_dt(&key_input, next_trig);

    if (pressed > 0)
    {
        app_key_gpio_press();
    }
}

void key_init(void)
{
    int ret;

    if (!device_is_ready(key_input.port))
    {
        APP_PRINT_ERROR0("key_init: gpio device not ready");
        return;
    }

    ret = gpio_pin_configure_dt(&key_input, GPIO_INPUT);
    if (ret < 0)
    {
        APP_PRINT_ERROR1("key_init: pin configure failed %d", ret);
        return;
    }

    gpio_init_callback(&key_gpio_cb, gpio_isr_cb, BIT(key_input.pin));

    ret = gpio_add_callback(key_input.port, &key_gpio_cb);
    if (ret < 0)
    {
        APP_PRINT_ERROR1("key_init: add callback failed %d", ret);
        return;
    }

    ret = gpio_pin_interrupt_configure_dt(&key_input, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret < 0)
    {
        APP_PRINT_ERROR1("key_init: interrupt configure failed %d", ret);
        return;
    }

    APP_PRINT_INFO1("key_init: done, gpio pin %d", key_input.pin);
}

#endif
