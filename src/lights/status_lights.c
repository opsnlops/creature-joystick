
#include <limits.h>

#include <FreeRTOS.h>
#include <task.h>

#include "joystick/joystick.h"
#include "lights/colors.h"
#include "lights/status_lights.h"
#include "logging/logging.h"
#include "util/ranges.h"

#include "controller-config.h"

#include "ws2812.pio.h"

// Our Data
extern bool usb_bus_active;
extern bool device_mounted;

// Axii!
extern uint8_t number_of_axen;
extern axis* axis_collection[MAX_NUMBER_OF_AXEN];

// Buttons
extern button_t button_state_mask;


uint8_t status_lights_state_machine;
uint8_t axis_lights_state_machine;
uint8_t button_lights_state_machine;
TaskHandle_t status_lights_handle;

void status_lights_init() {
    debug("init'ing the status lights");

    uint offset = pio_add_program(STATUS_LIGHTS_PIO, &ws2812_program);

    status_lights_state_machine = pio_claim_unused_sm(STATUS_LIGHTS_PIO, true);
    debug("status lights state machine: %u", status_lights_state_machine);
    ws2812_program_init(STATUS_LIGHTS_PIO, status_lights_state_machine, offset,
                        STATUS_LIGHTS_GPIO, 800000, STATUS_LIGHTS_ARE_RGBW);

    axis_lights_state_machine = pio_claim_unused_sm(STATUS_LIGHTS_PIO, true);
    debug("axis lights state machine: %u", axis_lights_state_machine);
    ws2812_program_init(STATUS_LIGHTS_PIO, axis_lights_state_machine, offset,
                        AXIS_LIGHTS_GPIO, 800000, AXIS_LIGHTS_ARE_RGBW);

    button_lights_state_machine = pio_claim_unused_sm(STATUS_LIGHTS_PIO, true);
    debug("button lights state machine: %u", button_lights_state_machine);
    ws2812_program_init(STATUS_LIGHTS_PIO, button_lights_state_machine, offset,
                        BUTTON_LIGHTS_GPIO, 800000, BUTTON_LIGHTS_ARE_RGBW);
}

void status_lights_start() {
    info("starting up the status lights!");

    xTaskCreate(status_lights_task,
                "status_lights",
                configMINIMAL_STACK_SIZE + 512,
                (void*)0,
                1,
                &status_lights_handle);

}

void put_pixel(uint32_t pixel_grb, uint8_t state_machine) {
    pio_sm_put_blocking(STATUS_LIGHTS_PIO, state_machine, pixel_grb << 8u);
}


portTASK_FUNCTION(status_lights_task, pvParameters) {

    debug("hello from the status lights task");

    // Define our colors!
    hsv_t usb_bus_active_color =    {184.0, 1.0, STATUS_LIGHTS_BRIGHTNESS};     // Like a cyan
    hsv_t usb_bus_inactive_color =  {64.0,  1.0, STATUS_LIGHTS_BRIGHTNESS};     // Yellowish
    hsv_t device_mounted_color =    {127.0, 1.0, STATUS_LIGHTS_BRIGHTNESS};     // Green
    hsv_t device_unmounted_color =  {241.0, 1.0, STATUS_LIGHTS_BRIGHTNESS};     // Blue
    hsv_t error_color =             {0.0, 1.0, STATUS_LIGHTS_BRIGHTNESS};       // Red

    hsv_t idle_color =              {0.0, 1.0, 10};

    TickType_t lastDrawTime;

    uint32_t usb_bus_light;
    uint32_t device_mounted_light;
    uint32_t controller_state_color;

    uint32_t axis_color[MAX_NUMBER_OF_AXEN] = {0};
    uint32_t button_color[MAX_NUMBER_OF_BUTTONS] = {0};




#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

    for(EVER) {

        // Make note of now
        lastDrawTime = xTaskGetTickCount();


        if (usb_bus_active) {
            usb_bus_light = hsv_to_urgb(usb_bus_active_color);
        } else {
            usb_bus_light = hsv_to_urgb(usb_bus_inactive_color);
        }

        if(device_mounted) {
            device_mounted_light = hsv_to_urgb(device_mounted_color);
        } else {
            device_mounted_light = hsv_to_urgb(device_unmounted_color);
        }

        // TODO: Add controller state
        controller_state_color = hsv_to_urgb(idle_color);


        // Look at each of the axii in use
        for(uint i = 0; i < number_of_axen; i++) {

            // Convert the position to a hue
            uint16_t hue = convertRange(axis_collection[i]->filtered_value,
                                   0,
                                   UINT8_MAX,
                                   0,              // 0 is red
                                   23300);         // 233 * 100 (233 is blue)


            uint8_t brightness = STATUS_LIGHTS_BRIGHTNESS;

            // Make this into a color we can show
            hsv_t hsv;
            hsv.h = (double)((double)hue / 100.0);
            hsv.s = 1.0;
            hsv.v = (double)((double)brightness / UINT8_MAX);
            axis_color[i] = hsv_to_urgb(hsv);

        }

        for (int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++) {
            if(button_state_mask & (1 << i)) {
                button_color[i] = hsv_to_urgb(device_mounted_color);
            } else {
                button_color[i] = 0;    // Zero means off
            }
        }

        // Color the lights
        put_pixel(usb_bus_light, status_lights_state_machine);
        put_pixel(device_mounted_light, status_lights_state_machine);
        put_pixel(controller_state_color, status_lights_state_machine);

        for(int i = 0; i < number_of_axen; i++) {
            put_pixel(axis_color[i], axis_lights_state_machine);
        }

        for(int i = 0; i < MAX_NUMBER_OF_BUTTONS; i++) {
            put_pixel(button_color[i], button_lights_state_machine);
        }


        // Wait till it's time go again
        vTaskDelayUntil(&lastDrawTime, pdMS_TO_TICKS(STATUS_LIGHTS_TIME_MS));

    }

#pragma clang diagnostic pop


}