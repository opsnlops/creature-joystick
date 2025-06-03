
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <FreeRTOS.h>
#include <timers.h>

#include "controller-config.h"

#include "joystick/responsive_analog_read_filter.h"

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
    axis z;
} joystick;

typedef struct {
    axis z;
} pot;

void init_reader();
void register_axis(axis* a);

void update_axis(axis *axis, uint16_t new_value);
void read_value(axis* a);
joystick create_2axis_joystick(uint8_t x_adc_channel, uint8_t y_adc_channel);
joystick create_3axis_joystick(uint8_t x_adc_channel, uint8_t y_adc_channel, uint8_t z_adc_channel);
pot create_pot(uint8_t adc_channel);

void setButton(button_t *buttonState, uint8_t button);
void clearButton(button_t *buttonState, uint8_t button);
void toggleButton(button_t *buttonState, uint8_t button);

TimerHandle_t start_analog_reader_timer();
TimerHandle_t start_button_reader_timer();

void analog_reader_task();
void button_reader_task();


#ifdef __cplusplus
}
#endif