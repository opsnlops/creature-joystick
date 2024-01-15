


#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

#include "adc.h"

#include "logging/logging.h"

#define ADC0_CS_PIN             5
#define ADC1_CS_PIN             6

#define CHANNELS_PER_ADC        8
#define NUMBER_OF_ADCS          2
#define TOTAL_NUM_ADC_CHANNELS  CHANNELS_PER_ADC * NUMBER_OF_ADCS

void joystick_adc_init() {

    debug("bringing up the ADC");

    spi_init(spi0, 1000 * 750);
    spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(2, GPIO_FUNC_SPI);
    gpio_set_function(3, GPIO_FUNC_SPI);
    gpio_set_function(4, GPIO_FUNC_SPI);


    // Chip Select (ADC0)
    gpio_init(ADC0_CS_PIN);
    gpio_set_dir(ADC0_CS_PIN, GPIO_OUT);
    gpio_put(ADC0_CS_PIN, 1);

    // Chip Select (ADC1)
    gpio_init(ADC1_CS_PIN);
    gpio_set_dir(ADC1_CS_PIN, GPIO_OUT);
    gpio_put(ADC1_CS_PIN, 1);


    vTaskDelay(pdMS_TO_TICKS(5));

    info("SPI set up for spi0");
}


uint16_t joystick_read_adc(uint8_t analog_channel) {

    // This is a big bug if this happens
    assert(analog_channel > TOTAL_NUM_ADC_CHANNELS);

    uint8_t adc_channel = analog_channel % CHANNELS_PER_ADC;
    uint8_t acd_cs = analog_channel <= CHANNELS_PER_ADC ? ADC0_CS_PIN : ADC1_CS_PIN;

    verbose("read channel %u -> channel %u, CS %u", analog_channel, adc_channel, acd_cs);
    return adc_read(adc_channel, acd_cs);

}

uint16_t adc_read(uint8_t adc_channel, uint8_t adc_num_cs_pin) {

    /*
     * The datasheet for the MCP3304 is at:
     * https://ww1.microchip.com/downloads/aemDocuments/documents/APID/ProductDocuments/DataSheets/21697F.pdf
     */

    // Bring up the CS pin
    gpio_put(adc_num_cs_pin, 0);

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

    gpio_put(adc_num_cs_pin, 1);

    uint8_t low = b2;
    uint16_t reading = hi * 256 + low;

    return reading;
}