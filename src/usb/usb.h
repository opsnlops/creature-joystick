#include <sys/select.h>
#include <sys/cdefs.h>

#ifndef CREATURES_USB_H_
#define CREATURES_USB_H_

// Mark this as being in C to C++ apps
#ifdef __cplusplus
extern "C"
{
#endif

// FreeRTOS
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>

// TinyUSB
#include "tusb.h"

#define BOARD_TUD_RHPORT     0

// Increase stack size when debug log is enabled
#define USBD_STACK_SIZE    (3*configMINIMAL_STACK_SIZE/2) * (CFG_TUSB_DEBUG ? 2 : 1)
#define HID_STACK_SIZE      configMINIMAL_STACK_SIZE

_Noreturn
portTASK_FUNCTION_PROTO(usb_device_task, pvParameters);

_Noreturn portTASK_FUNCTION_PROTO(hid_task, pvParameters);

void write_to_cdc(char* line);

static void send_hid_report();


void usb_init();
void usb_start();

void usbDeviceTimerCallback(TimerHandle_t xTimer);
void usb_hid_task_callback(TimerHandle_t xTimer);


void cdc_send(char* buf);


bool hid_creature_joystick_report(uint8_t instance, uint8_t report_id,
                                  int8_t x,  int8_t y, int8_t z,
                                  int8_t rz, int8_t rx, int8_t ry,
                                  uint8_t left_dial, uint8_t right_dial,
                                  uint32_t buttons);

/**
 * Our custom HID report
 */
typedef struct TU_ATTR_PACKED
{
    int8_t  x;         ///< Delta x  movement of left analog-stick
    int8_t  y;         ///< Delta y  movement of left analog-stick
    int8_t  z;         ///< Delta z  movement of right analog-joystick
    int8_t  rz;        ///< Delta Rz movement of right analog-joystick
    int8_t  rx;        ///< Delta Rx movement of analog left trigger
    int8_t  ry;        ///< Delta Ry movement of analog right trigger
    uint8_t left_dial;
    uint8_t right_dial;
    //uint32_t buttons;  ///< Buttons mask for currently pressed buttons
} creature_joystick_report_t;


#ifdef __cplusplus
}
#endif

#endif /* CREATURES_USB_H_ */