# IO Sample Module Addition

## Basic Engineering Environment Requirements

Hardware Platform:
- Based on Realtek 8773G embedded platform

- Equipped with SPI/QSPI/RGB/8080 display interfaces

- Supports external PSRAM (required for high-resolution displays with RGB interface)

Software Environment:
- Zephyr RTOS

- Configured system clock, GPIO, SPI and other peripheral drivers

- Completed basic system initialization and task scheduling configuration

## LCD Module Addition Guide
### 1. Configure Related Macros

In the `lcd/prj.conf` file, enable the following macro definitions:

#### Enable Display Driver Basic Configuration
CONFIG_DISPLAY=y
CONFIG_REALTEK_DISPLAY=y
CONFIG_REALTEK_DISPLAY_ENABLE_LCDC=y
CONFIG_LCDC=y
CONFIG_LCDC_DBIC=y

#### Enable Realtek LCD Driver Module Device Support
CONFIG_REALTEK_WEARABLE=y

#### Select LCD Driver (choose one according to actual LCD model, set to y, others to n)
CONFIG_REALTEK_LCD_SH8601Z_410_502_QSPI=y    # SH8601Z 410x502 QSPI interface
CONFIG_REALTEK_LCD_ST7265_800_480_RGB=n      # ST7265 800x480 RGB interface
CONFIG_REALTEK_LCD_ST77916_360_360_8080=n    # ST77916 360x360 8080 interface
CONFIG_REALTEK_LCD_ST7789_170_320_SPI=n      # ST7789 170x320 SPI interface

### 2. Test Code to Add

In the `lcd/src/main.c` file, add the following test code to light up the screen and draw content:

```c
int main(void)
{
    printf("Hello LCD Demo! I am in task[%s], I am a QSPI Demo \n", k_thread_name_get(k_current_get()));
    app_system_lower_init();

    if (!device_is_ready(dev))
    {
        printf("Display device not ready");
        return -ENODEV;
    }
    printf("Display device is ready");

    display_get_capabilities(dev, &cfg);

    printf("Display capabilities: width=%d, height=%d, pixel format=%d",
           cfg.x_resolution, cfg.y_resolution, cfg.current_pixel_format);
    printf("Bits per pixel: %d", bpp);

    uint8_t *image = malloc(100 * 100 * 2);

    struct display_buffer_descriptor desc =
    {
        .height = 100,
        .pitch = 100,
        .width = 100,
        .buf_size = 100 * 100 * bpp,
    };

    memset(image, 0xF0, 100 * 100 * 2);
    display_write(dev, 200, 200, &desc, image);

    return 0;
}
```

### 3. Notes

1. **Select the correct LCD driver**: Select the corresponding configuration macro in `prj.conf` according to the actual LCD model used.

2. Initialization order: Must ensure app_system_lower_init() is called before display device initialization

3. Hardware dependencies: LCD display may depend on PSRAM through SPIC interface, need to ensure PSRAM initialization succeeds

4. Clock configuration: Display performance is affected by system clock and SPIC clock frequency, can be adjusted as needed

## PPE Module
Enable the PPE test module in a project that can light up the screen normally (in `prj.conf` file):
```
CONFIG_REALTEK_DISPLAY_ENABLE_PPE=y
```  
## IDU Module
Enable the IDU module in a project that can light up the screen normally (in `prj.conf` file):
```
CONFIG_REALTEK_DISPLAY_ENABLE_IDU=y
```
## JPU Module
Enable the JPU module in a project that can light up the screen normally (in `prj.conf` file):
```
CONFIG_REALTEK_DISPLAY_ENABLE_JPU=y
``` 

