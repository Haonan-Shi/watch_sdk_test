# Install script for directory: D:/watch/GItCode/zephyr_sdk/RTL87X3G_MCU_SDK_Watch_Zephyr_v1.14.3.1/modules/honeygui/realgui/widget

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/Zephyr-Kernel")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "C:/Users/Haonan_shi/zephyr-sdk-0.16.8/arm-zephyr-eabi/bin/arm-zephyr-eabi-objdump.exe")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("D:/watch/GItCode/zephyr_sdk/RTL87X3G_MCU_SDK_Watch_Zephyr_v1.14.3.1/realtek-app/applications/watch/build/modules/honeygui/realgui/widget/gui_canvas/cmake_install.cmake")
  include("D:/watch/GItCode/zephyr_sdk/RTL87X3G_MCU_SDK_Watch_Zephyr_v1.14.3.1/realtek-app/applications/watch/build/modules/honeygui/realgui/widget/gui_card_list/cmake_install.cmake")
  include("D:/watch/GItCode/zephyr_sdk/RTL87X3G_MCU_SDK_Watch_Zephyr_v1.14.3.1/realtek-app/applications/watch/build/modules/honeygui/realgui/widget/gui_geometry/cmake_install.cmake")
  include("D:/watch/GItCode/zephyr_sdk/RTL87X3G_MCU_SDK_Watch_Zephyr_v1.14.3.1/realtek-app/applications/watch/build/modules/honeygui/realgui/widget/gui_gif/cmake_install.cmake")
  include("D:/watch/GItCode/zephyr_sdk/RTL87X3G_MCU_SDK_Watch_Zephyr_v1.14.3.1/realtek-app/applications/watch/build/modules/honeygui/realgui/widget/gui_glass/cmake_install.cmake")
  include("D:/watch/GItCode/zephyr_sdk/RTL87X3G_MCU_SDK_Watch_Zephyr_v1.14.3.1/realtek-app/applications/watch/build/modules/honeygui/realgui/widget/gui_gray/cmake_install.cmake")
  include("D:/watch/GItCode/zephyr_sdk/RTL87X3G_MCU_SDK_Watch_Zephyr_v1.14.3.1/realtek-app/applications/watch/build/modules/honeygui/realgui/widget/gui_img/cmake_install.cmake")
  include("D:/watch/GItCode/zephyr_sdk/RTL87X3G_MCU_SDK_Watch_Zephyr_v1.14.3.1/realtek-app/applications/watch/build/modules/honeygui/realgui/widget/gui_list/cmake_install.cmake")
  include("D:/watch/GItCode/zephyr_sdk/RTL87X3G_MCU_SDK_Watch_Zephyr_v1.14.3.1/realtek-app/applications/watch/build/modules/honeygui/realgui/widget/gui_lite3d/cmake_install.cmake")
  include("D:/watch/GItCode/zephyr_sdk/RTL87X3G_MCU_SDK_Watch_Zephyr_v1.14.3.1/realtek-app/applications/watch/build/modules/honeygui/realgui/widget/gui_lite_video/cmake_install.cmake")
  include("D:/watch/GItCode/zephyr_sdk/RTL87X3G_MCU_SDK_Watch_Zephyr_v1.14.3.1/realtek-app/applications/watch/build/modules/honeygui/realgui/widget/gui_lottie/cmake_install.cmake")
  include("D:/watch/GItCode/zephyr_sdk/RTL87X3G_MCU_SDK_Watch_Zephyr_v1.14.3.1/realtek-app/applications/watch/build/modules/honeygui/realgui/widget/gui_menu_cellular/cmake_install.cmake")
  include("D:/watch/GItCode/zephyr_sdk/RTL87X3G_MCU_SDK_Watch_Zephyr_v1.14.3.1/realtek-app/applications/watch/build/modules/honeygui/realgui/widget/gui_obj/cmake_install.cmake")
  include("D:/watch/GItCode/zephyr_sdk/RTL87X3G_MCU_SDK_Watch_Zephyr_v1.14.3.1/realtek-app/applications/watch/build/modules/honeygui/realgui/widget/gui_particle/cmake_install.cmake")
  include("D:/watch/GItCode/zephyr_sdk/RTL87X3G_MCU_SDK_Watch_Zephyr_v1.14.3.1/realtek-app/applications/watch/build/modules/honeygui/realgui/widget/gui_qbcode/cmake_install.cmake")
  include("D:/watch/GItCode/zephyr_sdk/RTL87X3G_MCU_SDK_Watch_Zephyr_v1.14.3.1/realtek-app/applications/watch/build/modules/honeygui/realgui/widget/gui_soccer/cmake_install.cmake")
  include("D:/watch/GItCode/zephyr_sdk/RTL87X3G_MCU_SDK_Watch_Zephyr_v1.14.3.1/realtek-app/applications/watch/build/modules/honeygui/realgui/widget/gui_stream/cmake_install.cmake")
  include("D:/watch/GItCode/zephyr_sdk/RTL87X3G_MCU_SDK_Watch_Zephyr_v1.14.3.1/realtek-app/applications/watch/build/modules/honeygui/realgui/widget/gui_svg/cmake_install.cmake")
  include("D:/watch/GItCode/zephyr_sdk/RTL87X3G_MCU_SDK_Watch_Zephyr_v1.14.3.1/realtek-app/applications/watch/build/modules/honeygui/realgui/widget/gui_text/cmake_install.cmake")
  include("D:/watch/GItCode/zephyr_sdk/RTL87X3G_MCU_SDK_Watch_Zephyr_v1.14.3.1/realtek-app/applications/watch/build/modules/honeygui/realgui/widget/gui_video/cmake_install.cmake")
  include("D:/watch/GItCode/zephyr_sdk/RTL87X3G_MCU_SDK_Watch_Zephyr_v1.14.3.1/realtek-app/applications/watch/build/modules/honeygui/realgui/widget/gui_view/cmake_install.cmake")
  include("D:/watch/GItCode/zephyr_sdk/RTL87X3G_MCU_SDK_Watch_Zephyr_v1.14.3.1/realtek-app/applications/watch/build/modules/honeygui/realgui/widget/gui_win/cmake_install.cmake")

endif()

