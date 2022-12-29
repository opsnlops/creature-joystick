
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
#include "hardware/adc.h"

// Our stuff
#include "display/display_task.h"
#include "display/display_wrapper.h"
#include "joystick/joystick.h"
#include "logging/logging.h"
#include "usb/usb.h"

joystick joystick1;

int main(void)
{
    // All the SDK to bring up the stdio stuff, so we can write to the serial port
    stdio_init_all();

    logger_init();
    board_init();

    start_usb_tasks();

    // Set up the display
    display_t *d = display_create();
    display_start_task_running(d);

    adc_init();
    joystick1 = create_joystick(26,27);
    start_joystick(&joystick1);


    debug("starting task scheduler!");
    vTaskStartScheduler();

}

