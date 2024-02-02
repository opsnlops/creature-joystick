
#pragma once

#include <stdint.h>


#define ADC_I2C_BAUD_RATE 500000
#define ADC_I2C_CONTROLLER i2c0
#define ADC_I2C_DEVICE_ADDRESS 0x40


void ads_init();

void ads_read();
