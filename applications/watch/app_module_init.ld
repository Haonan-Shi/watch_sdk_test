/* applications/watch/app_module_init.ld */

#include <zephyr/linker/sections.h>
#include <zephyr/linker/linker-defs.h>
#include <zephyr/linker/linker-tool.h>

/* Start boundary */
SECTION_PROLOGUE(.app_module_init.start,,)
{
    KEEP(*(.app_module_init.start))
} GROUP_LINK_IN(ROMABLE_REGION)

/* Main init section */
SECTION_PROLOGUE(.app_module_init,,)
{
    KEEP(*(SORT(.app_module_init*)))
} GROUP_LINK_IN(ROMABLE_REGION)

/* End boundary */
SECTION_PROLOGUE(.app_module_init.end,,)
{
    KEEP(*(.app_module_init.end))
} GROUP_LINK_IN(ROMABLE_REGION)
