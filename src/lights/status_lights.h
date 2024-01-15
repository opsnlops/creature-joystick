
#pragma once

#include <stdint.h>

#include <FreeRTOS.h>
#include <task.h>

#include "ws2812.pio.h"

// This is 0.618033988749895
#define GOLDEN_RATIO_CONJUGATE 0.618033988749895


portTASK_FUNCTION_PROTO(status_lights_task, pvParameters);

void put_pixel(uint32_t pixel_grb, uint8_t state_machine);

void status_lights_init();
void status_lights_start();
