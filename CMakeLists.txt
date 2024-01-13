cmake_minimum_required(VERSION 3.24)

# Pull in the Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Pull in FreeRTOS
include($ENV{FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(joystick C CXX ASM)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

# Initialize the SDK
pico_sdk_init()


add_executable(joystick)

target_sources(joystick PUBLIC
        src/main.c
        src/controller-config.h
        src/tusb_config.h
        src/freertos_hook.c
        src/display/display.cpp
        src/display/display.h
        src/display/display_task.c
        src/display/display_task.h
        src/display/display_wrapper.cpp
        src/display/display_wrapper.h
        src/joystick/adc.c
        src/joystick/adc.h
        src/joystick/filter.c
        src/joystick/filter.h
        src/joystick/joystick.c
        src/joystick/joystick.h
        src/logging/logging.c
        src/logging/logging.h
        src/usb/usb.c
        src/usb/usb.h
        src/usb/usb_descriptors.c
        src/usb/usb_descriptors.h
        )

# Make sure TinyUSB can find tusb_config.h
target_include_directories(joystick PUBLIC
        .
        src/)

add_subdirectory(pico-ssd1306)

# In addition to pico_stdlib required for common PicoSDK functionality, add dependency on tinyusb_device
# for TinyUSB device support and tinyusb_board for the additional board support library used by the example
target_link_libraries(joystick PUBLIC
        FreeRTOS-Kernel
        FreeRTOS-Kernel-Heap4
        pico_stdlib
        pico_unique_id
        hardware_spi
        tinyusb_device
        tinyusb_board
        pico_ssd1306
        )

pico_add_extra_outputs(joystick)

# Enable the console over UART (which is the picoprobe!)
pico_enable_stdio_usb(joystick 0)
pico_enable_stdio_uart(joystick 1)



#
# A small app to debug the ADC
#
add_executable(adc-debugger)

target_sources(adc-debugger PUBLIC
        src/freertos_hook.c
        src/joystick/adc.c
        src/joystick/adc.h
        src/joystick/filter.c
        src/joystick/filter.h
        src/joystick/joystick.c
        src/joystick/joystick.h
        src/logging/logging.c
        src/logging/logging.h
        src/adc-debugger/FreeRTOSConfig.h
        src/adc-debugger/adc-debugger.c
        src/adc-debugger/tusb_config.h
        src/adc-debugger/usb.c
        src/adc-debugger/usb.h
        src/adc-debugger/usb_descriptors.c
        src/adc-debugger/usb_descriptors.h
)

target_include_directories(adc-debugger PUBLIC
        src/adc-debugger
        src/)

target_link_libraries(adc-debugger PUBLIC
        FreeRTOS-Kernel
        FreeRTOS-Kernel-Heap4
        pico_stdlib
        pico_unique_id
        hardware_spi
        tinyusb_device
        tinyusb_board
)

pico_add_extra_outputs(adc-debugger)

pico_enable_stdio_usb(adc-debugger 0)
pico_enable_stdio_uart(adc-debugger 1)
