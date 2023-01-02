
#include <limits.h>

#include <FreeRTOS.h>
#include <task.h>

#include "joystick/joystick.h"
#include "logging/logging.h"
#include "usb_descriptors.h"
#include "usb/usb.h"

uint32_t reports_sent = 0;
bool usb_bus_active = false;
bool device_mounted = false;
uint32_t events_processed = 0;

extern joystick joystick1;
extern TaskHandle_t joystick1_task_handler;

extern pot pot1;
extern TaskHandle_t pot1_task_handler;

StaticTask_t usb_device_task_handle;
StaticTask_t hid_task_handle;
StackType_t usb_device_stack[USBD_STACK_SIZE];
StackType_t hid_stack[HID_STACK_SIZE];

void start_usb_tasks() {

    debug("starting up USB tasks");

    // Create a task for tinyusb device stack
    xTaskCreateStatic(usb_device_task,
                "usbd",
                USBD_STACK_SIZE,
                NULL,
                configMAX_PRIORITIES - 1,
                usb_device_stack,
                &usb_device_task_handle);

    // Create HID task
    xTaskCreateStatic(hid_task,
                "hid",
                HID_STACK_SIZE,
                NULL,
                configMAX_PRIORITIES - 2,
                hid_stack,
                &hid_task_handle);

}


//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
    debug("device mounted");
    device_mounted = true;
    usb_bus_active = true;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
    debug("device unmounted");
    device_mounted = false;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
    debug("USB bus suspended");

    device_mounted = false;
    usb_bus_active = false;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
    debug("USB bus resumed");
    usb_bus_active = true;
}




//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

static void send_hid_report()
{
    // skip if hid is not ready yet
    if ( !tud_hid_ready() ) {

        // Before we go, if the reader is running, stop it.
        if( eTaskGetState(joystick1_task_handler) != eSuspended ) {
            debug("suspending reader task");
            vTaskSuspend(joystick1_task_handler);
            vTaskSuspend(pot1_task_handler);
        }

        return;
    }

    // Make sure the reader is running
    if(eTaskGetState(joystick1_task_handler) == eSuspended ) {
        debug("resuming joystick reader");
        vTaskResume(joystick1_task_handler);
        vTaskResume(pot1_task_handler);
    }

    hid_gamepad_report_t report =
    {
            .x = joystick1.x.filtered_value + SCHAR_MIN,
            .y = joystick1.y.filtered_value + SCHAR_MIN,
            .rx = 0,
            .ry = 0,
            .z = pot1.z.filtered_value + SCHAR_MIN,
            .rz = 0,
            .hat = 0,
            .buttons = 0
    };

    if (tud_hid_ready())
    {
        tud_hid_n_report(0x00, 0x01, &report, sizeof(report));
    }

    reports_sent++;
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint8_t len)
{
    (void) instance;
    (void) len;

    uint8_t next_report_id = report[0] + 1;

    if (next_report_id < REPORT_ID_COUNT)
    {
       // send_hid_report(next_report_id, board_button_read());
    }
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
    // TODO not Implemented
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;

    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
    (void) instance;

    debug("report: %d", report_type);

    if (report_type == HID_REPORT_TYPE_OUTPUT)
    {

    }
}


void hid_task(void *param) {
    (void) param;

    for (EVER) {
        // Poll every 10ms
        vTaskDelay(pdMS_TO_TICKS(10));

        // Remote wakeup
        if (tud_suspended()) {
            // Wake up host if we are in suspend mode
            // and REMOTE_WAKEUP feature is enabled by host
            tud_remote_wakeup();
        }

        send_hid_report();

        events_processed++;

    }
}


// USB Device Driver task
// This top level thread process all usb events and invoke callbacks
void usb_device_task(void *param) {
    (void) param;

    tusb_init();

    // init device stack on configured roothub port
    // This should be called after scheduler/kernel is started.
    // Otherwise, it could cause kernel issue since USB IRQ handler does use RTOS queue API.
    tud_init(BOARD_TUD_RHPORT);

    // RTOS forever loop
    for (EVER) {
        // put this thread to waiting state until there is new events
        tud_task();
    }
}
