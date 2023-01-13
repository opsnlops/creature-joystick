

#include <limits.h>
#include <stdio.h>


#include "joystick/adc.h"
#include "joystick/joystick.h"

#include "logging/logging.h"


#define MAX_NUMBER_OF_AXEN      24

// Keep track of the number of axis we read
uint8_t number_of_axen;
axis* axis_collection[MAX_NUMBER_OF_AXEN];


void init_reader() {

    number_of_axen = 0;

    // Wipe out our axis collection
    memset(axis_collection, '\0', sizeof(axis*) * MAX_NUMBER_OF_AXEN);

    debug("created the array of axen");


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
    a->filtered_value = (uint8_t)(filter_value >> 2);

    verbose("read value %d from ADC channel %d", read_value, a->adc_channel);
}


axis create_axis(uint8_t adc_channel) {
    axis a;
    a.adc_channel = adc_channel;
    a.raw_value = 0;
    a.filtered_value = 0;
    a.adc_max = 1023;       // We're using 10 bit ADCs
    a.adc_min = 0;
    a.inverted = false;
    a.filter = create_analog_filter(true, (float)0.1);

    debug("created a new axis on ADC channel %d", adc_channel);

    return a;
}


joystick create_joystick(uint8_t x_adc_channel, uint8_t y_adc_channel) {

    joystick j;
    axis x, y;

    x = create_axis(x_adc_channel);
    y = create_axis(y_adc_channel);

    j.x = x;
    j.y = y;

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


TaskHandle_t start_analog_reader_task()
{
    TaskHandle_t reader_handle;

    xTaskCreate(analog_reader_task,
                "analog_reader_task",
                1024,
                (void*)0,
                1,
                &reader_handle);

    // Start off suspended! Will be started when the device is
    // mounted on the host
    vTaskSuspend(reader_handle);

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

        vTaskDelay(pdMS_TO_TICKS(10));

    }

#pragma clang diagnostic pop

}
