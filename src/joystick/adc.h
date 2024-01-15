
#pragma once

#include "logging/logging.h"

#ifdef __cplusplus
extern "C"
{
#endif

void joystick_adc_init();
uint16_t joystick_read_adc(uint8_t adc_channel);
uint16_t adc_read(uint8_t adc_channel, uint8_t adc_num_cs_pin);


#ifdef __cplusplus
}
#endif