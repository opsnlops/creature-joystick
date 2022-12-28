

#include "logging/logging.h"
#include "usb_descriptors.h"
#include "usb/usb.h"

static uint32_t reports_sent = 0;
static bool usb_bus_active = false;
static bool device_mounted = false;
static uint32_t events_processed = 0;

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

static void send_hid_report(uint8_t report_id, uint32_t btn)
{
    // skip if hid is not ready yet
    if ( !tud_hid_ready() ) return;

    debug("send report");

    switch(report_id)
    {

        case REPORT_ID_GAMEPAD:
        {
            // use to avoid send multiple consecutive zero report for keyboard
            static bool has_gamepad_key = false;

            hid_gamepad_report_t report =
                    {
                            .x   = 0, .y = 0, .z = 0, .rz = 0, .rx = 0, .ry = 0,
                            .hat = 0, .buttons = 0
                    };

            if ( btn )
            {
                report.hat = GAMEPAD_HAT_UP;
                report.buttons = GAMEPAD_BUTTON_A;
                tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));

                has_gamepad_key = true;
            }else
            {
                report.hat = GAMEPAD_HAT_CENTERED;
                report.buttons = 0;
                if (has_gamepad_key) tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
                has_gamepad_key = false;
            }

            break;
        }


        default:
            break;
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
    // Otherwise it could cause kernel issue since USB IRQ handler does use RTOS queue API.
    tud_init(BOARD_TUD_RHPORT);

    // RTOS forever loop
    while (1) {
        // put this thread to waiting state until there is new events
        tud_task();
    }
}
