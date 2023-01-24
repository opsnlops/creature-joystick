
// System
#include <stdlib.h>
#include <stdio.h>

// FreeRTOS
#include <FreeRTOS.h>
#include <task.h>

// TinyUSB
#include "bsp/board.h"
#include "tusb.h"

// Pico SDK
#include "pico/stdlib.h"

// Our stuff
#include "display/display_task.h"
#include "display/display_wrapper.h"
#include "joystick/joystick.h"
#include "logging/logging.h"
#include "usb/usb.h"

joystick joystick1;
pot pot1;
pot pot2;


TaskHandle_t analog_reader_task_handler;

int main(void)
{
    // All the SDK to bring up the stdio stuff, so we can write to the serial port
    stdio_init_all();

    logger_init();
    board_init();
    init_reader();

    start_usb_tasks();

    // Set up the display
    display_t *d = display_create();
    display_start_task_running(d);

    joystick1 = create_joystick(1,0);
    pot1 = create_pot(2);
    pot2 = create_pot(3);
    pot2.z.inverted = true;

    register_axis(&joystick1.x);
    register_axis(&joystick1.y);
    register_axis(&pot1.z);
    register_axis(&pot2.z);
    analog_reader_task_handler = start_analog_reader_task();

    debug("starting task scheduler!");
    vTaskStartScheduler();

}

