


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

    /*
     * The datasheet for the MCP3304 is at:
     * https://ww1.microchip.com/downloads/aemDocuments/documents/APID/ProductDocuments/DataSheets/21697F.pdf
     */

    // Bring up the CS pin
    gpio_put(ADC0_CS_PIN, 0);

    uint8_t command_bits = 12;              // 00001100
    command_bits |= (adc_channel >> 1);
    spi_write_blocking(spi0, &command_bits, 1); // throw away the read value

    command_bits = 0;
    command_bits |= (adc_channel << 7);

    uint8_t b1, b2; // The two bytes we care about

    spi_write_read_blocking(spi0, &command_bits, &b1, 1);

    b1 |= 224;                  // 11100000
    uint8_t hi = b1 & 15;       // 00001111

    spi_read_blocking(spi0, 0, &b2, 1);

    vTaskDelay(pdMS_TO_TICKS(0.05));
    gpio_put(ADC0_CS_PIN, 1);

    uint8_t low = b2;
    uint16_t reading = hi * 256 + low;

    return reading;

}
