
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
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include "pico/unique_id.h"

// Our stuff
#include "display/display_task.h"
#include "display/display_wrapper.h"
#include "joystick/joystick.h"
#include "lights/status_lights.h"
#include "logging/logging.h"
#include "usb/usb.h"

joystick joystick1;
pot pot1;

joystick joystick2;
pot pot2;

char* pico_board_id;

TaskHandle_t analog_reader_task_handler;
TaskHandle_t button_reader_task_handler;

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

    // Tell picotool all about us
    bi_decl(bi_program_name("joystick"));
    bi_decl(bi_program_description("April's Creature Workshop Joystick"));
    bi_decl(bi_program_version_string("1.0"));
    bi_decl(bi_program_feature("FreeRTOS version " tskKERNEL_VERSION_NUMBER));
    bi_decl(bi_program_url("https://creature.engineering/hardware/creature-joystick/"));
    bi_decl(bi_2pins_with_func(0, 1, GPIO_FUNC_UART));
    bi_decl(bi_4pins_with_func(2, 3, 4, 5, GPIO_FUNC_SPI));
    bi_decl(bi_2pins_with_func(14, 15, GPIO_FUNC_I2C));
    bi_decl(bi_1pin_with_name(STATUS_LIGHTS_GPIO, "Status Lights"));
    bi_decl(bi_1pin_with_name(AXIS_LIGHTS_GPIO, "Axis Lights"));
    bi_decl(bi_1pin_with_name(BUTTON_LIGHTS_GPIO, "Button Lights"));
    bi_decl(bi_1pin_with_name(BUTTON_MUX0, "Button Mux 0"));
    bi_decl(bi_1pin_with_name(BUTTON_MUX1, "Button Mux 1"));
    bi_decl(bi_1pin_with_name(BUTTON_MUX2, "Button Mux 2"));
    bi_decl(bi_1pin_with_name(BUTTON_MUX3, "Button Mux 3"));
    bi_decl(bi_1pin_with_name(BUTTON_IN, "Button In"));


    // All the SDK to bring up the stdio stuff, so we can write to the serial port
    stdio_init_all();

    logger_init();
    debug("Logging running!");

    // Look up our chip ID
    get_chip_id();

    board_init();
    init_reader();

    // Set up the display
    volatile display_t *d = display_create();
    display_start_task_running(d);

    // Left Half
    joystick1 = create_3axis_joystick(1, 0, 2);
    joystick1.x.inverted = true;
    pot1 = create_pot(3);

    register_axis(&joystick1.x);
    register_axis(&joystick1.y);
    register_axis(&joystick1.z);
    register_axis(&pot1.z);


    // Right Half
    joystick2 = create_3axis_joystick(5, 4, 6);
    joystick2.x.inverted = true;
    pot2 = create_pot(7);

    register_axis(&joystick2.x);
    register_axis(&joystick2.y);
    register_axis(&joystick2.z);
    register_axis(&pot2.z);

    // And go!
    analog_reader_task_handler = start_analog_reader_task();
    button_reader_task_handler = start_button_reader_task();

    // Start up the status lights
    status_lights_init();
    status_lights_start();


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