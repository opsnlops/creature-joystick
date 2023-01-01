
#pragma once

#include "logging/logging.h"

#ifdef __cplusplus
extern "C"
{
#endif


    void joystick_adc_init();

uint16_t joystick_read_adc(uint8_t adc_channel);








#ifdef __cplusplus
}
#endif