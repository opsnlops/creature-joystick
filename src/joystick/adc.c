


#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

#include "adc.h"

#include "logging/logging.h"

#define ADC0_CS_PIN         5

void joystick_adc_init() {

    debug("bringing up the ADC");

    spi_init(spi0, 1000 * 750);
    spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(2, GPIO_FUNC_SPI);
    gpio_set_function(3, GPIO_FUNC_SPI);
    gpio_set_function(4, GPIO_FUNC_SPI);


    // Chip Select
    gpio_init(ADC0_CS_PIN);
    gpio_set_dir(ADC0_CS_PIN, GPIO_OUT);
    gpio_put(ADC0_CS_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(5));

    info("SPI set up for spi0");
}


uint16_t joystick_read_adc(uint8_t adc_channel)
{
    uint16_t wBuff[] = {0x01, (0x08 | adc_channel) << 4, 0x00};
    uint16_t rBuff[3];

    gpio_put(ADC0_CS_PIN, 0);
    spi_write16_read16_blocking(spi0, wBuff, rBuff, 3);
    vTaskDelay(pdMS_TO_TICKS(0.05));
    gpio_put(ADC0_CS_PIN, 1);
    return ((uint16_t)rBuff[1] & 0x03) << 8 | (uint16_t)rBuff[2];
}
