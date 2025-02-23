cmake_minimum_required(VERSION 3.25)

set(FREERTOS_HEAP "4" CACHE STRING "" FORCE)
set(FREERTOS_PORT "TEMPLATE" CACHE STRING "" FORCE)

#set(PICO_PLATFORM rp2350)
if (NOT PICO_PLATFORM AND NOT DEFINED ENV{PICO_PLATFORM})
        message("PICO_PLATFORM not defined, defaulting to rp2040")
        set(PICO_PLATFORM rp2040)
endif()


# Pull in the Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Make sure we have the right version of the SDK
if (PICO_SDK_VERSION_STRING VERSION_LESS "2.1.0")
        message(FATAL_ERROR "Raspberry Pi Pico SDK version 2.1.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Pull in FreeRTOS from RPI directly so we get the RP2350 support
if(PICO_PLATFORM STREQUAL "rp2040")
        include($ENV{PICO_SDK_PATH}/../FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)
else()
        if (PICO_PLATFORM STREQUAL "rp2350-riscv")
                include($ENV{PICO_SDK_PATH}/../FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2350_RISC-V/FreeRTOS_Kernel_import.cmake)
        else()
                include($ENV{PICO_SDK_PATH}/../FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2350_ARM_NTZ/FreeRTOS_Kernel_import.cmake)
        endif()
endif()

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
        src/eeprom/eeprom.c
        src/eeprom/eeprom.h
        src/joystick/adc.c
        src/joystick/adc.h
        src/joystick/responsive_analog_read_filter.c
        src/joystick/responsive_analog_read_filter.h
        src/joystick/joystick.c
        src/joystick/joystick.h
        src/lights/colors.c
        src/lights/colors.h
        src/lights/status_lights.c
        src/lights/status_lights.h
        src/logging/logging.c
        src/logging/logging.h
        src/usb/usb.c
        src/usb/usb.h
        src/usb/usb_descriptors.c
        src/usb/usb_descriptors.h
        src/util/ranges.c
        src/util/ranges.h
        )

# Make sure TinyUSB can find tusb_config.h
target_include_directories(joystick PUBLIC
        src/
        generated/)

add_subdirectory(lib/ssd1306)


# PIO-base NeoPixel control 😍
pico_generate_pio_header(joystick ${CMAKE_CURRENT_LIST_DIR}/src/lights/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

# In addition to pico_stdlib required for common PicoSDK functionality, add dependency on tinyusb_device
# for TinyUSB device support and tinyusb_board for the additional board support library used by the example
target_link_libraries(joystick PUBLIC
        FreeRTOS-Kernel
        FreeRTOS-Kernel-Heap4
        pico_stdlib
        pico_unique_id
        hardware_pio
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
        src/joystick/responsive_analog_read_filter.c
        src/joystick/responsive_analog_read_filter.h
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
