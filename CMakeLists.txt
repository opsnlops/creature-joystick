cmake_minimum_required(VERSION 3.24)

# Pull in the Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Pull in FreeRTOS
include($ENV{FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(joystick C CXX ASM)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

add_executable(joystick)

target_sources(joystick PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/src/main.c
        ${CMAKE_CURRENT_LIST_DIR}/src/freertos_hook.c
        ${CMAKE_CURRENT_LIST_DIR}/src/display/display.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/display/display_task.c
        ${CMAKE_CURRENT_LIST_DIR}/src/display/display_wrapper.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/joystick/adc.c
        ${CMAKE_CURRENT_LIST_DIR}/src/joystick/filter.c
        ${CMAKE_CURRENT_LIST_DIR}/src/joystick/joystick.c
        ${CMAKE_CURRENT_LIST_DIR}/src/logging/logging.c
        ${CMAKE_CURRENT_LIST_DIR}/src/usb/usb.c
        ${CMAKE_CURRENT_LIST_DIR}/src/usb/usb_descriptors.c
        )

# Make sure TinyUSB can find tusb_config.h
target_include_directories(joystick PUBLIC
        .
        ${CMAKE_CURRENT_LIST_DIR}/src)

add_subdirectory(pico-ssd1306)

# In addition to pico_stdlib required for common PicoSDK functionality, add dependency on tinyusb_device
# for TinyUSB device support and tinyusb_board for the additional board support library used by the example
target_link_libraries(joystick PUBLIC
        FreeRTOS-Kernel
        FreeRTOS-Kernel-Heap4
        pico_stdlib
        hardware_spi
        tinyusb_device
        tinyusb_board
        pico_ssd1306
        )

pico_add_extra_outputs(joystick)

# Enable the console over UART (which is the picoprobe!)
pico_enable_stdio_usb(joystick 0)
pico_enable_stdio_uart(joystick 1)