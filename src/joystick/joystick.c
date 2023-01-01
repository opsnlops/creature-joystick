

#include <limits.h>
#include <stdio.h>


#include "joystick/adc.h"
#include "joystick/joystick.h"

#include "logging/logging.h"

// TODO: This should be tunable
#define ADC_MIN 372
#define ADC_MAX 645

/**
 * @brief Reads a value on an axis from the hardware
 *
 * @param a the axis to check (in/out)
 */
void read_value(axis* a) {

   int read_value = joystick_read_adc(a->adc_channel);

    if(read_value > ADC_MAX) {
        warning("clipping joystick reading at %d (was %d)", ADC_MAX, read_value);
        read_value = ADC_MAX;
    }

    if(read_value < ADC_MIN) {
        warning("clipping joystick reading at %d (was %d)", ADC_MIN, read_value);
        read_value = ADC_MIN;
    }

    // Convert this to an 8-bit value
    float percent = (float)(read_value - ADC_MIN) / (float)(ADC_MAX - ADC_MIN);
    a->value = (int8_t)((UCHAR_MAX * percent) + SCHAR_MIN);

    verbose("read value %d from ADC channel %d", read_value, a->adc_channel);
}


axis create_axis(uint8_t adc_channel) {
    axis a;
    a.adc_channel = adc_channel;
    a.value = 0;

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


TaskHandle_t start_joystick(joystick* j)
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
    //vTaskSuspend(reader_handle);


    return reader_handle;
}

portTASK_FUNCTION(joystick_reader_task, pvParameters) {

    joystick_adc_init();

     joystick* j = (joystick*)pvParameters;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

    for(EVER) {

        read_value(&j->x);
        read_value(&j->y);

        verbose("Reading: x: %d, y: %d", j->x.value, j->y.value);

        vTaskDelay(pdMS_TO_TICKS(10));

    }

#pragma clang diagnostic pop

}
