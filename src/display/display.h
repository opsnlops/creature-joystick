
#pragma once

#include "hardware/i2c.h"

#include "pico-ssd1306/ssd1306.h"

#define DISPLAY_I2C_BAUD_RATE 1000000
#define DISPLAY_I2C_CONTROLLER i2c0
#define DISPLAY_I2C_DEVICE_ADDRESS 0x3D

// Use the namespace for convenience
using namespace pico_ssd1306;

/**
 * Debugging OLED Display
 *
 * Everything in this is static because it's updated via a task. I'm
 * using classes here mostly to keep the namespace clean.
 */
class Display {

public:
    Display(Controller* controller, IOHandler* io);

    void init();
    void start();

    Controller* getController();
    IOHandler* getIOHandler();
    SSD1306* getOLED();

    void createOLEDDisplay();

private:
    SSD1306* oled;
    Controller* controller;
    IOHandler* io;

};


