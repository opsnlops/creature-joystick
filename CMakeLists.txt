cmake_minimum_required(VERSION 3.24)

# Pull in the Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Pull in FreeRTOS
include($ENV{FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(joystick C CXX ASM)

set(CMAKE_C_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

add_executable(joystick)

target_sources(joystick PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/src/main.c
        ${CMAKE_CURRENT_LIST_DIR}/src/usb_descriptors.c
        )

# Make sure TinyUSB can find tusb_config.h
target_include_directories(joystick PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/src)

# In addition to pico_stdlib required for common PicoSDK functionality, add dependency on tinyusb_device
# for TinyUSB device support and tinyusb_board for the additional board support library used by the example
target_link_libraries(joystick PUBLIC
        FreeRTOS-Kernel
        FreeRTOS-Kernel-Heap4
        pico_stdlib
        tinyusb_device
        tinyusb_board
        )

pico_add_extra_outputs(joystick)

