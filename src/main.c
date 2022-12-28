
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
#include "logging/logging.h"
#include "usb/usb_descriptors.h"
#include "usb/usb.h"



int main(void)
{
    // All the SDK to bring up the stdio stuff, so we can write to the serial port
    stdio_init_all();

    logger_init();
    board_init();

    start_usb_tasks();

    debug("starting task scheduler!");
    vTaskStartScheduler();

}

