.. _rtl87x3g_psram_sample:

PSRAM usage sample
##################

Overview
********

Demonstrates the four common ways to use the on-package psram on the rtl87x3g
series, each as a small self-checking demo that prints ``[PASS]`` / ``[FAIL]``:

#. **Static / global variables in psram** -- variables placed in the
   ``PSRAM0_MCU`` / ``PSRAM1_MCU`` memory-regions via the section macros in
   ``src/platform/psram_section.h``. These regions are ``NOLOAD`` (not zeroed or
   copied at boot) and psram is only powered up in ``app_system_lower_init()``,
   so a variable that needs a zero initial value is cleared at runtime rather
   than relying on implicit zero-init. See ``src/demo/psram_static_var.c``.

#. **psram as a heap (PSRAM0 + PSRAM1)** -- ``psram_heap_init()`` (SoC osif layer,
   ``zephyr/soc/realtek/bee/rtl87x3g/osif/zephyr/osif_zephyr.c``, declared in
   ``osif_zephyr.h``) registers ``heap_psram0`` / ``heap_psram1`` into the SoC
   multi-heap under ``RAM_TYPE_PSRAM0`` / ``RAM_TYPE_PSRAM1``. The two nodes are
   checked separately with ``DT_NODE_EXISTS()``; a missing node is reported at
   runtime and only that one psram heap stays unavailable. ``os_mem_alloc()`` / ``os_mem_zalloc()`` /
   ``os_mem_aligned_alloc()`` with ``OS_MEM_TYPE_PSRAM0`` / ``OS_MEM_TYPE_PSRAM1``
   then allocate from those heaps. The demo also does a heap "static check" --
   ``os_mem_peek(OS_MEM_TYPE_PSRAM0 / OS_MEM_TYPE_PSRAM1)`` returns the free bytes
   of each psram heap (the ``RAM_TYPE_PSRAM0`` / ``RAM_TYPE_PSRAM1`` cases of
   ``os_mem_peek_zephyr()``; a heap whose devicetree node is missing, or that
   ``psram_heap_init()`` has not registered yet, peeks as 0), and the heap size
   comes from the devicetree node. See ``src/demo/psram_heap_demo.c``.

#. **psram as a thread stack** (``src/demo/psram_stack.c``) -- two variants, both
   creating the task at runtime:

   * *3.1 dynamic task + dynamic stack* -- ``psram_task_create()`` /
     ``psram_task_delete()`` (implemented in the same file, mirroring
     ``os_task_create()``) allocate both the task control block and the thread
     stack from a psram heap; the ram_type argument selects PSRAM0 or PSRAM1.
   * *3.2 dynamic task + static stack* -- the ``struct k_thread`` is allocated at
     runtime from a psram heap, the stack is a build-time array placed in the
     ``PSRAM0_MCU`` region (``Z_KERNEL_STACK_DEFINE_IN``), and the thread is
     created with ``k_thread_create()``.

   Dynamic psram stacks are 8-byte aligned; HW stack protection on this
   Cortex-M55 uses ``PSPLIM``, so no power-of-2 alignment is required (same as
   the SoC ``os_task_create()`` path).

   **A compile-time thread cannot use a psram stack.** A statically defined
   thread (``K_THREAD_DEFINE``) has its stack set up by the kernel during boot
   (``z_init_static_threads()`` -> ``z_setup_new_thread()`` writes the initial
   stack frame) -- before ``main()`` runs ``app_system_lower_init()`` to power
   the psram controller. Writing that stack into un-powered psram would fault,
   and ``K_THREAD_DEFINE`` offers no way to relocate its stack to a psram
   section. A psram thread stack must therefore be created at runtime, as the
   variants above do; a compile-time thread has to keep its stack in regular RAM.

#. **psram cache configuration -- cacheable vs non-cacheable** -- the cache/MPU
   setup is done in ``app_system_lower_init()`` -> ``app_mpu_config()``
   (``src/app_lower_init.c``). Each psram chip's first 2 MB is split into a 1 MB
   cacheable region (MPU attr ``0xAA``, write-through) and a 1 MB non-cacheable
   region (MPU attr ``0x44``); on this SoC both map to the same address, so
   cacheability is decided purely by which MPU region the address lands in.
   ARMv8-M MPU regions must not overlap. Region numbers are chosen from the free
   slots reported by the boot MPU map (dumped by ``dump_mpu_regions()`` on the
   console before/after config): psram0 uses regions 2 (cacheable, overwriting
   the boot external-memory region) and 6 (non-cacheable); psram1 uses regions 4
   and 5. Region 3 is left alone -- at boot it holds the live SPIC2 (0x60000000)
   region, so reusing it would silently drop that mapping. The demo puts one
   buffer
   in the cacheable region and one in the non-cacheable region of the same chip,
   verifies the data, and times repeated sequential reads of each -- the
   non-cacheable buffer is several times slower (measured ~6x on hardware)
   because every access goes out to psram instead of the cpu cache. Non-cacheable
   memory is what you want for buffers shared with another bus master (DMA, DSP)
   where stale cache lines would be a hazard. See ``src/demo/psram_cache_demo.c``.

Source layout::

   src/
     main.c                                  entry + `psram` shell commands
     platform/
       app_lower_init.c/.h                   psram power-up + MPU/cache config
       psram_section.h                       PSRAM0/1 _MCU / _NC section macros
     demo/
       psram_demo.h                          demo entry points + psram_task_create/delete
       psram_static_var.c                    [1] static variables in psram
       psram_heap_demo.c                     [2] psram heap + usage peek
       psram_stack.c                         [3] psram thread stacks + psram_task_create/delete
       psram_cache_demo.c                    [4] cacheable vs non-cacheable

The board's psram layout (``boards/rtl87x3g_sample.overlay``) per psram chip:

* ``*_for_mcu`` -- 1 MB cacheable, ``NOLOAD`` (static vars / thread stacks)
* ``*_nc``      -- 1 MB non-cacheable, ``NOLOAD`` (coherent shared buffers)
* ``heap_psram*`` -- 2 MB, the multi-heap backing store

Because these regions are ``NOLOAD`` (not zeroed/copied at boot) and psram is
only powered up in ``app_system_lower_init()``, any variable placed in them must
be initialised at runtime.

Building
********

.. code-block:: console

   west build -b rtl87x3g_sample -p
   west build -b rtl87x3g_sample -S flash_4M_bank0 -p
   west build -b rtl87x3g_sample -S flash_8M_bank0 -p
   west build -b rtl87x3g_sample -S flash_16M_bank0 -p

Flashing
********

.. code-block:: console

   west flash --port=/dev/ttyUSB0 --auto --auto-reset-port=/dev/ttyUSB1

Console is ``uart2`` at 921600 baud (``/dev/ttyUSB0``). ``printf`` output relies
on ``CONFIG_UART_CONSOLE=y`` in ``prj.conf`` (it installs the libc stdout hook);
without a console driver only ``printk`` would appear.

Running
*******

All four demos run once at boot and print a summary::

   ==== PSRAM sample summary: ALL PASS (0 demo group(s) failed) ====

With the shell they can be re-run individually::

   psram all
   psram static
   psram heap
   psram stack
   psram cache
   psram peek

Because ``printf`` (console poll-out) and the shell share ``uart2``, boot output
may interleave slightly with the shell prompt; the content is unaffected.
