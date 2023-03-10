

#include <stdio.h>

#include "controller-config.h"

#include "joystick/adc.h"
#include "joystick/joystick.h"

#include "logging/logging.h"


// Keep track of the number of axis we read
uint8_t number_of_axen;
axis* axis_collection[MAX_NUMBER_OF_AXEN];

uint8_t number_of_buttons;
button* button_collection[MAX_NUMBER_OF_BUTTONS];

bool current_button_state[MAX_NUMBER_OF_BUTTONS];

void init_reader() {

    number_of_axen = 0;
    number_of_buttons = 0;

    // Wipe out our axis collection
    memset(axis_collection, '\0', sizeof(axis*) * MAX_NUMBER_OF_AXEN);
    debug("created the array of axen");

    memset(button_collection, '\0', sizeof(button*) * MAX_NUMBER_OF_BUTTONS);
    debug("created the array of buttons");

    for(int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++)
        current_button_state[i] = false;
    debug("set the state of the buttons to false");


}

void register_axis(axis* a) {

    if(number_of_axen >= MAX_NUMBER_OF_AXEN){
        fatal("more than %d axen registered", MAX_NUMBER_OF_AXEN);
        tight_loop_contents();
    }

    axis_collection[number_of_axen] = a;
    number_of_axen++;
    debug("registered new axis on channel %d. total number: %d", a->adc_channel, number_of_axen);
}

/**
 * @brief Reads a value on an axis from the hardware
 *
 * @param a the axis to check (in/out)
 * @param read_mode which mode should we be in?
 */
void read_value(axis* a) {

    uint16_t read_value = joystick_read_adc(a->adc_channel);

    if(a->inverted) {
        read_value = a->adc_max - read_value;
    }

    // Update the raw value
    a->raw_value = read_value;

    if(read_value > a->adc_max) {
        warning("clipping adc channel %d reading at %d (was %d)",
                a->adc_channel, a->adc_max, read_value);
        read_value = a->adc_max;
    }

    if(read_value < a->adc_min) {
        warning("clipping adc channel %d reading at %d (was %d)",
                a->adc_channel, a->adc_min, read_value);
        read_value = a->adc_min;
    }

    // Update the filter
    analog_filter_update(&a->filter, read_value);

    // Get the filter's current value
    uint16_t filter_value = analog_filter_get_value(&a->filter);

    // Convert this to an 8-bit value
    a->filtered_value = (uint8_t)(filter_value >> 4);

    verbose("read adc %d - raw: %d, filtered: %d, 8-bit: %d",
            a->adc_channel,read_value, filter_value, a->filtered_value);
}


axis create_axis(uint8_t adc_channel) {
    axis a;
    a.adc_channel = adc_channel;
    a.raw_value = 0;
    a.filtered_value = 0;
    a.adc_max = 4095;       // We're using 13 bit ADCs
    a.adc_min = 0;
    a.inverted = false;
    a.filter = create_analog_filter(true, (float)0.1);

    debug("created a new axis on ADC channel %u", adc_channel);

    return a;
}


joystick create_2axis_joystick(uint8_t x_adc_channel, uint8_t y_adc_channel) {

    joystick j;
    axis x, y;

    x = create_axis(x_adc_channel);
    y = create_axis(y_adc_channel);

    j.x = x;
    j.y = y;

    debug("created a new joystick");
    return j;

}

joystick create_3axis_joystick(uint8_t x_adc_channel, uint8_t y_adc_channel, uint8_t z_adc_channel) {

    joystick j;
    axis x, y, z;

    x = create_axis(x_adc_channel);
    y = create_axis(y_adc_channel);
    z = create_axis(z_adc_channel);

    j.x = x;
    j.y = y;
    j.z = z;

    debug("created a new joystick");
    return j;

}

pot create_pot(uint8_t adc_channel) {

    pot p;
    axis z;

    z = create_axis(adc_channel);
    p.z = z;
    debug("created a new pot");
    return p;

}

button create_button(uint8_t gpio_pin, bool inverted) {

    button b;

    b.gpio_pin = gpio_pin;
    b.inverted = inverted;
    b.pressed = false;

    // Set up this GPIO pin
    gpio_set_function(gpio_pin, GPIO_FUNC_SIO);
    gpio_set_dir(gpio_pin, false);
    gpio_pull_up(gpio_pin);

    debug("created a new button on GPIO %u", gpio_pin);
    return b;
}


void register_button(button * b) {

    if(number_of_buttons >= MAX_NUMBER_OF_BUTTONS){
        fatal("more than %u buttons registered", MAX_NUMBER_OF_BUTTONS);
        tight_loop_contents();
    }

    button_collection[number_of_buttons] = b;
    number_of_buttons++;
}

void read_button(button* b) {

    // They're active low
    if(!gpio_get(b->gpio_pin)) {
        b->pressed = true;
    }
    else {
        b->pressed = false;
    }

}


TaskHandle_t start_analog_reader_task()
{
    TaskHandle_t reader_handle;

    xTaskCreate(analog_reader_task,
                "analog_reader_task",
                1024,
                (void*)0,
                1,
                &reader_handle);

#ifdef SUSPEND_READER_WHEN_NO_USB
    // Start off suspended! Will be started when the device is
    // mounted on the host
    vTaskSuspend(reader_handle);
#endif

    return reader_handle;
}

TaskHandle_t start_button_reader_task()
{
    TaskHandle_t reader_handle;

    xTaskCreate(button_reader_task,
                "button_reader_task",
                1024,
                (void*)0,
                1,
                &reader_handle);

#ifdef SUSPEND_READER_WHEN_NO_USB
    // Start off suspended! Will be started when the device is
    // mounted on the host
    vTaskSuspend(reader_handle);
#endif

    return reader_handle;
}


portTASK_FUNCTION(analog_reader_task, pvParameters) {

    joystick_adc_init();

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

    for(EVER) {

        for(int i = 0; i < number_of_axen; i++) {

            axis* a = axis_collection[i];
            read_value(a);
            verbose("read value %d (%d) from adc_channel %d", a->filtered_value, a->raw_value,  a->adc_channel);
        }

        vTaskDelay(pdMS_TO_TICKS(POLLING_INTERVAL));

    }

#pragma clang diagnostic pop

}


portTASK_FUNCTION(button_reader_task, pvParameters) {


    info("starting the button reader task");

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

    for(EVER) {

        for(int i = 0; i < number_of_buttons; i++) {

            button* b = button_collection[i];
            read_button(b);

            if(b->pressed && current_button_state[i] == false) {

                // We switched to true
                current_button_state[i] = true;
                debug("button %u pressed (gpio %u)", i, b->gpio_pin);
            }

            else if(!b->pressed && current_button_state[i] == true) {

                // We switched to false
                current_button_state[i] = false;
                debug("button %u released (gpio %u)", i, b->gpio_pin);
            }


        }

        vTaskDelay(pdMS_TO_TICKS(POLLING_INTERVAL));

    }

#pragma clang diagnostic pop

}