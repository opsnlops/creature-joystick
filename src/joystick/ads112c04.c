
#include <FreeRTOS.h>

#include "ads112c04.h"
#include "ads112c04_api.h"

#include "hardware/i2c.h"
#include "pico/stdlib.h"

#include "logging/logging.h"


ads112c04_handler_t sensor_handler; //creates the sensor struct

// HAL functions from the library
void i2c_write (uint8_t address, uint8_t *txBuffer, uint8_t size) {
    i2c_write_blocking(ADC_I2C_CONTROLLER, address, txBuffer, size, false);
}

void i2c_read (uint8_t address, uint8_t *rxBuffer, uint8_t size) {
    i2c_read_blocking(ADC_I2C_CONTROLLER, address, rxBuffer, size, false);
}

void delay_ms(uint32_t ms) {
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

void ads_read() {
    uint16_t data = ads112c04_readData(&sensor_handler);

    debug("ADS Data: %d", data);
}

void ads_init() {
    info("setting up the ADC i2c bus");

    // Set up the pins for i2c0
    gpio_set_function(16, GPIO_FUNC_I2C);
    gpio_set_function(17, GPIO_FUNC_I2C);
    gpio_pull_up(16);
    gpio_pull_up(17);


    // ADC I2C setup
    i2c_init(ADC_I2C_CONTROLLER, ADC_I2C_BAUD_RATE);


    ads112c04_init(&sensor_handler); //inits sensor



    ads112c04_setGain(&sensor_handler, SENSOR_GAIN_8); //set the PGA gain to 8

    ads112c04_setRefVoltage(&sensor_handler, REFP_REFN); //set the voltage reference as external.

    ads112c04_setCurrentOutput(&sensor_handler, IDAC1, AIN2); //set IDAC1 for output on AIN2
    ads112c04_setCurrentOutput(&sensor_handler, IDAC2, AIN3); //set IDAC2 for output on AIN3

    ads112c04_setCurrent(&sensor_handler, CURRENT_500_uA);

    ads112c04_setTemperatureSensor(&sensor_handler, TEMPERATURE_SENSOR_ENABLED);

    //At this point we can change conversion mode to continuos conversion. For the porpose of the example will change the mode.
    ads112c04_setConversionMode(&sensor_handler, CONTINUOS_CONVERSION);

    ads112c04_startConversion(&sensor_handler);



}