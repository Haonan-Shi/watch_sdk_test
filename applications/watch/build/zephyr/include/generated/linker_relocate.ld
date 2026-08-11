

/* Linker section for memory region FLASH for TEXT section  */

	SECTION_PROLOGUE(.flash_text_reloc,,)
        {
                . = ALIGN(4);
                
                KEEP(*prep_c.c.obj(.rel.text.z_prep_c))

                KEEP(*prep_c.c.obj(.text.z_prep_c))

                KEEP(*reset.S.obj(.rel.text._reset_section))

                KEEP(*reset.S.obj(.text._reset_section))

                . = ALIGN(4);
	} 
#ifdef CONFIG_XIP
GROUP_DATA_LINK_IN(FLASH , ROMABLE_REGION)
#else
GROUP_DATA_LINK_IN(FLASH , FLASH )
#endif

        __flash_text_reloc_end = .;
        __flash_text_reloc_start = ADDR(.flash_text_reloc);
        __flash_text_reloc_size = __flash_text_reloc_end - __flash_text_reloc_start;

        __flash_text_rom_start = LOADADDR(.flash_text_reloc);


/* Linker section for memory region ITCM for TEXT section  */

	SECTION_PROLOGUE(.itcm_text_reloc,,)
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
#ifdef CONFIG_XIP
GROUP_DATA_LINK_IN(ITCM , ROMABLE_REGION)
#else
GROUP_DATA_LINK_IN(ITCM , ITCM )
#endif

        __itcm_text_reloc_end = .;
        __itcm_text_reloc_start = ADDR(.itcm_text_reloc);
        __itcm_text_reloc_size = __itcm_text_reloc_end - __itcm_text_reloc_start;

        __itcm_text_rom_start = LOADADDR(.itcm_text_reloc);


/* Linker section for memory region ITCM for RODATA section  */

	SECTION_PROLOGUE(.itcm_rodata_reloc,,)
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

#if 0
                . = ALIGN(0);
#else
                MPU_ALIGN(__itcm_rodata_reloc_size);
#endif
                __itcm_rodata_reloc_end = .;
	} 
#ifdef CONFIG_XIP
GROUP_DATA_LINK_IN(ITCM , ROMABLE_REGION)
#else
GROUP_DATA_LINK_IN(ITCM , ITCM )
#endif

        __itcm_rodata_reloc_size = __itcm_rodata_reloc_end - __itcm_rodata_reloc_start;

        __itcm_rodata_rom_start = LOADADDR(.itcm_rodata_reloc);


/* Linker section for memory region ITCM for DATA section  */

	SECTION_PROLOGUE(.itcm_data_reloc,,)
        {
                . = ALIGN(4);
                
                KEEP(*nmi.c.obj(.data.handler))

                KEEP(*nmi.c.obj(.rel.data.handler))

                . = ALIGN(4);
	} 
#ifdef CONFIG_XIP
GROUP_DATA_LINK_IN(ITCM , ROMABLE_REGION)
#else
GROUP_DATA_LINK_IN(ITCM , ITCM )
#endif

        __itcm_data_reloc_end = .;
        __itcm_data_reloc_start = ADDR(.itcm_data_reloc);
        __itcm_data_reloc_size = __itcm_data_reloc_end - __itcm_data_reloc_start;

        __itcm_data_rom_start = LOADADDR(.itcm_data_reloc);


/* Linker section for memory region ITCM for BSS section  */

	SECTION_PROLOGUE(.itcm_bss_reloc,,)
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
	} GROUP_LINK_IN(ITCM )
        __itcm_bss_reloc_end = .;
        __itcm_bss_reloc_start = ADDR(.itcm_bss_reloc);
        __itcm_bss_reloc_size = __itcm_bss_reloc_end - __itcm_bss_reloc_start;


/* Linker section for memory region SRAM for DATA section  */

	SECTION_PROLOGUE(.sram_data_reloc,,)
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
#ifdef CONFIG_XIP
GROUP_DATA_LINK_IN(SRAM , ROMABLE_REGION)
#else
GROUP_DATA_LINK_IN(SRAM , SRAM )
#endif

        __sram_data_reloc_end = .;
        __sram_data_reloc_start = ADDR(.sram_data_reloc);
        __sram_data_reloc_size = __sram_data_reloc_end - __sram_data_reloc_start;

        __sram_data_rom_start = LOADADDR(.sram_data_reloc);


/* Linker section for memory region SRAM for BSS section  */

	SECTION_PROLOGUE(.sram_bss_reloc,,)
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
	} GROUP_LINK_IN(SRAM )
        __sram_bss_reloc_end = .;
        __sram_bss_reloc_start = ADDR(.sram_bss_reloc);
        __sram_bss_reloc_size = __sram_bss_reloc_end - __sram_bss_reloc_start;


/* Linker section for memory region SRAM for NOINIT section  */

	SECTION_PROLOGUE(.sram_noinit_reloc,(NOLOAD),)
        {
                . = ALIGN(4);
                
                KEEP(*fat_fs.c.obj(.noinit.*WEST_TOPDIR/zephyr/subsys/fs/fat_fs.c*.k_mem_slab_buf_fatfs_dirp_pool))

                KEEP(*fat_fs.c.obj(.noinit.*WEST_TOPDIR/zephyr/subsys/fs/fat_fs.c*.k_mem_slab_buf_fatfs_filep_pool))

                KEEP(*osif_zephyr.c.obj(.noinit.*WEST_TOPDIR/zephyr/soc/realtek/bee/rtl87x3g/osif/zephyr/osif_zephyr.c*.k_mem_slab_buf_osif_task_slab))

                KEEP(*osif_zephyr.c.obj(.noinit.*WEST_TOPDIR/zephyr/soc/realtek/bee/rtl87x3g/osif/zephyr/osif_zephyr.c*.k_mem_slab_buf_osif_timer_slab))

                . = ALIGN(4);
	} GROUP_NOLOAD_LINK_IN(SRAM , SRAM )
        __sram_noinit_reloc_end = .;
        __sram_noinit_reloc_start = ADDR(.sram_noinit_reloc);
        __sram_noinit_reloc_size = __sram_noinit_reloc_end - __sram_noinit_reloc_start;
