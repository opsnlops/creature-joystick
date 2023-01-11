
#pragma once

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico-ads1115/lib/include/ads1115.h"

#include "logging/logging.h"
#include "joystick.h"

#define ADS1115_I2C_PORT i2c1
#define ADS1115_I2C_FREQ 400000
#define ADS1115_I2C_ADDR 0x48


#define ADS1115_JOYSTICK_ADC_MIN 0
#define ADS1115_JOYSTICK_ADC_MAX USHRT_MAX

void joystick_ads1115_init(struct ads1115_adc *adc);
uint16_t joystick_ads1115_read_input(struct ads1115_adc *adc, axis* a);

TaskHandle_t start_ads1115_joystick(joystick* j);

portTASK_FUNCTION_PROTO(ads1115_reader_task, pvParameters);