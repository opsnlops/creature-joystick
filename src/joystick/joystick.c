

#include <limits.h>
#include <stdio.h>


#include "joystick/mcp3008.h"
#include "joystick/joystick.h"

#include "logging/logging.h"

// TODO: This should be tunable
#define JOYSTICK_ADC_MIN 372
#define JOYSTICK_ADC_MAX 645

#define POT_ADC_MIN 0
#define POT_ADC_MAX 1023

/**
 * @brief Reads a value on an axis from the hardware
 *
 * @param a the axis to check (in/out)
 * @param read_mode which mode should we be in?
 */
void read_value(axis* a, uint8_t read_mode) {

    uint16_t read_value = joystick_mcp3008_adc(a->adc_channel);

    // Update the raw value
    a->raw_value = read_value;

    uint16_t min_value = 0;
    uint16_t max_value = 999;

    // TODO: Clean this up
    if(read_mode == READ_MODE_JOYSTICK) {
        min_value = JOYSTICK_ADC_MIN;
        max_value = JOYSTICK_ADC_MAX;
    } else if(read_mode == READ_MODE_POT) {
        min_value = POT_ADC_MIN;
        max_value = POT_ADC_MAX;
    }

    if(read_value > max_value) {
        warning("clipping adc channel %d reading at %d (was %d)",
                a->adc_channel, max_value, read_value);
        read_value = max_value;
    }

    if(read_value < min_value) {
        warning("clipping adc channel %d reading at %d (was %d)",
                a->adc_channel, min_value, read_value);
        read_value = min_value;
    }

    // Update the filter
    analog_filter_update(&a->filter, read_value);

    // Get the filter's current value
    uint16_t filter_value = analog_filter_get_value(&a->filter);

    // Convert this to an 8-bit value
    float percent = (float)(filter_value - min_value) / (float)(max_value - min_value);
    a->filtered_value = (uint8_t)(UCHAR_MAX * percent);

    verbose("read value %d from ADC channel %d", read_value, a->adc_channel);
}


axis create_axis(uint8_t adc_channel) {
    axis a;
    a.adc_channel = adc_channel;
    a.raw_value = 0;
    a.filtered_value = 0;
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


TaskHandle_t start_mpc3008_joystick(joystick* j)
{
    TaskHandle_t reader_handle;

    xTaskCreate(joystick_reader_task,
                "joystick_reader_task",
                1024,
                (void*)j,
                1,
                &reader_handle);

    // Start off suspended! Will be started when the device is
    // mounted on the host
    vTaskSuspend(reader_handle);

    return reader_handle;
}


TaskHandle_t start_mpc3008_pot(pot* p)
{
    TaskHandle_t reader_handle;

    xTaskCreate(pot_reader_task,
                "pot_reader_task",
                1024,
                (void*)p,
                1,
                &reader_handle);

    // Start off suspended! Will be started when the device is
    // mounted on the host
    vTaskSuspend(reader_handle);

    return reader_handle;
}

portTASK_FUNCTION(joystick_reader_task, pvParameters) {

    joystick_mcp3008_init();

     joystick* j = (joystick*)pvParameters;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

    for(EVER) {

        read_value(&j->x, READ_MODE_JOYSTICK);
        read_value(&j->y, READ_MODE_JOYSTICK);

        verbose("Reading joystick: x: %d (%d), y: %d (%d)",
                j->x.raw_value, j->x.filtered_value, j->y.raw_value, j->y.filtered_value);

        vTaskDelay(pdMS_TO_TICKS(10));

    }

#pragma clang diagnostic pop

}


portTASK_FUNCTION(pot_reader_task, pvParameters) {

    pot* p = (pot*)pvParameters;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

    for(EVER) {

        read_value(&p->z, READ_MODE_POT);

        verbose("Reading pot: %d (%d)",
                p->z.raw_value, p->z.filtered_value);

        vTaskDelay(pdMS_TO_TICKS(10));

    }

#pragma clang diagnostic pop

}