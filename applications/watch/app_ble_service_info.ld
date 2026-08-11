/* applications/watch/app_ble_service_info.ld */

#include <zephyr/linker/sections.h>
#include <zephyr/linker/linker-defs.h>
#include <zephyr/linker/linker-tool.h>

/* Start boundary */
SECTION_PROLOGUE(.app_ble_service_info.start,,)
{
    KEEP(*(.app_ble_service_info.start))
} GROUP_LINK_IN(ROMABLE_REGION)

/* Main section */
SECTION_PROLOGUE(.app_ble_service_info,,)
{
    KEEP(*(.app_ble_service_info))
} GROUP_LINK_IN(ROMABLE_REGION)

/* End boundary */
SECTION_PROLOGUE(.app_ble_service_info.end,,)
{
    KEEP(*(.app_ble_service_info.end))
} GROUP_LINK_IN(ROMABLE_REGION)