
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
#include "joystick/joystick.h"
#include "logging/logging.h"
#include "usb/usb.h"

joystick joystick1;
pot pot1;
button button1;

joystick joystick2;
pot pot2;
button button2;

char* pico_board_id;


void get_chip_id();


/**
 * According to the docs, the USB tasks should only be started after the FreeRTOS
 * scheduler is up and running!
 *
 * @param pvParameters
 */
portTASK_FUNCTION_PROTO(startup_task, pvParameters);


volatile size_t xFreeHeapSpace;

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

    // Left Half
    joystick1 = create_3axis_joystick(1, 0, 2);
    joystick1.x.inverted = true;
    pot1 = create_pot(3);
    button1 = create_button(BUTTON_7_PIN, false);

    register_axis(&joystick1.x);
    register_axis(&joystick1.y);
    register_axis(&joystick1.z);
    register_axis(&pot1.z);
    register_button(&button1);


    // Right Half
    joystick2 = create_3axis_joystick(5, 4, 6);
    joystick2.x.inverted = true;
    pot2 = create_pot(7);
    button2 = create_button(BUTTON_0_PIN, false);

    register_axis(&joystick2.x);
    register_axis(&joystick2.y);
    register_axis(&joystick2.z);
    register_axis(&pot2.z);
    register_button(&button2);

    // Queue up the startup task for right after the scheduler starts
    TaskHandle_t startup_task_handle;
    xTaskCreate(startup_task,
                "startup_task",
                configMINIMAL_STACK_SIZE,
                NULL,
                1,
                &startup_task_handle);

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


portTASK_FUNCTION(startup_task, pvParameters) {

    // These are in a task because the docs say:

    /*
        init device stack on configured roothub port
        This should be called after scheduler/kernel is started.
        Otherwise, it could cause kernel issue since USB IRQ handler does use RTOS queue API.
     */

    usb_init();
    usb_start();


    // Bye!
    vTaskDelete(NULL);

}