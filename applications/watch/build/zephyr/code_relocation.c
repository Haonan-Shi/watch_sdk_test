
/* Auto generated code. Do not modify.*/
#include <zephyr/kernel.h>
#include <zephyr/linker/linker-defs.h>
#include <zephyr/kernel_structs.h>
#include <kernel_internal.h>

extern char __flash_text_reloc_start[];
extern char __flash_text_rom_start[];
extern char __flash_text_reloc_size[];

extern char __itcm_text_reloc_start[];
extern char __itcm_text_rom_start[];
extern char __itcm_text_reloc_size[];

extern char __itcm_rodata_reloc_start[];
extern char __itcm_rodata_rom_start[];
extern char __itcm_rodata_reloc_size[];

extern char __itcm_data_reloc_start[];
extern char __itcm_data_rom_start[];
extern char __itcm_data_reloc_size[];

extern char __itcm_bss_reloc_start[];
extern char __itcm_bss_rom_start[];
extern char __itcm_bss_reloc_size[];

extern char __sram_data_reloc_start[];
extern char __sram_data_rom_start[];
extern char __sram_data_reloc_size[];

extern char __sram_bss_reloc_start[];
extern char __sram_bss_rom_start[];
extern char __sram_bss_reloc_size[];

void data_copy_xip_relocation(void)
{

	z_early_memcpy(&__flash_text_reloc_start, &__flash_text_rom_start,
		           (size_t) &__flash_text_reloc_size);


	z_early_memcpy(&__itcm_text_reloc_start, &__itcm_text_rom_start,
		           (size_t) &__itcm_text_reloc_size);


	z_early_memcpy(&__itcm_rodata_reloc_start, &__itcm_rodata_rom_start,
		           (size_t) &__itcm_rodata_reloc_size);


	z_early_memcpy(&__itcm_data_reloc_start, &__itcm_data_rom_start,
		           (size_t) &__itcm_data_reloc_size);


	z_early_memcpy(&__sram_data_reloc_start, &__sram_data_rom_start,
		           (size_t) &__sram_data_reloc_size);


}

void bss_zeroing_relocation(void)
{

	z_early_memset(&__itcm_bss_reloc_start, 0,
		           (size_t) &__itcm_bss_reloc_size);

	z_early_memset(&__sram_bss_reloc_start, 0,
		           (size_t) &__sram_bss_reloc_size);

}
