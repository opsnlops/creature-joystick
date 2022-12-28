

#include <stdio.h>

#include "hardware/adc.h"
#include "joystick.h"

#include "logging/logging.h"


/**
 * @brief Reads a value on an axis from the hardware
 *
 * @param a the axis to check (in/out)
 */
void read_value(axis* a) {

    adc_select_input(a->gpio_pin - 26);
    a->value = adc_read();

    verbose("read value %d from gpio %d", a->value, a->gpio_pin);
}


axis create_axis(uint8_t gpio_pin) {
    axis a;
    a.gpio_pin = gpio_pin;
    a.value = 0;

    adc_gpio_init(gpio_pin);

    debug("created a new axis on gpio %d", gpio_pin);

    return a;
}


joystick create_joystick(uint8_t x_gpio_pin, uint8_t y_gpio_pin) {

    joystick j;
    axis x, y;

    x = create_axis(x_gpio_pin);
    y = create_axis(y_gpio_pin);

    j.x = x;
    j.y = y;

    debug("created a new joystick");
    return j;

}


TaskHandle_t start_joystick(joystick* j)
{
    TaskHandle_t reader_handle;

    xTaskCreate(joystick_reader_task,
                "joystick_reader_task",
                1024,
                (void*)j,
                1,
                &reader_handle);

    return reader_handle;
}

portTASK_FUNCTION(joystick_reader_task, pvParameters) {

     joystick* j = (joystick*)pvParameters;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

    for(EVER) {

        read_value(&j->x);
        read_value(&j->y);

        info("Reading: x: %d, y: %d", j->x, j->y);

        vTaskDelay(pdMS_TO_TICKS(500));

    }

#pragma clang diagnostic pop

}
