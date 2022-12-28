
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

extern "C" {

    // Grab these out of the global scope
static uint32_t reports_sent;
static bool usb_bus_active;
static bool device_mounted;
static uint32_t events_processed;
}



// Located in tasks.cpp
TaskHandle_t displayUpdateTaskHandle;

Display::Display() {

    debug("starting up the display");

    this->oled = nullptr;

    debug("setting up the display's i2c");

    // Display
    i2c_init(DISPLAY_I2C_CONTROLLER, DISPLAY_I2C_BAUD_RATE);

    // Set up pins 12 and 13
    gpio_set_function(12, GPIO_FUNC_I2C);
    gpio_set_function(13, GPIO_FUNC_I2C);
    gpio_pull_up(12);
    gpio_pull_up(13);

    debug("leaving Display()");

}

void Display::init() {
    // NOOP
}

void Display::start()
{
    debug("starting display");
    xTaskCreate(displayUpdateTask,
                "displayUpdateTask",
                1024,
                (void*)this,         // Pass in a reference to ourselves
                0,                      // Low priority
                &displayUpdateTaskHandle);
}


void Display::createOLEDDisplay() {
    oled = new SSD1306(DISPLAY_I2C_CONTROLLER, DISPLAY_I2C_DEVICE_ADDRESS, Size::W128xH64);
}

// Read from the queue and print it to the screen for now
portTASK_FUNCTION(displayUpdateTask, pvParameters) {

    auto display = (Display*)pvParameters;

    /**
     * So this is a bit weird. The display needs some time to settle after the I2C bus
     * is set up. If the main task (before the scheduler is started) is delayed, FreeRTOS
     * throws an assert and halts.
     *
     * Since it does that, let's start it here, once we're in a task. It's safe to bake in
     * a delay at this point.
     */
    vTaskDelay(pdMS_TO_TICKS(250));
    display->createOLEDDisplay();

    Display::oled->setOrientation(false);  // False means horizontally

    // Allocate one buffer_line_one for the display
    char buffer[DISPLAY_NUMBER_OF_LINES][DISPLAY_BUFFER_SIZE + 1];

    for(auto & i : buffer)
        memset(i, '\0', DISPLAY_BUFFER_SIZE + 1);


#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    for (EVER) {

        // Clear the display
        Display::oled->clear();

        // Null out the buffers
        for(auto & i : buffer)
            memset(i, '\0', DISPLAY_BUFFER_SIZE + 1);


        sprintf(buffer[0], "Reports: %-5lu", reports_sent);
        sprintf(buffer[1], " Events: %-5lu",  events_processed);
        sprintf(buffer[2], "    Mem: %d", xPortGetFreeHeapSize());
        sprintf(buffer[3], "Mounted: %s   Bus: %s",
                device_mounted ? "Yes" : "No",
                usb_bus_active ? "Yes" : "No");


        drawText(Display::oled, font_5x8, buffer[0], 0, 0);
        drawText(Display::oled, font_5x8, buffer[1], 0, 7);
        drawText(Display::oled, font_5x8, buffer[2], 0, 14);
        drawText(Display::oled, font_5x8, buffer[3], 0, 21);

        Display::oled->sendBuffer();

        vTaskDelay(pdMS_TO_TICKS(DISPLAY_UPDATE_TIME_MS));
    }
#pragma clang diagnostic pop
}
