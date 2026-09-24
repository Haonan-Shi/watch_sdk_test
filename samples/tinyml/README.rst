Overview
********

TinyML inference demos for the rtl87x3g series (Cortex-M55, TFLite-Micro
runtime). One of four applications is selected at build time via the
``TINYML_APP`` Kconfig choice; the shared inference engine, PSRAM heap and
external-PSRAM bring-up are common to all four (``src/common``).

Each application is described with the same fields -- selection symbol,
scenario, model, feature front-end:

- **bench** -- ``CONFIG_TINYML_APP_BENCH`` -- generic / edge inference
  template; the model and input data are streamed from the host over the
  data UART at runtime (no embedded model); no feature library.
- **ic** -- ``CONFIG_TINYML_APP_IC`` -- RGB image classification
  (handwritten digits 0-9); 96x96 RGB model input; normalization in
  ``ic_normalize.c`` (source, no feature library).
- **kws** -- ``CONFIG_TINYML_APP_KWS`` -- keyword spotting ("hi_realtek"
  streaming wake-word model); log-Mel front-end from the prebuilt
  ``libkws_feature_cm55.a``.
- **motion** -- ``CONFIG_TINYML_APP_MOTION`` -- 3-axis accelerometer motion
  recognition (idle/snake/updown/wave); z-score + int8 quantization in
  ``motion_preprocess.c`` (source, no feature library).

Building
********

Build for the rtl87x3g_sample board. Select exactly one application with
its ``CONFIG_TINYML_APP_*`` symbol (default: ``kws``):

.. code-block:: console

   west build -b rtl87x3g_sample -p -- -DCONFIG_TINYML_APP_BENCH=y
   west build -b rtl87x3g_sample -p -- -DCONFIG_TINYML_APP_IC=y
   west build -b rtl87x3g_sample -p -- -DCONFIG_TINYML_APP_KWS=y
   west build -b rtl87x3g_sample -p -- -DCONFIG_TINYML_APP_MOTION=y
