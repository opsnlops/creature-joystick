
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

#include "joystick/filter.h"

// Reader task for this joystick
portTASK_FUNCTION_PROTO(analog_reader_task, pvParameters);

typedef struct {
    uint8_t adc_channel;
    uint16_t raw_value;
    uint8_t filtered_value;
    uint16_t adc_min;
    uint16_t adc_max;
    analog_filter filter;
    bool inverted;
} axis;

typedef struct {
    axis x;
    axis y;
} joystick;

typedef struct {
    axis z;
} pot;

void init_reader();
void register_axis(axis* a);

void update_axis(axis *axis, uint16_t new_value);
void read_value(axis* a);
joystick create_joystick(uint8_t x_adc_channel, uint8_t y_adc_channel);
pot create_pot(uint8_t adc_channel);

TaskHandle_t start_analog_reader_task();

#ifdef __cplusplus
}
#endif