
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
    uint8_t gpio_pin;
    uint16_t value;
} axis;

typedef struct {
    axis x;
    axis y;
} joystick;


TaskHandle_t start_joystick(joystick* j);

#ifdef __cplusplus
}
#endif