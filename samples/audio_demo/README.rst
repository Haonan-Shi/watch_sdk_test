Overview
********

Audio demo for the rtl87x3g series, exercising the audio pipeline
(passthrough, ANC, KWS, line-in, VAD, track) over the console.

Building
********

Build for the rtl87x3g_sample board. Each flash-size variant is a
separate, verified command:

.. code-block:: console

   west build -b rtl87x3g_sample -p
   west build -b rtl87x3g_sample -S flash_4M_bank0 -p
   west build -b rtl87x3g_sample -S flash_8M_bank0 -p
   west build -b rtl87x3g_sample -S flash_16M_bank0 -p
