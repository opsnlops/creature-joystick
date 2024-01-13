
#include "controller-config.h"

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
extern button button1;



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


    // Something's gone really wrong if we can't create the timer
    configASSERT (usbDeviceTimer != NULL);

    // Start timers
    xTimerStart(usbDeviceTimer, 0);

    info("USB service timer started");

}



void usbDeviceTimerCallback(TimerHandle_t xTimer) {
    tud_task();
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

        gpio_put(PICO_DEFAULT_LED_PIN, true);
        tud_cdc_n_write_str(0, buf);
        tud_cdc_n_write_flush(0);

        gpio_put(PICO_DEFAULT_LED_PIN, false);
    }
    else {
        info("skipped CDC send");
    }
}