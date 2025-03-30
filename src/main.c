
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
#include "eeprom/eeprom.h"
#include "joystick/joystick.h"
#include "lights/status_lights.h"
#include "logging/logging.h"
#include "usb/usb.h"
#include "usb/usb_descriptors.h"

joystick joystick1;
pot pot1;

joystick joystick2;
pot pot2;

char* pico_board_id;

TaskHandle_t analog_reader_task_handler;
TaskHandle_t button_reader_task_handler;

// MARK: - USB Descriptors
uint16_t usb_pid;
uint16_t usb_vid;
uint16_t usb_version;
char usb_serial[16] = {0};
char usb_product[16] = {0};
char usb_manufacturer[32] = {0};

// What level of logging we want (this is overridden from the EEPROM if it exists)
uint8_t configured_logging_level = LOG_LEVEL_DEBUG;

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
    bi_decl(bi_program_version_string("3.0"));
    bi_decl(bi_program_feature("FreeRTOS version " tskKERNEL_VERSION_NUMBER));
    bi_decl(bi_program_url("https://creature.engineering/hardware/creature-joystick/"));
    bi_decl(bi_2pins_with_func(0, 1, GPIO_FUNC_UART));
    bi_decl(bi_4pins_with_func(2, 3, 4, 5, GPIO_FUNC_SPI));
    bi_decl(bi_2pins_with_func(14, 15, GPIO_FUNC_I2C));
    bi_decl(bi_2pins_with_func(16, 17, GPIO_FUNC_I2C));
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

    // Read the EEPROM before setting up the USB subsystem
    eeprom_setup_i2c();
    read_eeprom_and_configure();
    usb_descriptors_init();

    board_init();

    // TinyUSB board init callback after init
    if (board_init_after_tusb) {
        debug("board_init_after_tusb");
        board_init_after_tusb();
    }


    init_reader();

    // Set up the display
    volatile display_t *d = display_create();
    display_start_task_running(d);

    // Left Half
    joystick1 = create_3axis_joystick(0, 1, 2);
    joystick1.y.inverted = true;
    pot1 = create_pot(3);
    pot1.z.inverted = true;


    register_axis(&joystick1.x);
    register_axis(&joystick1.y);
    register_axis(&joystick1.z);
    register_axis(&pot1.z);


    // Right Half
    joystick2 = create_3axis_joystick(4, 5, 6);
    joystick2.z.inverted = true;
    pot2 = create_pot(7);
    pot2.z.inverted = true;

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