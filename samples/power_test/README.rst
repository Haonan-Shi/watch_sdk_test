Overview
********

Power test sample for the rtl87x3g series, ported from the Keil ``power_test``
project. Exercises power modes (DLPS/LPS/powerdown), DVFS / CPU / DSP1
frequency, flash stress, RF TX power / continuous-TX / packet-RX,
BR/EDR (A2DP/AVRCP/HFP) and BLE link power tests.

The command interface is the Zephyr shell, running on **uart2 @ 921600 8N1**.
All test commands live under a single root command ``power_test``; type
``power_test`` + <tab> or ``power_test help`` to list sub-commands.

Building
********

.. code-block:: console

   west build -b rtl87x3g_sample -p
   west build -b rtl87x3g_sample -S flash_4M_bank0 -p
   west build -b rtl87x3g_sample -S flash_8M_bank0 -p
   west build -b rtl87x3g_sample -S flash_16M_bank0 -p

Commands
********

All commands are sub-commands of ``power_test``, e.g. ``power_test dvfs high``.

Power state / clocks
====================

======================================  ============================================================
Command                                 Parameters
======================================  ============================================================
``power_test state <mode>``             mode: ``lps`` | ``dlps`` | ``dlps_ret`` | ``dlps_pfm`` |
                                        ``down`` | ``off`` | ``btsleep`` | ``btactive``
``power_test dvfs <level>``             level: ``high`` | ``low`` | ``normal1v1`` | ``normal0v9``
``power_test cpu <freq>``               freq: ``sleep`` | ``active`` | ``625k`` | ``max`` |
                                        <MHz num> (e.g. 20 40 80 100 120 160)
``power_test dsp1 <freq>``              freq: ``disable`` | <MHz num>
                                        (20 40 80 120 140 160 180 200 280 320)
``power_test 32k <on|off>``             on / off the 32k clock in power-down
======================================  ============================================================

flash
=================

======================================  ============================================================
Command                                 Parameters
======================================  ============================================================
``power_test flash <op>``               op: ``write`` | ``read`` | ``erase`` | ``xip`` | ``cache`` |
                                        ``half_cache`` | ``dma_read`` | ``write_prepare`` |
                                        ``erase_prepare``
======================================  ============================================================

RF
==

======================================  ============================================================
Command                                 Parameters
======================================  ============================================================
``power_test txpower <br_1M> <edr_2M>   five int8 values in 0.5 dBm steps
<edr_3M> <le_1M> <le_2M>``
``power_test cont_tx <tx_power>         continuous TX. packet_type: 1DH5:2 2DH5:5 3DH5:8
<packet_type>``                         LE1M:9 LE2M:11
``power_test packet_rx <packet_type>``  packet_type: 1DH5:2 2DH5:5 3DH5:8 LE1M:9 LE2M:11
======================================  ============================================================

BR/EDR (gap_legacy)
===================

Syntax: ``power_test gap_legacy <sub-action> [params...] <a0> <a1> <a2> <a3> <a4> <a5>``.
Every sub-action takes a trailing 6-byte peer bd_addr (space-separated bytes),
even where it is not used, because the argument count is validated.

===============================  =========================================================
Sub-action                       Params (before the 6 addr bytes)
===============================  =========================================================
``inquiry_scan_param_set``       <type> <interval> <window>
``page_scan_param_set``          <type> <interval> <window> <page_timeout>
``radio_mode_set``               <radio_mode>
``sniff_enter``                  <min_interval> <max_interval> <sniff_attempt> <sniff_timeout>
``sniff_exit``                   (none)
``default_link_policy_set``      <link_policy>
``link_policy_set``              <link_policy>
``inquiry_start``                <inquiry_timeout>
``inquiry_stop``                 (none)
``page_start``                   (none)
``page_stop``                    (none)
``hfp_ag_connect``               (none)
``hfp_ag_disconnect``            (none)
``legacy_disconnect``            (none)
``remove_bond``                  (none)
===============================  =========================================================

Example: ``power_test gap_legacy radio_mode_set 3 0x11 0x22 0x33 0x44 0x55 0x66``

BLE (gap_le)
============

Syntax: ``power_test gap_le <sub-action> [params...]``.

======================================  =================================================
Sub-action                              Params
======================================  =================================================
``le_adv_power_test_start``             <adv_interval> <adv_data_length>
``le_adv_power_test_stop``              (none)
``le_conn_power_test_start``            <slave_latency> <conn_interval_min> <conn_interval_max>
``le_conn_power_test_stop``             (none)
``le_scan_power_test_start``            <scan_mode> <scan_interval> <scan_window> <duplicate_enable>
``le_scan_power_test_stop``             (none)
``le_create_conn``                      <a0> <a1> <a2> <a3> <a4> <a5>  (peer bd_addr)
``le_cancel_conn``                      (none)
======================================  =================================================
