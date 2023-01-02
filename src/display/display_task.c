
#include <stdbool.h>

#include <FreeRTOS.h>
#include <task.h>
#include <string.h>
#include <stdio.h>

#include "joystick/joystick.h"
#include "logging/logging.h"

#include "display_wrapper.h"
#include "display_task.h"

#include "controller-config.h"


TaskHandle_t display_update_task_handle;


// Grab these out of the global scope
extern uint32_t reports_sent;
extern bool usb_bus_active;
extern bool device_mounted;
extern uint32_t events_processed;
extern joystick joystick1;
extern TaskHandle_t joystick1_task_handler;
extern pot pot1;

void display_start_task_running(display_t *d) {

    info("starting display");

    xTaskCreate(display_update_task,
                "display_update_task",
                1024,
                (void*)d,         // Pass in a reference to our display
                0,                      // Low priority
                &display_update_task_handle);
}



// Read from the queue and print it to the screen for now
portTASK_FUNCTION(display_update_task, pvParameters) {

    display_t *d = (display_t*)pvParameters;

    /**
     * So this is a bit weird. The display needs some time to settle after the I2C bus
     * is set up. If the main task (before the scheduler is started) is delayed, FreeRTOS
     * throws an assert and halts.
     *
     * Since it does that, let's start it here, once we're in a task. It's safe to bake in
     * a delay at this point.
     */
    vTaskDelay(pdMS_TO_TICKS(250));
    display_init(d);
    display_set_orientation(d, false); // False means horizontal


    // Allocate one buffer_line_one for the display
    char buffer[DISPLAY_NUMBER_OF_LINES][DISPLAY_BUFFER_SIZE + 1];

    for(int i = 0; i < DISPLAY_NUMBER_OF_LINES; i++)
        memset(buffer[i], '\0', DISPLAY_BUFFER_SIZE + 1);

    // Used in the loop to show the task state
    char* taskState;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    for (EVER) {

    // Clear the display
    display_clear(d);

    // Null out the buffers
    for(int i = 0; i < DISPLAY_NUMBER_OF_LINES; i++)
        memset(buffer[i], '\0', DISPLAY_BUFFER_SIZE + 1);


    sprintf(buffer[0], "Reports: %-5lu", reports_sent);
    sprintf(buffer[1], " Events: %-5lu",  events_processed);
    sprintf(buffer[2], "    Mem: %d", xPortGetFreeHeapSize());
    sprintf(buffer[3], "Mounted: %s   Bus: %s",
                            device_mounted ? "Yes" : "No",
                            usb_bus_active ? "Yes" : "No");
    sprintf(buffer[4], "%4d %4d %4d", joystick1.x.filtered_value, joystick1.y.filtered_value, pot1.z.filtered_value);

    switch(eTaskGetState(joystick1_task_handler)) {

        case(eSuspended):
            taskState = "Suspended";
            break;

        // The spends most of its life in blocked while running
        case(eBlocked):
        case(eRunning):
            taskState = "Running";
            break;

        case(eDeleted):
            taskState = "Deleted";
            break;
        case(eInvalid):
            taskState = "Invalid";
            break;
        case(eReady):
            taskState = "Ready";
            break;
    }

    sprintf(buffer[5], "%25s", taskState);


    display_draw_text_small(d, buffer[0], 0, 0);
    display_draw_text_small(d, buffer[1], 0, 7);
    display_draw_text_small(d, buffer[2], 0, 14);
    display_draw_text_small(d, buffer[3], 0, 21);
    display_draw_text_medium(d, buffer[4], 0, 40);
    display_draw_text_small(d, buffer[5], 0, 56);

    display_send_buffer(d);

    vTaskDelay(pdMS_TO_TICKS(DISPLAY_UPDATE_TIME_MS));

    }

#pragma clang diagnostic pop
}
