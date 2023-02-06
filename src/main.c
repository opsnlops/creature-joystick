
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
#include "pico/unique_id.h"
#include "pico/stdlib.h"

// Our stuff
#include "display/display_task.h"
#include "display/display_wrapper.h"
#include "joystick/joystick.h"
#include "logging/logging.h"
#include "usb/usb.h"

joystick joystick1;
pot pot1;

joystick joystick2;
pot pot2;

char* pico_board_id;

TaskHandle_t analog_reader_task_handler;

void get_chip_id();

int main(void)
{
    // All the SDK to bring up the stdio stuff, so we can write to the serial port
    stdio_init_all();

    logger_init();
    debug("Logging running!");

    // Look up our chip ID
    get_chip_id();

    board_init();
    init_reader();

    start_usb_tasks();

    // Set up the display
    volatile display_t *d = display_create();
    display_start_task_running(d);

    // Left Half
    joystick1 = create_3axis_joystick(1, 0, 2);
    joystick1.x.inverted = true;
    joystick1.y.inverted = true;
    pot1 = create_pot(3);
    pot1.z.inverted = true;

    register_axis(&joystick1.x);
    register_axis(&joystick1.y);
    register_axis(&joystick1.z);
    register_axis(&pot1.z);


    // Right Half
    joystick2 = create_3axis_joystick(5, 4, 6);
    joystick2.x.inverted = true;
    joystick2.y.inverted = true;
    pot2 = create_pot(7);
    pot2.z.inverted = true;

    register_axis(&joystick2.x);
    register_axis(&joystick2.y);
    register_axis(&joystick2.z);
    register_axis(&pot2.z);

    // And go!
    analog_reader_task_handler = start_analog_reader_task();

    debug("starting task scheduler!");
    vTaskStartScheduler();

}

void get_chip_id() {

    pico_unique_board_id_t board_id;
    pico_get_unique_board_id(&board_id);
    pico_board_id = (char*)pvPortMalloc(sizeof(char) * (2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1));
    memset(pico_board_id, '\0', 2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1);
    pico_get_unique_board_id_string(pico_board_id, 2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1);

    debug("board id: %s", pico_board_id);

}