#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

set(SCRIPT_PATH "${CMAKE_CURRENT_LIST_DIR}/../script/prebuild/process_ui_resource.py")
message(STATUS "pre process ui scipt path: ${SCRIPT_PATH}")

# app_target: application target name. e.g. "app", "guiLib"
# input_file: source file to copy
# output_dir: destination directory
# dependency_target: optional target that must finish before copy runs
function(honeygui_resource_copy 
        app_target 
        input_file 
        output_dir)
        
    if(NOT EXISTS "${input_file}")
        message(WARNING "[PreBuild][UI Copy] source file not found: ${input_file}")
    endif()

    # get filename without extension and convert to valid C identifier for target naming
    get_filename_component(input_name_we "${input_file}" NAME_WE)
    get_filename_component(input_file_name "${input_file}" NAME)
    string(MAKE_C_IDENTIFIER "${input_name_we}" input_name_id)
    set(COPY_TARGET_NAME "ui_copy_${app_target}_${input_name_id}")

    # create custom target for copying file
    add_custom_target(${COPY_TARGET_NAME}
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${output_dir}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${output_dir}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${input_file}" "${output_dir}/${input_file_name}"
        COMMENT "[PreBuild][UI Copy] Copied ${input_file_name} to ${output_dir}."
        USES_TERMINAL
    )

    # add dependency to ensure copy runs before app target builds
    add_dependencies(${app_target} ${COPY_TARGET_NAME})
endfunction()

# app_target: application target name. e.g. "app", "guiLib"
# userdata_addr: userdata address. e.g. 0x704D1400
# gui_demo: gui demo name. e.g. "410_502"
function(honeygui_resource_preprocess 
        app_target 
        userdata_addr
        gui_demo
        )

    set(PREBUILD_TARGET_NAME "ui_prebuild_${app_target}")
    set(UI_OUTPUT_DIR "${CMAKE_BINARY_DIR}/../honeygui_userdata")

    # add custom target for prebuild ui resource
    add_custom_target(${PREBUILD_TARGET_NAME}
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${UI_OUTPUT_DIR}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${UI_OUTPUT_DIR}"

        COMMAND ${Python3_EXECUTABLE} "${SCRIPT_PATH}"
                --ui_demo "${gui_demo}"
                --ui_base_addr "${userdata_addr}"
                --output "${UI_OUTPUT_DIR}/"
        COMMENT "[PreBuild][UI Process] starting for ${gui_demo} / ${userdata_addr}...\n"

        COMMAND ${CMAKE_COMMAND} -E echo "[PreBuild][UI Process] for ${gui_demo} / ${userdata_addr} completed."
        COMMAND ${CMAKE_COMMAND} -E echo "[PreBuild][UI Process] file exported to ${UI_OUTPUT_DIR}."
        USES_TERMINAL
        )

    # build app target after prebuild target
    add_dependencies(${app_target} ${PREBUILD_TARGET_NAME})

endfunction()