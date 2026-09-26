#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#
# Compile exactly the one application selected by the TINYML_APP choice.  Each
# app owns a same-named main()/model, so only one may be added to the build.

if(CONFIG_TINYML_APP_BENCH)
    set(TINYML_APP_NAME "bench")
    add_subdirectory(bench)
elseif(CONFIG_TINYML_APP_IC)
    set(TINYML_APP_NAME "ic")
    add_subdirectory(ic)
elseif(CONFIG_TINYML_APP_KWS)
    set(TINYML_APP_NAME "kws")
    add_subdirectory(kws)
elseif(CONFIG_TINYML_APP_MOTION)
    set(TINYML_APP_NAME "motion")
    add_subdirectory(motion)
else()
    message(FATAL_ERROR "No TINYML_APP selected (set CONFIG_TINYML_APP_* =y)")
endif()

message(STATUS "=======================================")
message(STATUS "  TinyML app selected: ${TINYML_APP_NAME}")
message(STATUS "=======================================")
