
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
portTASK_FUNCTION_PROTO(joystick_reader_task, pvParameters);
portTASK_FUNCTION_PROTO(pot_reader_task, pvParameters);

#define READ_MODE_JOYSTICK   0
#define READ_MODE_POT        1


typedef struct {
    uint8_t adc_channel;
    uint16_t raw_value;
    uint8_t filtered_value;
    analog_filter filter;
} axis;

typedef struct {
    axis x;
    axis y;
} joystick;

typedef struct {
    axis z;
} pot;

void update_axis(axis *axis, uint16_t new_value);
joystick create_joystick(uint8_t x_adc_channel, uint8_t y_adc_channel);
pot create_pot(uint8_t adc_channel);

TaskHandle_t start_joystick(joystick* j);
TaskHandle_t start_pot(pot* p);

#ifdef __cplusplus
}
#endif