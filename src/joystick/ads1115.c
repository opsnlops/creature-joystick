
#include <limits.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico-ads1115/lib/include/ads1115.h"


#include "joystick/ads1115.h"
#include "joystick/joystick.h"
#include "logging/logging.h"

const uint8_t ADS1115_SDA_PIN = 10;
const uint8_t ADS1115_SCL_PIN = 11;


void joystick_ads1115_init(struct ads1115_adc *adc) {

    debug("bringing up the ads1115");

    // Initialise I2C
    i2c_init(ADS1115_I2C_PORT, ADS1115_I2C_FREQ);
    gpio_set_function(ADS1115_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(ADS1115_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(ADS1115_SDA_PIN);
    gpio_pull_up(ADS1115_SCL_PIN);

    // Initialise ADC
    ads1115_init(ADS1115_I2C_PORT, ADS1115_I2C_ADDR, adc);

    ads1115_set_operating_mode(ADS1115_MODE_SINGLE_SHOT, adc);
    ads1115_set_pga(ADS1115_PGA_4_096, adc);
    ads1115_set_data_rate(ADS1115_RATE_128_SPS, adc);

    // Write the configuration for this to have an effect.
    ads1115_write_config(adc);

}


uint16_t joystick_ads1115_read_input(struct ads1115_adc *adc, axis* a) {

    if(a == NULL)
    {
        error("attempting to read a null axis");
        return 0;
    }

    switch(a->adc_channel)
    {
        case 0:
            ads1115_set_input_mux(ADS1115_MUX_SINGLE_0, adc);
            ads1115_write_config(adc);
            verbose("configured to read ads1115 pin 0");
            break;
        case 1:
            ads1115_set_input_mux(ADS1115_MUX_SINGLE_1, adc);
            ads1115_write_config(adc);
            verbose("configured to read ads1115 pin 1");
            break;
        case 2:
            ads1115_set_input_mux(ADS1115_MUX_SINGLE_2, adc);
            ads1115_write_config(adc);
            verbose("configured to read ads1115 pin 2");
            break;
        case 3:
            ads1115_set_input_mux(ADS1115_MUX_SINGLE_3, adc);
            ads1115_write_config(adc);
            verbose("configured to read ads1115 pin 3");
            break;
        default:
            warning("joystick_ads1115_read_input was not in range 0-3, assuming 0");
            ads1115_set_input_mux(ADS1115_MUX_SINGLE_0, adc);
            ads1115_write_config(adc);
            break;

    }

    uint16_t adc_value;

    ads1115_read_adc(&adc_value, adc);
    if(adc_value > ADS1115_JOYSTICK_ADC_MAX) {
        warning("clipping adc channel %d reading at %d (was %d)",
                a->adc_channel, ADS1115_JOYSTICK_ADC_MAX, adc_value);
        adc_value = ADS1115_JOYSTICK_ADC_MAX;
    }

    if(adc_value < ADS1115_JOYSTICK_ADC_MIN) {
        warning("clipping adc channel %d reading at %d (was %d)",
                a->adc_channel, ADS1115_JOYSTICK_ADC_MIN, adc_value);
        adc_value = ADS1115_JOYSTICK_ADC_MIN;
    }


    // Update the filter
    analog_filter_update(&a->filter, adc_value);

    // Get the filter's current value
    uint16_t filter_value = analog_filter_get_value(&a->filter);

    // Convert this to an 8-bit value
    //float percent = (float)(filter_value - ADS1115_JOYSTICK_ADC_MIN) / (float)(ADS1115_JOYSTICK_ADC_MAX - ADS1115_JOYSTICK_ADC_MIN);
    //a->filtered_value = (uint8_t)(UCHAR_MAX * percent);
    a->filtered_value = filter_value;

    debug("read value %d from ADC channel %d (filtered: %d)", adc_value, a->adc_channel, a->filtered_value);

    return adc_value;
}


TaskHandle_t start_ads1115_joystick(joystick* j)
{
    TaskHandle_t reader_handle;

    xTaskCreate(ads1115_reader_task,
                "ads1115_reader_task",
                1024,
                (void*)j,
                1,
                &reader_handle);

    // Start off suspended! Will be started when the device is
    // mounted on the host
    //vTaskSuspend(reader_handle);

    return reader_handle;
}


portTASK_FUNCTION(ads1115_reader_task, pvParameters) {

    struct ads1115_adc adc;
    joystick_ads1115_init(&adc);

    joystick* j = (joystick*)pvParameters;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

    for(EVER) {

        joystick_ads1115_read_input(&adc, &j->x);
        joystick_ads1115_read_input(&adc, &j->y);

        verbose("Reading joystick: x: %d (%d), y: %d (%d)",
                j->x.raw_value, j->x.filtered_value, j->y.raw_value, j->y.filtered_value);

        vTaskDelay(pdMS_TO_TICKS(10));



    }

#pragma clang diagnostic pop

}