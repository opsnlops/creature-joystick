
#pragma once

#include "hardware/i2c.h"

#include "pico-ssd1306/ssd1306.h"

#define DISPLAY_I2C_BAUD_RATE 1000000
#define DISPLAY_I2C_CONTROLLER i2c0
#define DISPLAY_I2C_DEVICE_ADDRESS 0x3D

// Use the namespace for convenience
using namespace pico_ssd1306;

/**
 * This project is in C, because tinyUSB is in C. (And I just wanted to
 * write C.)
 *
 * This is mostly a wrapper to get into the C++ code from C.
 */
class Display {

public:
    Display();

    void init();
    void start();

    static SSD1306* oled;

    void createOLEDDisplay();

};

portTASK_FUNCTION_PROTO(displayUpdateTask, pvParameters);
