 OUTPUT_FORMAT("elf32-littlearm")
_region_min_align = 4;
MEMORY
    {
    FLASH (rx) : ORIGIN = (0x70000000 + 0x9e000), LENGTH = (0x180000 - 0x0)
    RAM (wx) : ORIGIN = 0x2000b800, LENGTH = (81 * 1K)
    TRACE : ORIGIN = 3764649984, LENGTH = 4194304 ITCM : ORIGIN = 789504, LENGTH = 27648 SRAM : ORIGIN = 537001984, LENGTH = 106496 DSPRAM : ORIGIN = 537165824, LENGTH = 81920 PSRAM0_MCU : ORIGIN = 570425344, LENGTH = 4194304 PSRAM1_DSP : ORIGIN = 603979776, LENGTH = 524288 PSRAM1_MCU : ORIGIN = 604504064, LENGTH = 2621440 PSRAM1_NC : ORIGIN = 607125504, LENGTH = 1048576
    IDT_LIST (wx) : ORIGIN = 0xFFFF7FFF, LENGTH = 32K
    }
ENTRY("__start")
SECTIONS
    {
 .rel.plt :
 {
 *(.rel.plt)
 PROVIDE_HIDDEN (__rel_iplt_start = .);
 *(.rel.iplt)
 PROVIDE_HIDDEN (__rel_iplt_end = .);
 }
 .rela.plt :
 {
 *(.rela.plt)
 PROVIDE_HIDDEN (__rela_iplt_start = .);
 *(.rela.iplt)
 PROVIDE_HIDDEN (__rela_iplt_end = .);
 }
 .rel.dyn :
 {
 *(.rel.*)
 }
 .rela.dyn :
 {
 *(.rela.*)
 }
    /DISCARD/ :
 {
 *(.plt)
 }
    /DISCARD/ :
 {
 *(.iplt)
 }
   
 __rom_region_start = (0x70000000 + 0x9e000);
    rom_start :
 {
HIDDEN(__rom_start_address = .);
FILL(0x00);
. += 0x0 - (. - __rom_start_address);
. = ALIGN(4);
KEEP(*(.image_header))
KEEP(*(.".image_header.*"))
__end = .;
. = ALIGN( 1 << LOG2CEIL(4 * 32) );
. = ALIGN( 1 << LOG2CEIL(4 * (16 + 116)) );
_vector_start = .;
KEEP(*(.exc_vector_table))
KEEP(*(".exc_vector_table.*"))
KEEP(*(.vectors))
_vector_end = .;
. = ALIGN(4);
KEEP(*(.gnu.linkonce.irq_vector_table*))
 _vector_end = .;
 } > FLASH
 .flash_text_reloc :
        {
                . = ALIGN(4);
                KEEP(*prep_c.c.obj(.rel.text.z_prep_c))
                KEEP(*prep_c.c.obj(.text.z_prep_c))
                KEEP(*reset.S.obj(.rel.text._reset_section))
                KEEP(*reset.S.obj(.text._reset_section))
                . = ALIGN(4);
 }
> FLASH AT > FLASH
        __flash_text_reloc_end = .;
        __flash_text_reloc_start = ADDR(.flash_text_reloc);
        __flash_text_reloc_size = __flash_text_reloc_end - __flash_text_reloc_start;
        __flash_text_rom_start = LOADADDR(.flash_text_reloc);
 .itcm_text_reloc :
        {
                . = ALIGN(4);
                KEEP(*__aeabi_atexit.c.obj(.rel.text.__aeabi_atexit))
                KEEP(*__aeabi_atexit.c.obj(.text.__aeabi_atexit))
                KEEP(*coredump.c.obj(.rel.text.arch_coredump_info_dump))
                KEEP(*coredump.c.obj(.text.arch_coredump_info_dump))
                KEEP(*coredump.c.obj(.text.arch_coredump_tgt_code_get))
                KEEP(*cortex_m_systick.c.obj(.rel.text.elapsed))
                KEEP(*cortex_m_systick.c.obj(.rel.text.sys_clock_cycle_get_32))
                KEEP(*cortex_m_systick.c.obj(.rel.text.sys_clock_driver_init))
                KEEP(*cortex_m_systick.c.obj(.rel.text.sys_clock_idle_exit))
                KEEP(*cortex_m_systick.c.obj(.rel.text.sys_clock_isr))
                KEEP(*cortex_m_systick.c.obj(.rel.text.sys_clock_only_add_cycle_count))
                KEEP(*cortex_m_systick.c.obj(.text.elapsed))
                KEEP(*cortex_m_systick.c.obj(.text.sys_clock_cycle_get_32))
                KEEP(*cortex_m_systick.c.obj(.text.sys_clock_disable))
                KEEP(*cortex_m_systick.c.obj(.text.sys_clock_driver_init))
                KEEP(*cortex_m_systick.c.obj(.text.sys_clock_elapsed))
                KEEP(*cortex_m_systick.c.obj(.text.sys_clock_idle_exit))
                KEEP(*cortex_m_systick.c.obj(.text.sys_clock_isr))
                KEEP(*cortex_m_systick.c.obj(.text.sys_clock_only_add_cycle_count))
                KEEP(*cortex_m_systick.c.obj(.text.sys_clock_set_timeout))
                KEEP(*cpu_idle.c.obj(.text.arch_cpu_atomic_idle))
                KEEP(*cpu_idle.c.obj(.text.arch_cpu_idle))
                KEEP(*cpu_idle.c.obj(.text.z_arm_cpu_idle_init))
                KEEP(*device.c.obj(.rel.text.pm_device_action_run))
                KEEP(*device.c.obj(.rel.text.pm_device_driver_init))
                KEEP(*device.c.obj(.rel.text.pm_device_is_any_busy))
                KEEP(*device.c.obj(.rel.text.pm_device_state_str))
                KEEP(*device.c.obj(.text.pm_device_action_run))
                KEEP(*device.c.obj(.text.pm_device_busy_clear))
                KEEP(*device.c.obj(.text.pm_device_busy_set))
                KEEP(*device.c.obj(.text.pm_device_driver_init))
                KEEP(*device.c.obj(.text.pm_device_is_any_busy))
                KEEP(*device.c.obj(.text.pm_device_is_busy))
                KEEP(*device.c.obj(.text.pm_device_is_powered))
                KEEP(*device.c.obj(.text.pm_device_on_power_domain))
                KEEP(*device.c.obj(.text.pm_device_power_domain_add))
                KEEP(*device.c.obj(.text.pm_device_power_domain_remove))
                KEEP(*device.c.obj(.text.pm_device_state_get))
                KEEP(*device.c.obj(.text.pm_device_state_str))
                KEEP(*device.c.obj(.text.pm_device_wakeup_enable))
                KEEP(*device.c.obj(.text.pm_device_wakeup_is_capable))
                KEEP(*device.c.obj(.text.pm_device_wakeup_is_enabled))
                KEEP(*dynamic_isr.c.obj(.rel.text.arch_irq_connect_dynamic))
                KEEP(*dynamic_isr.c.obj(.rel.text.z_isr_install))
                KEEP(*dynamic_isr.c.obj(.text.arch_irq_connect_dynamic))
                KEEP(*dynamic_isr.c.obj(.text.z_isr_install))
                KEEP(*exc_exit.c.obj(.rel.text._HandlerModeExit))
                KEEP(*exc_exit.c.obj(.text._HandlerModeExit))
                KEEP(*fatal.c.obj(.rel.text.arch_syscall_oops))
                KEEP(*fatal.c.obj(.rel.text.z_arm_fatal_error))
                KEEP(*fatal.c.obj(.rel.text.z_do_kernel_oops))
                KEEP(*fatal.c.obj(.text.arch_syscall_oops))
                KEEP(*fatal.c.obj(.text.z_arm_fatal_error))
                KEEP(*fatal.c.obj(.text.z_do_kernel_oops))
                KEEP(*fault.c.obj(.rel.text.bus_fault.constprop.0))
                KEEP(*fault.c.obj(.rel.text.mem_manage_fault.constprop.0))
                KEEP(*fault.c.obj(.rel.text.usage_fault.constprop.0))
                KEEP(*fault.c.obj(.rel.text.z_arm_fault))
                KEEP(*fault.c.obj(.rel.text.z_log_msg_simple_create_0.constprop.0))
                KEEP(*fault.c.obj(.text.bus_fault.constprop.0))
                KEEP(*fault.c.obj(.text.mem_manage_fault.constprop.0))
                KEEP(*fault.c.obj(.text.usage_fault.constprop.0))
                KEEP(*fault.c.obj(.text.z_arm_fault))
                KEEP(*fault.c.obj(.text.z_arm_fault_init))
                KEEP(*fault.c.obj(.text.z_log_msg_simple_create_0.constprop.0))
                KEEP(*fault_s.S.obj(.rel.text.__fault))
                KEEP(*fault_s.S.obj(.text.__fault))
                KEEP(*fpu.c.obj(.rel.text.z_arm_save_fp_context))
                KEEP(*fpu.c.obj(.text.z_arm_restore_fp_context))
                KEEP(*fpu.c.obj(.text.z_arm_save_fp_context))
                KEEP(*irq_init.c.obj(.text.z_arm_interrupt_init))
                KEEP(*irq_manage.c.obj(.rel.text._arch_isr_direct_pm))
                KEEP(*irq_manage.c.obj(.rel.text.arch_irq_connect_dynamic))
                KEEP(*irq_manage.c.obj(.rel.text.z_arm_irq_priority_set))
                KEEP(*irq_manage.c.obj(.rel.text.z_irq_spurious))
                KEEP(*irq_manage.c.obj(.text._arch_isr_direct_pm))
                KEEP(*irq_manage.c.obj(.text.arch_irq_connect_dynamic))
                KEEP(*irq_manage.c.obj(.text.arch_irq_disable))
                KEEP(*irq_manage.c.obj(.text.arch_irq_enable))
                KEEP(*irq_manage.c.obj(.text.arch_irq_is_enabled))
                KEEP(*irq_manage.c.obj(.text.z_arm_irq_priority_set))
                KEEP(*irq_manage.c.obj(.text.z_irq_spurious))
                KEEP(*isr_wrapper.c.obj(.rel.text._isr_wrapper))
                KEEP(*isr_wrapper.c.obj(.text._isr_wrapper))
                KEEP(*nmi.c.obj(.rel.text.z_arm_nmi))
                KEEP(*nmi.c.obj(.rel.text.z_arm_nmi_set_handler))
                KEEP(*nmi.c.obj(.text.z_arm_nmi))
                KEEP(*nmi.c.obj(.text.z_arm_nmi_set_handler))
                KEEP(*nmi_on_reset.S.obj(.rel.text.z_SysNmiOnReset))
                KEEP(*nmi_on_reset.S.obj(.text.z_SysNmiOnReset))
                KEEP(*osif_zephyr.c.obj(.rel.text.RamVectorTableUpdate_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.__wrap__calloc_r))
                KEEP(*osif_zephyr.c.obj(.rel.text.__wrap__free_r))
                KEEP(*osif_zephyr.c.obj(.rel.text.__wrap__malloc_r))
                KEEP(*osif_zephyr.c.obj(.rel.text.__wrap__realloc_r))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_delay_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_heap_choice))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_heap_init_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_mem_aligned_alloc_intern_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_mem_aligned_free_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_mem_alloc_intern_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_mem_free_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_mem_peek_printf))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_mem_peek_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_mem_zalloc_intern_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_msg_queue_create_intern_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_msg_queue_delete_intern_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_msg_recv_intern_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_msg_send_intern_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_mutex_create_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_mutex_delete_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_mutex_give_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_mutex_take_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_pm_find_nearest_timeout_tick_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_pm_return_to_idle_task_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_pm_store_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_sched_resume_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_sched_start_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_sched_state_get_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_sched_stop_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_sched_suspend_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_sem_create_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_sem_delete_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_sem_give_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_sem_take_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_sys_tick_increase_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_sys_time_get_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_systick_handler_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_task_create_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_task_delete_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_task_handle_get_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_task_priority_get_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_task_priority_set_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_task_resume_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_task_signal_clear_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_task_signal_recv_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_task_signal_send_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_task_suspend_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_task_yield_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_timer_auto_reload_get_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_timer_create_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_timer_delete_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_timer_handle_get))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_timer_id_get_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_timer_index_get))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_timer_pend_function_call_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_timer_restart_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_timer_start_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_timer_state_get_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_timer_stop_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.os_zephyr_patch_init))
                KEEP(*osif_zephyr.c.obj(.rel.text.osif_mem_func_init_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.osif_msg_func_init_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.osif_pm_func_init_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.osif_sched_func_init_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.osif_sync_func_init_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.osif_task_func_init_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.osif_timer_func_init_zephyr))
                KEEP(*osif_zephyr.c.obj(.rel.text.pendcall_handler))
                KEEP(*osif_zephyr.c.obj(.rel.text.rtk_update_isr))
                KEEP(*osif_zephyr.c.obj(.rel.text.sys_multi_heap_free_kheap))
                KEEP(*osif_zephyr.c.obj(.text.RamVectorTableUpdate_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.__wrap__calloc_r))
                KEEP(*osif_zephyr.c.obj(.text.__wrap__free_r))
                KEEP(*osif_zephyr.c.obj(.text.__wrap__malloc_r))
                KEEP(*osif_zephyr.c.obj(.text.__wrap__realloc_r))
                KEEP(*osif_zephyr.c.obj(.text.os_delay_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_heap_choice))
                KEEP(*osif_zephyr.c.obj(.text.os_heap_init_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_lock_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_mem_aligned_alloc_intern_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_mem_aligned_free_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_mem_alloc_intern_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_mem_free_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_mem_peek_printf))
                KEEP(*osif_zephyr.c.obj(.text.os_mem_peek_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_mem_zalloc_intern_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_msg_queue_create_intern_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_msg_queue_delete_intern_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_msg_queue_peek_intern_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_msg_recv_intern_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_msg_send_intern_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_mutex_create_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_mutex_delete_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_mutex_give_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_mutex_take_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_pm_find_nearest_timeout_tick_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_pm_restore_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_pm_return_to_idle_task_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_pm_store_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_sched_resume_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_sched_start_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_sched_state_get_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_sched_stop_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_sched_suspend_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_sem_create_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_sem_delete_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_sem_give_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_sem_take_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_sys_tick_clk_get_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_sys_tick_increase_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_sys_tick_rate_get_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_sys_time_get_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_systick_handler_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_task_create_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_task_delete_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_task_handle_get_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_task_priority_get_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_task_priority_set_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_task_resume_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_task_signal_clear_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_task_signal_create_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_task_signal_recv_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_task_signal_send_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_task_suspend_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_task_yield_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_timer_auto_reload_get_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_timer_create_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_timer_delete_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_timer_handle_get))
                KEEP(*osif_zephyr.c.obj(.text.os_timer_id_get_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_timer_index_get))
                KEEP(*osif_zephyr.c.obj(.text.os_timer_pend_function_call_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_timer_restart_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_timer_start_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_timer_state_get_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_timer_stop_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_unlock_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.os_zephyr_patch_init))
                KEEP(*osif_zephyr.c.obj(.text.osif_mem_func_init_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.osif_msg_func_init_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.osif_pm_func_init_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.osif_sched_func_init_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.osif_sync_func_init_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.osif_task_func_init_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.osif_timer_func_init_zephyr))
                KEEP(*osif_zephyr.c.obj(.text.pendcall_handler))
                KEEP(*osif_zephyr.c.obj(.text.rtk_update_isr))
                KEEP(*osif_zephyr.c.obj(.text.sys_multi_heap_free_kheap))
                KEEP(*power.c.obj(.rel.text.pm_resume_devices_rtk))
                KEEP(*power.c.obj(.rel.text.pm_reusme_systick_and_process_timeout))
                KEEP(*power.c.obj(.rel.text.pm_suspend_devices_rtk))
                KEEP(*power.c.obj(.rel.text.rtl87x3g_power_init))
                KEEP(*power.c.obj(.text.pm_resume_devices_rtk))
                KEEP(*power.c.obj(.text.pm_reusme_systick_and_process_timeout))
                KEEP(*power.c.obj(.text.pm_state_exit_post_ops))
                KEEP(*power.c.obj(.text.pm_suspend_devices_rtk))
                KEEP(*power.c.obj(.text.rtl87x3g_power_init))
                KEEP(*prep_c.c.obj(.rel.text.z_prep_c))
                KEEP(*prep_c.c.obj(.text.z_prep_c))
                KEEP(*reset.S.obj(.rel.text._reset_section))
                KEEP(*reset.S.obj(.text._reset_section))
                KEEP(*scb.c.obj(.text.sys_arch_reboot))
                KEEP(*spinlock_validate.c.obj(.rel.text.z_spin_lock_set_owner))
                KEEP(*spinlock_validate.c.obj(.rel.text.z_spin_lock_valid))
                KEEP(*spinlock_validate.c.obj(.rel.text.z_spin_unlock_valid))
                KEEP(*spinlock_validate.c.obj(.text.z_spin_lock_set_owner))
                KEEP(*spinlock_validate.c.obj(.text.z_spin_lock_valid))
                KEEP(*spinlock_validate.c.obj(.text.z_spin_unlock_valid))
                KEEP(*sw_isr_common.c.obj(.rel.text.z_get_sw_isr_table_idx))
                KEEP(*sw_isr_common.c.obj(.text.z_get_sw_isr_table_idx))
                KEEP(*swap.c.obj(.rel.text.arch_swap))
                KEEP(*swap.c.obj(.text.arch_swap))
                KEEP(*swap_helper.S.obj(.rel.text.z_arm_pendsv))
                KEEP(*swap_helper.S.obj(.rel.text.z_arm_svc))
                KEEP(*swap_helper.S.obj(.text.z_arm_pendsv))
                KEEP(*swap_helper.S.obj(.text.z_arm_svc))
                KEEP(*thread.c.obj(.rel.text.arch_float_disable))
                KEEP(*thread.c.obj(.rel.text.arch_new_thread))
                KEEP(*thread.c.obj(.rel.text.arch_switch_to_main_thread))
                KEEP(*thread.c.obj(.rel.text.k_thread_runtime_stats_all_get))
                KEEP(*thread.c.obj(.rel.text.k_thread_runtime_stats_get))
                KEEP(*thread.c.obj(.rel.text.k_thread_state_str))
                KEEP(*thread.c.obj(.rel.text.k_thread_user_mode_enter))
                KEEP(*thread.c.obj(.rel.text.z_impl_k_is_preempt_thread))
                KEEP(*thread.c.obj(.rel.text.z_impl_k_thread_create))
                KEEP(*thread.c.obj(.rel.text.z_impl_k_thread_name_copy))
                KEEP(*thread.c.obj(.rel.text.z_impl_k_thread_name_set))
                KEEP(*thread.c.obj(.rel.text.z_impl_k_thread_stack_space_get))
                KEEP(*thread.c.obj(.rel.text.z_impl_k_thread_start))
                KEEP(*thread.c.obj(.rel.text.z_setup_new_thread))
                KEEP(*thread.c.obj(.rel.text.z_thread_mark_switched_in))
                KEEP(*thread.c.obj(.rel.text.z_thread_mark_switched_out))
                KEEP(*thread.c.obj(.text.arch_float_disable))
                KEEP(*thread.c.obj(.text.arch_float_enable))
                KEEP(*thread.c.obj(.text.arch_irq_lock_outlined))
                KEEP(*thread.c.obj(.text.arch_irq_unlock_outlined))
                KEEP(*thread.c.obj(.text.arch_new_thread))
                KEEP(*thread.c.obj(.text.arch_switch_to_main_thread))
                KEEP(*thread.c.obj(.text.configure_builtin_stack_guard))
                KEEP(*thread.c.obj(.text.k_is_in_isr))
                KEEP(*thread.c.obj(.text.k_thread_name_get))
                KEEP(*thread.c.obj(.text.k_thread_runtime_stats_all_get))
                KEEP(*thread.c.obj(.text.k_thread_runtime_stats_get))
                KEEP(*thread.c.obj(.text.k_thread_state_str))
                KEEP(*thread.c.obj(.text.k_thread_user_mode_enter))
                KEEP(*thread.c.obj(.text.z_impl_k_is_preempt_thread))
                KEEP(*thread.c.obj(.text.z_impl_k_thread_create))
                KEEP(*thread.c.obj(.text.z_impl_k_thread_name_copy))
                KEEP(*thread.c.obj(.text.z_impl_k_thread_name_set))
                KEEP(*thread.c.obj(.text.z_impl_k_thread_priority_get))
                KEEP(*thread.c.obj(.text.z_impl_k_thread_stack_space_get))
                KEEP(*thread.c.obj(.text.z_impl_k_thread_start))
                KEEP(*thread.c.obj(.text.z_init_thread_base))
                KEEP(*thread.c.obj(.text.z_setup_new_thread))
                KEEP(*thread.c.obj(.text.z_stack_space_get))
                KEEP(*thread.c.obj(.text.z_thread_mark_switched_in))
                KEEP(*thread.c.obj(.text.z_thread_mark_switched_out))
                KEEP(*thread_abort.c.obj(.rel.text.z_impl_k_thread_abort))
                KEEP(*thread_abort.c.obj(.text.z_impl_k_thread_abort))
                KEEP(*usage.c.obj(.rel.text.k_sys_runtime_stats_disable))
                KEEP(*usage.c.obj(.rel.text.k_sys_runtime_stats_enable))
                KEEP(*usage.c.obj(.rel.text.z_sched_cpu_usage))
                KEEP(*usage.c.obj(.rel.text.z_sched_thread_usage))
                KEEP(*usage.c.obj(.rel.text.z_sched_usage_start))
                KEEP(*usage.c.obj(.rel.text.z_sched_usage_stop))
                KEEP(*usage.c.obj(.text.k_sys_runtime_stats_disable))
                KEEP(*usage.c.obj(.text.k_sys_runtime_stats_enable))
                KEEP(*usage.c.obj(.text.sched_cpu_update_usage))
                KEEP(*usage.c.obj(.text.z_sched_cpu_usage))
                KEEP(*usage.c.obj(.text.z_sched_thread_usage))
                KEEP(*usage.c.obj(.text.z_sched_usage_start))
                KEEP(*usage.c.obj(.text.z_sched_usage_stop))
                . = ALIGN(4);
 }
> ITCM AT > FLASH
        __itcm_text_reloc_end = .;
        __itcm_text_reloc_start = ADDR(.itcm_text_reloc);
        __itcm_text_reloc_size = __itcm_text_reloc_end - __itcm_text_reloc_start;
        __itcm_text_rom_start = LOADADDR(.itcm_text_reloc);
 .itcm_rodata_reloc :
        {
                __itcm_rodata_reloc_start = .;
                KEEP(*cortex_m_systick.c.obj(.rodata.sys_clock_cycle_get_32.str1.1))
                KEEP(*device.c.obj(.rel.rodata.CSWTCH.23))
                KEEP(*device.c.obj(.rodata.CSWTCH.23))
                KEEP(*device.c.obj(.rodata.action_expected_state))
                KEEP(*device.c.obj(.rodata.action_target_state))
                KEEP(*device.c.obj(.rodata.pm_device_state_str.str1.1))
                KEEP(*device.c.obj(.rodata.str1.1))
                KEEP(*dynamic_isr.c.obj(.rodata.z_isr_install.str1.1))
                KEEP(*fatal.c.obj(.rodata.z_arm_fatal_error.str1.1))
                KEEP(*fault.c.obj(.rodata.bus_fault.constprop.0.str1.1))
                KEEP(*fault.c.obj(.rodata.mem_manage_fault.constprop.0.str1.1))
                KEEP(*fault.c.obj(.rodata.usage_fault.constprop.0.str1.1))
                KEEP(*fault.c.obj(.rodata.z_arm_fault.str1.1))
                KEEP(*fpu.c.obj(.rodata.z_arm_save_fp_context.str1.1))
                KEEP(*irq_manage.c.obj(.rodata.z_arm_irq_priority_set.str1.1))
                KEEP(*osif_zephyr.c.obj(.rodata.RamVectorTableUpdate_zephyr.str1.1))
                KEEP(*osif_zephyr.c.obj(.rodata.__func__.0))
                KEEP(*osif_zephyr.c.obj(.rodata.__func__.1))
                KEEP(*osif_zephyr.c.obj(.rodata.__func__.10))
                KEEP(*osif_zephyr.c.obj(.rodata.__func__.11))
                KEEP(*osif_zephyr.c.obj(.rodata.__func__.12))
                KEEP(*osif_zephyr.c.obj(.rodata.__func__.13))
                KEEP(*osif_zephyr.c.obj(.rodata.__func__.14))
                KEEP(*osif_zephyr.c.obj(.rodata.__func__.15))
                KEEP(*osif_zephyr.c.obj(.rodata.__func__.16))
                KEEP(*osif_zephyr.c.obj(.rodata.__func__.17))
                KEEP(*osif_zephyr.c.obj(.rodata.__func__.18))
                KEEP(*osif_zephyr.c.obj(.rodata.__func__.2))
                KEEP(*osif_zephyr.c.obj(.rodata.__func__.3))
                KEEP(*osif_zephyr.c.obj(.rodata.__func__.4))
                KEEP(*osif_zephyr.c.obj(.rodata.__func__.5))
                KEEP(*osif_zephyr.c.obj(.rodata.__func__.6))
                KEEP(*osif_zephyr.c.obj(.rodata.__func__.7))
                KEEP(*osif_zephyr.c.obj(.rodata.__func__.8))
                KEEP(*osif_zephyr.c.obj(.rodata.__func__.9))
                KEEP(*osif_zephyr.c.obj(.rodata.os_heap_choice.str1.1))
                KEEP(*osif_zephyr.c.obj(.rodata.os_heap_init_zephyr.str1.1))
                KEEP(*osif_zephyr.c.obj(.rodata.os_mem_peek_printf.str1.1))
                KEEP(*osif_zephyr.c.obj(.rodata.os_mem_peek_zephyr.str1.1))
                KEEP(*osif_zephyr.c.obj(.rodata.os_msg_queue_create_intern_zephyr.str1.1))
                KEEP(*osif_zephyr.c.obj(.rodata.os_mutex_create_zephyr.str1.1))
                KEEP(*osif_zephyr.c.obj(.rodata.os_mutex_delete_zephyr.str1.1))
                KEEP(*osif_zephyr.c.obj(.rodata.os_mutex_give_zephyr.str1.1))
                KEEP(*osif_zephyr.c.obj(.rodata.os_mutex_take_zephyr.str1.1))
                KEEP(*osif_zephyr.c.obj(.rodata.os_pm_find_nearest_timeout_tick_zephyr.str1.1))
                KEEP(*osif_zephyr.c.obj(.rodata.os_sched_start_zephyr.str1.1))
                KEEP(*osif_zephyr.c.obj(.rodata.os_sched_stop_zephyr.str1.1))
                KEEP(*osif_zephyr.c.obj(.rodata.os_sem_create_zephyr.str1.1))
                KEEP(*osif_zephyr.c.obj(.rodata.os_sem_delete_zephyr.str1.1))
                KEEP(*osif_zephyr.c.obj(.rodata.os_sem_give_zephyr.str1.1))
                KEEP(*osif_zephyr.c.obj(.rodata.os_sem_take_zephyr.str1.1))
                KEEP(*osif_zephyr.c.obj(.rodata.os_task_create_zephyr.str1.1))
                KEEP(*osif_zephyr.c.obj(.rodata.os_task_delete_zephyr.str1.1))
                KEEP(*osif_zephyr.c.obj(.rodata.os_task_priority_get_zephyr.str1.1))
                KEEP(*osif_zephyr.c.obj(.rodata.os_task_priority_set_zephyr.str1.1))
                KEEP(*osif_zephyr.c.obj(.rodata.os_task_resume_zephyr.str1.1))
                KEEP(*osif_zephyr.c.obj(.rodata.os_task_signal_clear_zephyr.str1.1))
                KEEP(*osif_zephyr.c.obj(.rodata.os_task_signal_recv_zephyr.str1.1))
                KEEP(*osif_zephyr.c.obj(.rodata.os_task_signal_send_zephyr.str1.1))
                KEEP(*osif_zephyr.c.obj(.rodata.os_task_suspend_zephyr.str1.1))
                KEEP(*osif_zephyr.c.obj(.rodata.os_task_yield_zephyr.str1.1))
                KEEP(*osif_zephyr.c.obj(.rodata.os_timer_create_zephyr.str1.1))
                KEEP(*osif_zephyr.c.obj(.rodata.os_timer_delete_zephyr.str1.1))
                KEEP(*osif_zephyr.c.obj(.rodata.os_timer_handle_get.str1.1))
                KEEP(*osif_zephyr.c.obj(.rodata.os_timer_id_get_zephyr.str1.1))
                KEEP(*osif_zephyr.c.obj(.rodata.os_timer_index_get.str1.1))
                KEEP(*osif_zephyr.c.obj(.rodata.os_timer_pend_function_call_zephyr.str1.1))
                KEEP(*osif_zephyr.c.obj(.rodata.os_timer_stop_zephyr.str1.1))
                KEEP(*power.c.obj(.rodata.pm_suspend_devices_rtk.str1.1))
                KEEP(*power.c.obj(.rodata.str1.1))
                KEEP(*sw_isr_common.c.obj(.rodata.z_get_sw_isr_table_idx.str1.1))
                KEEP(*thread.c.obj(.rel.rodata.state_string.0))
                KEEP(*thread.c.obj(.rodata.k_thread_state_str.str1.1))
                KEEP(*thread.c.obj(.rodata.state_string.0))
                KEEP(*thread.c.obj(.rodata.str1.1))
                KEEP(*thread.c.obj(.rodata.z_impl_k_thread_create.str1.1))
                KEEP(*thread.c.obj(.rodata.z_setup_new_thread.str1.1))
                KEEP(*usage.c.obj(.rodata.z_sched_usage_stop.str1.1))
                . = ALIGN(_region_min_align);
                __itcm_rodata_reloc_end = .;
 }
> ITCM AT > FLASH
        __itcm_rodata_reloc_size = __itcm_rodata_reloc_end - __itcm_rodata_reloc_start;
        __itcm_rodata_rom_start = LOADADDR(.itcm_rodata_reloc);
 .itcm_data_reloc :
        {
                . = ALIGN(4);
                KEEP(*nmi.c.obj(.data.handler))
                KEEP(*nmi.c.obj(.rel.data.handler))
                . = ALIGN(4);
 }
> ITCM AT > FLASH
        __itcm_data_reloc_end = .;
        __itcm_data_reloc_start = ADDR(.itcm_data_reloc);
        __itcm_data_reloc_size = __itcm_data_reloc_end - __itcm_data_reloc_start;
        __itcm_data_rom_start = LOADADDR(.itcm_data_reloc);
 .itcm_bss_reloc :
        {
                . = ALIGN(4);
                KEEP(*coredump.c.obj(.bss.arch_blk))
                KEEP(*coredump.c.obj(.bss.z_arm_coredump_fault_sp))
                KEEP(*cortex_m_systick.c.obj(.bss.cycle_count))
                KEEP(*cortex_m_systick.c.obj(.bss.last_load))
                KEEP(*cortex_m_systick.c.obj(.bss.lock))
                KEEP(*cortex_m_systick.c.obj(.bss.overflow_cyc))
                KEEP(*osif_zephyr.c.obj(.bss.k_heap_array))
                KEEP(*osif_zephyr.c.obj(.bss.multi_heap))
                KEEP(*osif_zephyr.c.obj(.bss.osif_timer_pool))
                KEEP(*osif_zephyr.c.obj(.bss.state.19))
                KEEP(*power.c.obj(.bss.cpu_store_buffer))
                KEEP(*power.c.obj(.bss.num_susp_rtk))
                KEEP(*power.c.obj(.bss.peri_on_store_buffer))
                KEEP(*usage.c.obj(.bss.usage_lock))
                . = ALIGN(4);
 } > ITCM
        __itcm_bss_reloc_end = .;
        __itcm_bss_reloc_start = ADDR(.itcm_bss_reloc);
        __itcm_bss_reloc_size = __itcm_bss_reloc_end - __itcm_bss_reloc_start;
 .sram_data_reloc :
        {
                . = ALIGN(4);
                *audio_a2dp_src.c.obj(.data.a2dp_sbc_time)
                *audio_a2dp_src.c.obj(.data.a2dp_src_timer)
                *audio_record.c.obj(.data.mic_record)
                *gui_port_os.c.obj(.data.os_api)
                *gui_port_os.c.obj(.rel.data.os_api)
                *ota_service.c.obj(.data.protocol_info)
                . = ALIGN(4);
 }
> SRAM AT > FLASH
        __sram_data_reloc_end = .;
        __sram_data_reloc_start = ADDR(.sram_data_reloc);
        __sram_data_reloc_size = __sram_data_reloc_end - __sram_data_reloc_start;
        __sram_data_rom_start = LOADADDR(.sram_data_reloc);
 .sram_bss_reloc :
        {
                . = ALIGN(4);
                *app_a2dp_enc.c.obj(.bss.a2dp_enc_queue)
                *app_audio_if.c.obj(.bss.play_fail_index)
                *audio_a2dp_src.c.obj(.bss.a2dp_src_db)
                *audio_a2dp_src.c.obj(.bss.a2dp_timer_handle)
                *audio_a2dp_src.c.obj(.bss.adjust_audio_pipe_vol_flag)
                *audio_a2dp_src.c.obj(.bss.audio_pipe_handle)
                *audio_a2dp_src.c.obj(.bss.cur_pair_idx)
                *audio_a2dp_src.c.obj(.bss.cur_timestamp)
                *audio_a2dp_src.c.obj(.bss.dsp_tx_ack)
                *audio_a2dp_src.c.obj(.bss.hw_period)
                *audio_a2dp_src.c.obj(.bss.p_snk_data_buf)
                *audio_a2dp_src.c.obj(.bss.pre_timestamp)
                *audio_a2dp_src.c.obj(.bss.seq_num.1)
                *audio_a2dp_src.c.obj(.bss.src_seq_num)
                *audio_a2dp_src.c.obj(.bss.src_timestamp)
                *audio_a2dp_src.c.obj(.bss.timestamp_total)
                *audio_hfp.c.obj(.bss.hfp_voice_nrec_instance)
                *audio_hfp.c.obj(.bss.is_mic_mute)
                *audio_hfp.c.obj(.bss.music_need_resume)
                *audio_playback.c.obj(.bss.app_playback_time_id)
                *audio_playback.c.obj(.bss.g_curr_song)
                *audio_playback.c.obj(.bss.g_playback_put_data_time_ms)
                *audio_playback.c.obj(.bss.g_playback_single_preq_pkts)
                *audio_playback.c.obj(.bss.playback_db)
                *audio_playback.c.obj(.bss.playback_track_handle)
                *audio_playback.c.obj(.bss.timer_idx_playback_put_data)
                *audio_record.c.obj(.bss.audio_record_timer_id)
                *audio_record.c.obj(.bss.play_dat)
                *audio_record.c.obj(.bss.read_data)
                *audio_record.c.obj(.bss.record_dat)
                *audio_record.c.obj(.bss.record_file_path)
                *audio_record.c.obj(.bss.record_play_level_high)
                *audio_record.c.obj(.bss.seq_num.0)
                *audio_record.c.obj(.bss.seq_num.1)
                *audio_record.c.obj(.bss.timer_idx_record_update_time)
                *audio_record.c.obj(.bss.timer_idx_record_write)
                *dfu_app.c.obj(.bss.dfu_gap_conn_state)
                *dfu_app.c.obj(.bss.dfu_gap_cur_state)
                *dfu_common.c.obj(.bss.user_data_valid_bitmap)
                *dfu_common.c.obj(.bss.valid_bitmap)
                *dfu_main.c.obj(.bss.is_normal_ota_mode)
                *dfu_main.c.obj(.bss.normal_ota_total_timer_handle)
                *dfu_main.c.obj(.bss.normal_ota_wait4_conn_timer_handle)
                *dfu_main.c.obj(.bss.rtk_dfu_service_id)
                *dfu_service.c.obj(.bss.dfu_srv_id_local)
                *dfu_service.c.obj(.bss.p_fn_dfu_service_cb)
                *dfu_task.c.obj(.bss.dfu_evt_queue_handle)
                *dfu_task.c.obj(.bss.dfu_io_queue_handle)
                *dfu_task.c.obj(.bss.dfu_task_handle)
                *dfu_transport.c.obj(.bss.device_info)
                *dfu_transport.c.obj(.bss.dfu_active_reset_pending)
                *dfu_transport.c.obj(.bss.dfu_active_reset_to_ota_mode)
                *dfu_transport.c.obj(.bss.dfu_switch_to_ota_mode_pending)
                *dfu_transport.c.obj(.bss.ota_info)
                *dfu_transport.c.obj(.bss.ota_struct)
                *dfu_transport.c.obj(.bss.section_size)
                *dfu_transport.c.obj(.bss.temp_image_total_num)
                *gui_port_os.c.obj(.bss.ble_bond_new_device_flag)
                *gui_port_os.c.obj(.bss.fb_skip_count)
                *gui_port_os.c.obj(.bss.gui_delay_timer)
                *gui_port_os.c.obj(.bss.port_mem_heap)
                *gui_port_os.c.obj(.bss.time_ms_last.1)
                *gui_port_os.c.obj(.bss.time_ms_record.0)
                *ota_service.c.obj(.bss.device_info)
                *ota_service.c.obj(.bss.img_ver)
                *ota_service.c.obj(.bss.mac_addr)
                *ota_service.c.obj(.bss.p_fn_ota_service_cb)
                *ota_service.c.obj(.bss.section_size)
                *record_playlist.c.obj(.bss.name_buf)
                *record_playlist.c.obj(.bss.play_index)
                *record_playlist.c.obj(.bss.record_scan_hdl)
                . = ALIGN(4);
 } > SRAM
        __sram_bss_reloc_end = .;
        __sram_bss_reloc_start = ADDR(.sram_bss_reloc);
        __sram_bss_reloc_size = __sram_bss_reloc_end - __sram_bss_reloc_start;
 .sram_noinit_reloc (NOLOAD) :
        {
                . = ALIGN(4);
                KEEP(*fat_fs.c.obj(.noinit.*WEST_TOPDIR/zephyr/subsys/fs/fat_fs.c*.k_mem_slab_buf_fatfs_dirp_pool))
                KEEP(*fat_fs.c.obj(.noinit.*WEST_TOPDIR/zephyr/subsys/fs/fat_fs.c*.k_mem_slab_buf_fatfs_filep_pool))
                KEEP(*osif_zephyr.c.obj(.noinit.*WEST_TOPDIR/zephyr/soc/realtek/bee/rtl87x3g/osif/zephyr/osif_zephyr.c*.k_mem_slab_buf_osif_task_slab))
                KEEP(*osif_zephyr.c.obj(.noinit.*WEST_TOPDIR/zephyr/soc/realtek/bee/rtl87x3g/osif/zephyr/osif_zephyr.c*.k_mem_slab_buf_osif_timer_slab))
                . = ALIGN(4);
 } > SRAM AT > SRAM
        __sram_noinit_reloc_end = .;
        __sram_noinit_reloc_start = ADDR(.sram_noinit_reloc);
        __sram_noinit_reloc_size = __sram_noinit_reloc_end - __sram_noinit_reloc_start;
    text :
 {
 __text_region_start = .;
 *(.text)
 *(".text.*")
 *(".TEXT.*")
 *(.gnu.linkonce.t.*)
 *(.glue_7t) *(.glue_7) *(.vfp11_veneer) *(.v4_bx)
 . = ALIGN(4);
 } > FLASH
 __text_region_end = .;
 .ARM.extab :
 {
 *(.ARM.extab* .gnu.linkonce.armextab.*)
 } > FLASH
 .ARM.exidx :
 {
 __exidx_start = .;
 *(.ARM.exidx* gnu.linkonce.armexidx.*)
 __exidx_end = .;
 } > FLASH
 __rodata_region_start = .;
 initlevel :
 {
  __init_start = .;
  __init_EARLY_start = .; KEEP(*(SORT(.z_init_EARLY?_*))); KEEP(*(SORT(.z_init_EARLY??_*)));
  __init_PRE_KERNEL_1_start = .; KEEP(*(SORT(.z_init_PRE_KERNEL_1?_*))); KEEP(*(SORT(.z_init_PRE_KERNEL_1??_*)));
  __init_PRE_KERNEL_2_start = .; KEEP(*(SORT(.z_init_PRE_KERNEL_2?_*))); KEEP(*(SORT(.z_init_PRE_KERNEL_2??_*)));
  __init_POST_KERNEL_start = .; KEEP(*(SORT(.z_init_POST_KERNEL?_*))); KEEP(*(SORT(.z_init_POST_KERNEL??_*)));
  __init_APPLICATION_start = .; KEEP(*(SORT(.z_init_APPLICATION?_*))); KEEP(*(SORT(.z_init_APPLICATION??_*)));
  __init_SMP_start = .; KEEP(*(SORT(.z_init_SMP?_*))); KEEP(*(SORT(.z_init_SMP??_*)));
  __init_end = .;
  __deferred_init_list_start = .;
  KEEP(*(.z_deferred_init*))
  __deferred_init_list_end = .;
 } > FLASH
 device_area : SUBALIGN(4) { _device_list_start = .; KEEP(*(SORT(._device.static.*_?_*))); KEEP(*(SORT(._device.static.*_??_*))); _device_list_end = .; } > FLASH
 initlevel_error :
 {
  KEEP(*(SORT(.z_init_[_A-Z0-9]*)))
 }
 ASSERT(SIZEOF(initlevel_error) == 0, "Undefined initialization levels used.")
 app_shmem_regions : ALIGN_WITH_INPUT
 {
  __app_shmem_regions_start = .;
  KEEP(*(SORT(.app_regions.*)));
  __app_shmem_regions_end = .;
 } > FLASH
 k_p4wq_initparam_area : SUBALIGN(4) { _k_p4wq_initparam_list_start = .; KEEP(*(SORT_BY_NAME(._k_p4wq_initparam.static.*))); _k_p4wq_initparam_list_end = .; } > FLASH
 _static_thread_data_area : SUBALIGN(4) { __static_thread_data_list_start = .; KEEP(*(SORT_BY_NAME(.__static_thread_data.static.*))); __static_thread_data_list_end = .; } > FLASH
 device_deps : ALIGN_WITH_INPUT
 {
__device_deps_start = .;
KEEP(*(SORT(.__device_deps_pass2*)));
__device_deps_end = .;
 } > FLASH
ztest :
{
 _ztest_expected_result_entry_list_start = .; KEEP(*(SORT_BY_NAME(._ztest_expected_result_entry.static.*))); _ztest_expected_result_entry_list_end = .;
 _ztest_suite_node_list_start = .; KEEP(*(SORT_BY_NAME(._ztest_suite_node.static.*))); _ztest_suite_node_list_end = .;
 _ztest_unit_test_list_start = .; KEEP(*(SORT_BY_NAME(._ztest_unit_test.static.*))); _ztest_unit_test_list_end = .;
 _ztest_test_rule_list_start = .; KEEP(*(SORT_BY_NAME(._ztest_test_rule.static.*))); _ztest_test_rule_list_end = .;
} > FLASH
 ctors :
 {
  . = ALIGN(4);
  __ZEPHYR_CTOR_LIST__ = .;
  LONG((__ZEPHYR_CTOR_END__ - __ZEPHYR_CTOR_LIST__) / 4 - 2)
  KEEP(*(SORT_BY_NAME(".ctors*")))
  __CTOR_LIST__ = .;
  LONG(0)
  __ZEPHYR_CTOR_END__ = .;
  LONG(0)
  __CTOR_END__ = .;
 } > FLASH
 init_array :
 {
  . = ALIGN(4);
  __init_array_start = .;
  __init_array_end = .;
  __zephyr_init_array_start = .;
  KEEP(*(SORT_BY_NAME(".init_array*")))
  __zephyr_init_array_end = .;
 } > FLASH
 bt_l2cap_fixed_chan_area : SUBALIGN(4) { _bt_l2cap_fixed_chan_list_start = .; KEEP(*(SORT_BY_NAME(._bt_l2cap_fixed_chan.static.*))); _bt_l2cap_fixed_chan_list_end = .; } > FLASH
 bt_gatt_service_static_area : SUBALIGN(4) { _bt_gatt_service_static_list_start = .; KEEP(*(SORT_BY_NAME(._bt_gatt_service_static.static.*))); _bt_gatt_service_static_list_end = .; } > FLASH
 log_strings_area : SUBALIGN(4) { _log_strings_list_start = .; KEEP(*(SORT_BY_NAME(._log_strings.static.*))); _log_strings_list_end = .; } > FLASH
 log_const_area : SUBALIGN(4) { _log_const_list_start = .; KEEP(*(SORT_BY_NAME(._log_const.static.*))); _log_const_list_end = .; } > FLASH
 log_backend_area : SUBALIGN(4) { _log_backend_list_start = .; KEEP(*(SORT_BY_NAME(._log_backend.static.*))); _log_backend_list_end = .; } > FLASH
 log_link_area : SUBALIGN(4) { _log_link_list_start = .; KEEP(*(SORT_BY_NAME(._log_link.static.*))); _log_link_list_end = .; } > FLASH
 tracing_backend_area : SUBALIGN(4) { _tracing_backend_list_start = .; KEEP(*(SORT_BY_NAME(._tracing_backend.static.*))); _tracing_backend_list_end = .; } > FLASH
 zephyr_dbg_info : ALIGN_WITH_INPUT
 {
  KEEP(*(".dbg_thread_info"));
 } > FLASH
 intc_table_area : SUBALIGN(4) { _intc_table_list_start = .; KEEP(*(SORT_BY_NAME(._intc_table.static.*))); _intc_table_list_end = .; } > FLASH
 symbol_to_keep : ALIGN_WITH_INPUT
 {
  __symbol_to_keep_start = .;
  KEEP(*(SORT(.symbol_to_keep*)));
  __symbol_to_keep_end = .;
 } > FLASH
 shell_area : SUBALIGN(4) { _shell_list_start = .; KEEP(*(SORT_BY_NAME(._shell.static.*))); _shell_list_end = .; } > FLASH
 shell_root_cmds_area : SUBALIGN(4) { _shell_root_cmds_list_start = .; KEEP(*(SORT_BY_NAME(._shell_root_cmds.static.*))); _shell_root_cmds_list_end = .; } > FLASH
 shell_subcmds_area : SUBALIGN(4) { _shell_subcmds_list_start = .; KEEP(*(SORT_BY_NAME(._shell_subcmds.static.*))); _shell_subcmds_list_end = .; } > FLASH
 shell_dynamic_subcmds_area : SUBALIGN(4) { _shell_dynamic_subcmds_list_start = .; KEEP(*(SORT_BY_NAME(._shell_dynamic_subcmds.static.*))); _shell_dynamic_subcmds_list_end = .; } > FLASH
 cfb_font_area : SUBALIGN(4) { _cfb_font_list_start = .; KEEP(*(SORT_BY_NAME(._cfb_font.static.*))); _cfb_font_list_end = .; } > FLASH
_RTK_FLASH_CODE_SECTION :
{
    KEEP(*(.isr.text))
    KEEP(*(SORT(.honeygui$init*)))
 . = ALIGN(4);
} > FLASH
    rodata :
 {
 *(.rodata)
 *(".rodata.*")
 *(.gnu.linkonce.r.*)
 . = ALIGN(4);
 } > FLASH
 .gcc_except_table : ONLY_IF_RO
 {
 *(.gcc_except_table .gcc_except_table.*)
 } > FLASH
 __rodata_region_end = .;
 . = ALIGN(_region_min_align);
 __rom_region_end = __rom_region_start + . - ADDR(rom_start);
   
    /DISCARD/ : {
 *(.got.plt)
 *(.igot.plt)
 *(.got)
 *(.igot)
 }
   
 . = 0x2000b800;
 . = ALIGN(_region_min_align);
 _image_ram_start = .;
.ramfunc : ALIGN_WITH_INPUT
{
 __ramfunc_region_start = .;
 . = ALIGN(_region_min_align);
 __ramfunc_start = .;
 *(.ramfunc)
 *(".ramfunc.*")
 . = ALIGN(_region_min_align);
 __ramfunc_end = .;
} > RAM AT > FLASH
__ramfunc_size = __ramfunc_end - __ramfunc_start;
__ramfunc_load_start = LOADADDR(.ramfunc);
   
    datas : ALIGN_WITH_INPUT
 {
 __data_region_start = .;
 __data_start = .;
 *(.data)
 *(".data.*")
 *(".kernel.*")
 __data_end = .;
 } > RAM AT > FLASH
    __data_size = __data_end - __data_start;
    __data_load_start = LOADADDR(datas);
    __data_region_load_start = LOADADDR(datas);
 sw_isr_table : ALIGN_WITH_INPUT
 {
  . = ALIGN(4);
  *(.gnu.linkonce.sw_isr_table*)
 } > RAM AT > FLASH
        device_states : ALIGN_WITH_INPUT
        {
                __device_states_start = .;
  KEEP(*(".z_devstate"));
  KEEP(*(".z_devstate.*"));
                __device_states_end = .;
        } > RAM AT > FLASH
 pm_device_slots_area : ALIGN_WITH_INPUT { _pm_device_slots_list_start = .; KEEP(*(SORT_BY_NAME(._pm_device_slots.static.*))); _pm_device_slots_list_end = .; } > RAM AT > FLASH
 log_mpsc_pbuf_area : ALIGN_WITH_INPUT { _log_mpsc_pbuf_list_start = .; *(SORT_BY_NAME(._log_mpsc_pbuf.static.*)); _log_mpsc_pbuf_list_end = .; } > RAM AT > FLASH
 log_msg_ptr_area : ALIGN_WITH_INPUT { _log_msg_ptr_list_start = .; KEEP(*(SORT_BY_NAME(._log_msg_ptr.static.*))); _log_msg_ptr_list_end = .; } > RAM AT > FLASH
 log_dynamic_area : ALIGN_WITH_INPUT { _log_dynamic_list_start = .; KEEP(*(SORT_BY_NAME(._log_dynamic.static.*))); _log_dynamic_list_end = .; } > RAM AT > FLASH
 k_timer_area : ALIGN_WITH_INPUT { _k_timer_list_start = .; *(SORT_BY_NAME(._k_timer.static.*)); _k_timer_list_end = .; } > RAM AT > FLASH
 k_mem_slab_area : ALIGN_WITH_INPUT { _k_mem_slab_list_start = .; *(SORT_BY_NAME(._k_mem_slab.static.*)); _k_mem_slab_list_end = .; } > RAM AT > FLASH
 k_heap_area : ALIGN_WITH_INPUT { _k_heap_list_start = .; *(SORT_BY_NAME(._k_heap.static.*)); _k_heap_list_end = .; } > RAM AT > FLASH
 k_mutex_area : ALIGN_WITH_INPUT { _k_mutex_list_start = .; *(SORT_BY_NAME(._k_mutex.static.*)); _k_mutex_list_end = .; } > RAM AT > FLASH
 k_stack_area : ALIGN_WITH_INPUT { _k_stack_list_start = .; *(SORT_BY_NAME(._k_stack.static.*)); _k_stack_list_end = .; } > RAM AT > FLASH
 k_msgq_area : ALIGN_WITH_INPUT { _k_msgq_list_start = .; *(SORT_BY_NAME(._k_msgq.static.*)); _k_msgq_list_end = .; } > RAM AT > FLASH
 k_mbox_area : ALIGN_WITH_INPUT { _k_mbox_list_start = .; *(SORT_BY_NAME(._k_mbox.static.*)); _k_mbox_list_end = .; } > RAM AT > FLASH
 k_pipe_area : ALIGN_WITH_INPUT { _k_pipe_list_start = .; *(SORT_BY_NAME(._k_pipe.static.*)); _k_pipe_list_end = .; } > RAM AT > FLASH
 k_sem_area : ALIGN_WITH_INPUT { _k_sem_list_start = .; *(SORT_BY_NAME(._k_sem.static.*)); _k_sem_list_end = .; } > RAM AT > FLASH
 k_event_area : ALIGN_WITH_INPUT { _k_event_list_start = .; *(SORT_BY_NAME(._k_event.static.*)); _k_event_list_end = .; } > RAM AT > FLASH
 k_queue_area : ALIGN_WITH_INPUT { _k_queue_list_start = .; *(SORT_BY_NAME(._k_queue.static.*)); _k_queue_list_end = .; } > RAM AT > FLASH
 k_fifo_area : ALIGN_WITH_INPUT { _k_fifo_list_start = .; *(SORT_BY_NAME(._k_fifo.static.*)); _k_fifo_list_end = .; } > RAM AT > FLASH
 k_lifo_area : ALIGN_WITH_INPUT { _k_lifo_list_start = .; *(SORT_BY_NAME(._k_lifo.static.*)); _k_lifo_list_end = .; } > RAM AT > FLASH
 k_condvar_area : ALIGN_WITH_INPUT { _k_condvar_list_start = .; *(SORT_BY_NAME(._k_condvar.static.*)); _k_condvar_list_end = .; } > RAM AT > FLASH
 sys_mem_blocks_ptr_area : ALIGN_WITH_INPUT { _sys_mem_blocks_ptr_list_start = .; *(SORT_BY_NAME(._sys_mem_blocks_ptr.static.*)); _sys_mem_blocks_ptr_list_end = .; } > RAM AT > FLASH
 net_buf_pool_area : ALIGN_WITH_INPUT { _net_buf_pool_list_start = .; KEEP(*(SORT_BY_NAME(._net_buf_pool.static.*))); _net_buf_pool_list_end = .; } > RAM AT > FLASH
 .gcc_except_table : ALIGN_WITH_INPUT
 {
 *(.gcc_except_table .gcc_except_table.*)
 } > RAM AT > FLASH
    __data_region_end = .;
   bss (NOLOAD) : ALIGN_WITH_INPUT
 {
        . = ALIGN(4);
 __bss_start = .;
 __kernel_ram_start = .;
 *(.bss)
 *(".bss.*")
 *(COMMON)
 *(".kernel_bss.*")
 __bss_end = ALIGN(4);
 } > RAM AT > RAM
    noinit (NOLOAD) :
        {
        *(.noinit)
        *(".noinit.*")
 *(".kernel_noinit.*")
        } > RAM AT > RAM
    __kernel_ram_end = 0x2000b800 + (81 * 1K);
    __kernel_ram_size = __kernel_ram_end - __kernel_ram_start;

 .itcm : SUBALIGN(4)
 {
  __itcm_start = .;
  *(.itcm)
  *(".itcm.*")
KEEP(*(.ram_text))
  __itcm_end = .;
 } > ITCM AT> FLASH
 __itcm_size = __itcm_end - __itcm_start;
 __itcm_load_start = LOADADDR(.itcm);

.app_ble_service_info.start :
{
    KEEP(*(.app_ble_service_info.start))
} > FLASH
.app_ble_service_info :
{
    KEEP(*(.app_ble_service_info))
} > FLASH
.app_ble_service_info.end :
{
    KEEP(*(.app_ble_service_info.end))
} > FLASH
.app_module_init.start :
{
    KEEP(*(.app_module_init.start))
} > FLASH
.app_module_init :
{
    KEEP(*(SORT(.app_module_init*)))
} > FLASH
.app_module_init.end :
{
    KEEP(*(.app_module_init.end))
} > FLASH

 DSPRAM_DATA : ALIGN_WITH_INPUT
 {
  . = ALIGN(4);
  __dspram_data_start = .;
  *(.dspram_data)
  *(.dspram_text)
  *(".dspram_text*")
  *(".dspram_data*")
  __dspram_data_end = ALIGN(4);
 } > DSPRAM AT > FLASH
 DSPRAM_BSS (NOLOAD) : ALIGN_WITH_INPUT
 {
  . = ALIGN(4);
  __dspram_bss_start = .;
  *(.dspram_bss)
  *(".dspram_bss*")
  __dspram_bss_end = ALIGN(4);
 } > DSPRAM AT > DSPRAM
 __dspram_data_load_start = LOADADDR(DSPRAM_DATA);


 SRAM_DATA : ALIGN_WITH_INPUT
 {
  . = ALIGN(4);
  __sram_data_start = .;
  *(.sram_data)
  *(.sram_text)
  *(".sram_data*")
  *(".sram_text*")
  __sram_data_end = ALIGN(4);
 } > SRAM AT > FLASH
 SRAM_BSS (NOLOAD) : ALIGN_WITH_INPUT
 {
  . = ALIGN(4);
  __sram_bss_start = .;
  *(.sram_bss)
  *(".sram_bss*")
  __sram_bss_end = ALIGN(4);
 } > SRAM AT > SRAM
 __sram_data_load_start = LOADADDR(SRAM_DATA);


 REALTEK_LOG_TRACE : SUBALIGN(4)
 {
  __trace_start = .;
  KEEP(* (.TRACE_HEADER))
  *(.TRACE)
  *(".TRACE.*")
  __trace_end = .;
 } > TRACE

/DISCARD/ :
{
 KEEP(*(.irq_info*))
 KEEP(*(.intList*))
}
    .last_ram_section (NOLOAD) :
    {
 _image_ram_end = .;
 _image_ram_size = _image_ram_end - _image_ram_start;
 _end = .;
 z_mapped_end = .;
    } > RAM AT > RAM
   
 .stab 0 : { *(.stab) }
 .stabstr 0 : { *(.stabstr) }
 .stab.excl 0 : { *(.stab.excl) }
 .stab.exclstr 0 : { *(.stab.exclstr) }
 .stab.index 0 : { *(.stab.index) }
 .stab.indexstr 0 : { *(.stab.indexstr) }
 .gnu.build.attributes 0 : { *(.gnu.build.attributes .gnu.build.attributes.*) }
 .comment 0 : { *(.comment) }
 .debug 0 : { *(.debug) }
 .line 0 : { *(.line) }
 .debug_srcinfo 0 : { *(.debug_srcinfo) }
 .debug_sfnames 0 : { *(.debug_sfnames) }
 .debug_aranges 0 : { *(.debug_aranges) }
 .debug_pubnames 0 : { *(.debug_pubnames) }
 .debug_info 0 : { *(.debug_info .gnu.linkonce.wi.*) }
 .debug_abbrev 0 : { *(.debug_abbrev) }
 .debug_line 0 : { *(.debug_line .debug_line.* .debug_line_end ) }
 .debug_frame 0 : { *(.debug_frame) }
 .debug_str 0 : { *(.debug_str) }
 .debug_loc 0 : { *(.debug_loc) }
 .debug_macinfo 0 : { *(.debug_macinfo) }
 .debug_weaknames 0 : { *(.debug_weaknames) }
 .debug_funcnames 0 : { *(.debug_funcnames) }
 .debug_typenames 0 : { *(.debug_typenames) }
 .debug_varnames 0 : { *(.debug_varnames) }
 .debug_pubtypes 0 : { *(.debug_pubtypes) }
 .debug_ranges 0 : { *(.debug_ranges) }
 .debug_addr 0 : { *(.debug_addr) }
 .debug_line_str 0 : { *(.debug_line_str) }
 .debug_loclists 0 : { *(.debug_loclists) }
 .debug_macro 0 : { *(.debug_macro) }
 .debug_names 0 : { *(.debug_names) }
 .debug_rnglists 0 : { *(.debug_rnglists) }
 .debug_str_offsets 0 : { *(.debug_str_offsets) }
 .debug_sup 0 : { *(.debug_sup) }
    /DISCARD/ : { *(.note.GNU-stack) }
    .ARM.attributes 0 :
 {
 KEEP(*(.ARM.attributes))
 KEEP(*(.gnu.attributes))
 }
    TRACE (NOLOAD) : { __TRACE_start = .; KEEP(*(TRACE)) KEEP(*(TRACE.*)) __TRACE_end = .; } > TRACE __TRACE_size = __TRACE_end - __TRACE_start; __TRACE_load_start = LOADADDR(TRACE); ITCM (NOLOAD) : { __ITCM_start = .; KEEP(*(ITCM)) KEEP(*(ITCM.*)) __ITCM_end = .; } > ITCM __ITCM_size = __ITCM_end - __ITCM_start; __ITCM_load_start = LOADADDR(ITCM); SRAM (NOLOAD) : { __SRAM_start = .; KEEP(*(SRAM)) KEEP(*(SRAM.*)) __SRAM_end = .; } > SRAM __SRAM_size = __SRAM_end - __SRAM_start; __SRAM_load_start = LOADADDR(SRAM); DSPRAM (NOLOAD) : { __DSPRAM_start = .; KEEP(*(DSPRAM)) KEEP(*(DSPRAM.*)) __DSPRAM_end = .; } > DSPRAM __DSPRAM_size = __DSPRAM_end - __DSPRAM_start; __DSPRAM_load_start = LOADADDR(DSPRAM); PSRAM0_MCU (NOLOAD) : { __PSRAM0_MCU_start = .; KEEP(*(PSRAM0_MCU)) KEEP(*(PSRAM0_MCU.*)) __PSRAM0_MCU_end = .; } > PSRAM0_MCU __PSRAM0_MCU_size = __PSRAM0_MCU_end - __PSRAM0_MCU_start; __PSRAM0_MCU_load_start = LOADADDR(PSRAM0_MCU); PSRAM1_DSP (NOLOAD) : { __PSRAM1_DSP_start = .; KEEP(*(PSRAM1_DSP)) KEEP(*(PSRAM1_DSP.*)) __PSRAM1_DSP_end = .; } > PSRAM1_DSP __PSRAM1_DSP_size = __PSRAM1_DSP_end - __PSRAM1_DSP_start; __PSRAM1_DSP_load_start = LOADADDR(PSRAM1_DSP); PSRAM1_MCU (NOLOAD) : { __PSRAM1_MCU_start = .; KEEP(*(PSRAM1_MCU)) KEEP(*(PSRAM1_MCU.*)) __PSRAM1_MCU_end = .; } > PSRAM1_MCU __PSRAM1_MCU_size = __PSRAM1_MCU_end - __PSRAM1_MCU_start; __PSRAM1_MCU_load_start = LOADADDR(PSRAM1_MCU); PSRAM1_NC (NOLOAD) : { __PSRAM1_NC_start = .; KEEP(*(PSRAM1_NC)) KEEP(*(PSRAM1_NC.*)) __PSRAM1_NC_end = .; } > PSRAM1_NC __PSRAM1_NC_size = __PSRAM1_NC_end - __PSRAM1_NC_start; __PSRAM1_NC_load_start = LOADADDR(PSRAM1_NC);
.last_section :
{
  LONG(0xE015E015)
} > FLASH
_flash_used = LOADADDR(.last_section) + SIZEOF(.last_section) - __rom_region_start;
    }
