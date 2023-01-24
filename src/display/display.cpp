
#include "controller-config.h"

#include <cstdio>

#include <FreeRTOS.h>
#include <task.h>

#include "hardware/i2c.h"
#include "pico/stdlib.h"

#include "pico-ssd1306/ssd1306.h"
#include "pico-ssd1306/textRenderer/TextRenderer.h"

#include "display.h"
#include "logging/logging.h"



Display::Display() {

    debug("setting up the display");

    // This will get assigned when init() gets called
    this->oled = nullptr;

    debug("setting up the display's i2c");

    // Display
    i2c_init(DISPLAY_I2C_CONTROLLER, DISPLAY_I2C_BAUD_RATE);

    // Set up pins 12 and 13
    gpio_set_function(14, GPIO_FUNC_I2C);
    gpio_set_function(15, GPIO_FUNC_I2C);
    gpio_pull_up(14);
    gpio_pull_up(15);

    debug("leaving Display()");

}

/**
 * @brief Initialize the display
 *
 * This isn't in the constructor because the display needs a bit of time to power up after
 * the I2C bus is set up. It's called from the task that runs the display, and has a 250ms
 * pause built in. We can't call sleep() in the main thread, FreeRTOS will panic if we do.
 *
 * Moving it to the task is a bit weird, but it's just what you've gotta do when working
 * with actual hardware.
 *
 */
void Display::init() {
    debug("creating the oled display...");
    oled = new SSD1306(DISPLAY_I2C_CONTROLLER, DISPLAY_I2C_DEVICE_ADDRESS, Size::W128xH64);
}

void Display::start()
{
    // NOOP
}

void Display::setOrientation(bool orientation) {
    debug("set display to orientation %d", orientation);
    oled->setOrientation(orientation);
}

void Display::clear() {
    oled->clear();
}

void Display::sendBuffer() {
    oled->sendBuffer();
}

// TODO: There's gotta be a better way to do this
void Display::drawTextSmall(const char *text, uint8_t anchor_x, uint8_t anchor_y) {
    pico_ssd1306::drawText(oled, font_5x8, text, anchor_x, anchor_y);
}

void Display::drawTextMedium(const char *text, uint8_t anchor_x, uint8_t anchor_y) {
    pico_ssd1306::drawText(oled, font_8x8, text, anchor_x, anchor_y);
}