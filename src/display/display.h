
#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "hardware/i2c.h"

#include "ssd1306.h"
#include "textRenderer/TextRenderer.h"

#define DISPLAY_I2C_BAUD_RATE 1000000
#define DISPLAY_I2C_CONTROLLER i2c1
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

    void clear();
    void setOrientation(bool orientation);
    void drawTextSmall(const char *text, uint8_t anchor_x, uint8_t anchor_y);
    void drawTextMedium(const char *text, uint8_t anchor_x, uint8_t anchor_y);
    void sendBuffer();

private:
    SSD1306* oled;
};

#endif /* DISPLAY_H_ */
