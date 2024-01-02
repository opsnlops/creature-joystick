
#include "controller-config.h"

#include <sys/cdefs.h>
#include <limits.h>

#include <FreeRTOS.h>
#include <task.h>

#include "joystick/joystick.h"
#include "logging/logging.h"
#include "usb/usb.h"
#include "usb/usb_descriptors.h"

uint32_t reports_sent = 0;
bool usb_bus_active = false;
bool device_mounted = false;
uint32_t events_processed = 0;

extern joystick joystick1;
extern pot pot1;
extern button button1;

extern joystick joystick2;
extern pot pot2;
extern button button2;

extern TaskHandle_t analog_reader_task_handler;
extern TaskHandle_t button_reader_task_handler;

TaskHandle_t usb_device_task_handle;
TaskHandle_t hid_task_handle;


enum {
    ITF_LEFT = 0,
    ITF_RIGHT = 1
};

void start_usb_tasks() {

    debug("starting up USB tasks");

    // Create a task for tinyusb device stack
    xTaskCreate(usb_device_task,
                "usbd",
                USBD_STACK_SIZE,
                NULL,
                1,
                &usb_device_task_handle);

    // Create HID task
    xTaskCreate(hid_task,
                "hid",
                HID_STACK_SIZE,
                NULL,
                1,
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


/*
 * CDC Stuff
 */

void cdc_send(char* buf) {

    if (tud_cdc_connected()) {

        tud_cdc_n_write_str(0, buf);
        tud_cdc_n_write_flush(0);
    }
    else {
        info("skipped CDC send");
    }
}


//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

static void send_hid_report()
{

#ifdef SUSPEND_READER_WHEN_NO_USB

    // Skip if none of the HIDs are ready
    if ( !tud_hid_n_ready(ITF_LEFT) && !tud_hid_n_ready(ITF_RIGHT)) {

        // Before we go, if the reader is running, stop it.
        if( eTaskGetState(analog_reader_task_handler) != eSuspended ) {
            debug("suspending reader task");
            vTaskSuspend(analog_reader_task_handler);
            vTaskSuspend(button_reader_task_handler);
        }

        return;
    }

    // Make sure the reader is running
    if(eTaskGetState(analog_reader_task_handler) == eSuspended ) {
        debug("resuming joystick reader");
        vTaskResume(analog_reader_task_handler);
        vTaskResume(button_reader_task_handler);
    }

#endif

    if ( tud_hid_n_ready(ITF_LEFT) ) {

        verbose("send_hid_report: left");

        uint32_t buttons = 0x0;
        if(button1.pressed)
            buttons = 0x1;

        tud_hid_n_gamepad_report(
                ITF_LEFT,
                0x01,
                joystick1.x.filtered_value + SCHAR_MIN,
                joystick1.y.filtered_value + SCHAR_MIN,
                joystick1.z.filtered_value + SCHAR_MIN,
                0,
                pot1.z.filtered_value + SCHAR_MIN,
                0,
                0,
                buttons
                );
    }

    if ( tud_hid_n_ready(ITF_RIGHT) ) {

        verbose("send_hid_report: right");

        uint32_t buttons = 0x0;
        if(button2.pressed)
            buttons = 0x1;

        tud_hid_n_gamepad_report(
                ITF_RIGHT,
                0x01,
                joystick2.x.filtered_value + SCHAR_MIN,
                joystick2.y.filtered_value + SCHAR_MIN,
                joystick2.z.filtered_value + SCHAR_MIN,
                0,
                pot2.z.filtered_value + SCHAR_MIN,
                0,
                0,
                buttons
        );
    }

    reports_sent++;
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
{
    verbose("tud_hid_report_complete_cb: instance: %u, report: %u, len: %u", instance, report[0], len);

    /*
     * This could be used to send more reports, like if we had a keyboard. We're using a polling
     * joystick, so it doesn't really matter
     *
        uint8_t next_report_id = report[0] + 1;

        if (next_report_id < REPORT_ID_COUNT)
        {
           send_hid_report(next_report_id, board_button_read());
        }
     */
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

    verbose("report: %d", report_type);

    /*
    if (report_type == HID_REPORT_TYPE_OUTPUT)
    {

    }
     */
}


_Noreturn void hid_task(void *param) {
    (void) param;

    for (EVER) {

        // Poll every 10ms
        vTaskDelay(pdMS_TO_TICKS(POLLING_INTERVAL));

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
_Noreturn void usb_device_task(void *param) {
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
