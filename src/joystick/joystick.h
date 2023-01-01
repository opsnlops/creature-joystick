
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>

#include <FreeRTOS.h>
#include <task.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"

// Reader task for this joystick
portTASK_FUNCTION_PROTO(joystick_reader_task, pvParameters);

typedef struct {
    uint8_t adc_channel;
    int8_t value;
} axis;

typedef struct {
    axis x;
    axis y;
} joystick;


joystick create_joystick(uint8_t x_adc_channel, uint8_t y_adc_channel);
TaskHandle_t start_joystick(joystick* j);

#ifdef __cplusplus
}
#endif