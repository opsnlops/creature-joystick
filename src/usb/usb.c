
#include "controller-config.h"

#include "pico/stdlib.h"

#include <sys/cdefs.h>
#include <limits.h>

#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>

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

extern joystick joystick2;
extern pot pot2;

extern button_t button_state_mask;

extern TaskHandle_t analog_reader_task_handler;
extern TaskHandle_t button_reader_task_handler;

enum {
    JOYSTICK = 0
};


void usb_init() {

    // init TinyUSB
    tusb_init();

    // init device stack on configured roothub port
    // This should be called after scheduler/kernel is started.
    // Otherwise, it could cause kernel issue since USB IRQ handler does use RTOS queue API.
    tud_init(BOARD_TUD_RHPORT);

    // Use the onboard LED to show when we're sending CDC data
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, false);

}

void usb_start() {

    TimerHandle_t usbDeviceTimer = xTimerCreate(
            "usbDeviceTimer",              // Timer name
            pdMS_TO_TICKS(1),            // Every millisecond
            pdTRUE,                          // Auto-reload
            (void *) 0,                        // Timer ID (not used here)
            usbDeviceTimerCallback         // Callback function
    );

    TimerHandle_t hidTaskTimer = xTimerCreate(
            "hidTaskTimer",              // Timer name
            pdMS_TO_TICKS(1),            // Every millisecond
            pdTRUE,                          // Auto-reload
            (void *) 0,                        // Timer ID (not used here)
            usb_hid_task_callback         // Callback function
    );

    // Something's gone really wrong if we can't create the timer
    configASSERT (usbDeviceTimer != NULL);

    // Start timers
    xTimerStart(usbDeviceTimer, 0);
    xTimerStart(hidTaskTimer, 0);

    info("USB service timers started");

}



void usbDeviceTimerCallback(TimerHandle_t xTimer) {
    tud_task();
}

void usb_hid_task_callback(TimerHandle_t xTimer) {

    // Remote wakeup
    if (tud_suspended()) {
        // Wake up host if we are in suspend mode
        // and REMOTE_WAKEUP feature is enabled by host
        tud_remote_wakeup();
    }

    send_hid_report();
    events_processed++;
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

        // Flash the light as we're sending
        gpio_put(PICO_DEFAULT_LED_PIN, true);
        tud_cdc_n_write_str(0, buf);
        tud_cdc_n_write_flush(0);
        gpio_put(PICO_DEFAULT_LED_PIN, false);
    }
    else {
        verbose("skipped CDC send");
    }
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

bool hid_creature_joystick_report(uint8_t instance, uint8_t report_id,
                                  int8_t x,  int8_t y, int8_t z,
                                  int8_t rz, int8_t rx, int8_t ry,
                                  uint8_t left_dial, uint8_t right_dial,
                                  button_t buttons) {
    creature_joystick_report_t report =
            {
                    .x       = x,
                    .y       = y,
                    .z       = z,
                    .rz      = rz,
                    .rx      = rx,
                    .ry      = ry,
                    .left_dial = left_dial,
                    .right_dial = right_dial,
                    .buttons = buttons
            };


    verbose("instance: %d, report: %d %d %d %d %d %d, buttons: %ul", instance, report.x, report.y, report.z, report.rz, report.rx, report.ry, buttons);

    return tud_hid_n_report(instance, report_id, &report, sizeof(report));
}


static void send_hid_report()
{

    // Skip if we're not ready yet
    if ( !tud_hid_ready() ) return;


#ifdef SUSPEND_READER_WHEN_NO_USB

    // Skip if the HID isn't ready
    if ( !tud_hid_n_ready(JOYSTICK)) {

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

    verbose("send_hid_report");

    hid_creature_joystick_report(
            JOYSTICK,
            0x01,
            joystick1.x.filtered_value + SCHAR_MIN,
            joystick1.y.filtered_value + SCHAR_MIN,
            joystick1.z.filtered_value + SCHAR_MIN,
            joystick2.x.filtered_value + SCHAR_MIN,
            joystick2.y.filtered_value + SCHAR_MIN,
            joystick2.z.filtered_value + SCHAR_MIN,
            pot1.z.filtered_value + SCHAR_MIN,
            pot2.z.filtered_value + SCHAR_MIN,
            button_state_mask
            );

    reports_sent++;
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
{
    verbose("tud_hid_report_complete_cb: instance: %u, report: %u, len: %u", instance, report[0], len);
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

    debug("get report: %d, %d, %d, %d", instance, report_id, report_type, reqlen);

    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
    info("got report: %d on instance %d, size: %d", report_type, instance, bufsize);
}
