
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

char* pico_board_id;

TaskHandle_t analog_reader_task_handler;
TaskHandle_t button_reader_task_handler;

TaskHandle_t adc_debugger_task_handler;

portTASK_FUNCTION_PROTO(adc_debugger_task, pvParameters);
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
    joystick1 = create_2axis_joystick(1, 10);
    joystick1.x.inverted = true;
    pot1 = create_pot(3);

    register_axis(&joystick1.x);
    register_axis(&joystick1.y);
    register_axis(&pot1.z);


    // And go!
    analog_reader_task_handler = start_analog_reader_task();
    button_reader_task_handler = start_button_reader_task();


    // Queue up the startup task for right after the scheduler starts
    TaskHandle_t startup_task_handle;
    xTaskCreate(startup_task,
                "startup_task",
                configMINIMAL_STACK_SIZE,
                NULL,
                1,
                &startup_task_handle);


    xTaskCreate(adc_debugger_task,
                "adc_debugger",
                configMINIMAL_STACK_SIZE,
                NULL,
                1,
                &adc_debugger_task_handler);

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


portTASK_FUNCTION(adc_debugger_task, pvParameters) {


    info("starting the ADC debugger task");

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

    for(EVER) {


        int x_raw = joystick1.x.raw_value;
        int y_raw = joystick1.y.raw_value;

        int x_filtered = joystick1.x.filtered_value;
        int y_filtered = joystick1.y.filtered_value;


        info("raw: X: %d, Y: %d -> x: %u, y: %u", x_raw, y_raw, x_filtered, y_filtered);

        vTaskDelay(pdMS_TO_TICKS(50));

    }

#pragma clang diagnostic pop

}