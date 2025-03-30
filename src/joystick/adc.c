
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

// Function to convert an integer to a binary string
const char* toBinaryString(uint8_t value) {
    static char bStr[9];
    bStr[8] = '\0'; // Null terminator
    for (int i = 7; i >= 0; i--) {
        bStr[7 - i] = (value & (1 << i)) ? '1' : '0';
    }
    return bStr;
}


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
    configASSERT(analog_channel < TOTAL_NUM_ADC_CHANNELS);

    uint8_t adc_channel = analog_channel % CHANNELS_PER_ADC;
    uint8_t acd_cs = analog_channel <= CHANNELS_PER_ADC ? ADC0_CS_PIN : ADC1_CS_PIN;

    verbose("read channel %u -> channel %u, CS %u", analog_channel, adc_channel, acd_cs);
    return adc_read(adc_channel, acd_cs);

}

uint16_t adc_read(uint8_t adc_channel, uint8_t adc_num_cs_pin) {

    // Command to read from a specific channel in single-ended mode
    // Start bit, SGL/DIFF, and D2 bit of the channel
    uint8_t cmd0 = 0b00000110 | ((adc_channel & 0b100) >> 2);
    uint8_t cmd1 = (adc_channel & 0b011) << 6; // Remaining channel bits positioned

    uint8_t txBuffer[3] = {cmd0, cmd1, 0x00}; // The last byte doesn't matter, it's just to clock out the ADC data
    uint8_t rxBuffer[3] = {0}; // To store the response

    gpio_put(adc_num_cs_pin, 0); // Activate CS to start the transaction
    spi_write_read_blocking(spi0, txBuffer, rxBuffer, 3); // Send the command and read the response
    gpio_put(adc_num_cs_pin, 1); // Deactivate CS to end the transaction

    // Now, interpret the response:
    // Skip the first 6 bits of rxBuffer[1], then take the next 10 bits as the ADC value
    uint16_t adcResult = ((rxBuffer[1] & 0x0F) << 8) | rxBuffer[2];

    // Debug print
#if DEBUG_ADC == 1
    if (adc_channel == 2)
        debug("ADC Channel: %d, Raw SPI Data: %s %s %s, ADC Result: %u",
               adc_channel,
               toBinaryString(rxBuffer[0]),
               toBinaryString(rxBuffer[1]),
               toBinaryString(rxBuffer[2]),
               adcResult);
#endif

    return adcResult;
}
