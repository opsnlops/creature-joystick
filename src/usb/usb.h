
#ifndef CREATURES_USB_H_
#define CREATURES_USB_H_

// FreeRTOS
#include <FreeRTOS.h>
#include <task.h>

// TinyUSB
#include "bsp/board.h"
#include "tusb.h"


#define HID_STACK_SIZE      configMINIMAL_STACK_SIZE
#define USBD_STACK_SIZE     512

portTASK_FUNCTION_PROTO(usb_device_task, pvParameters);
portTASK_FUNCTION_PROTO(hid_task, pvParameters);

void start_usb_tasks();
void tud_mount_cb(void);
void tud_umount_cb(void);
void tud_suspend_cb(bool remote_wakeup_en);
void tud_resume_cb(void);
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize);
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint8_t len);
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen);

static void send_hid_report(uint8_t report_id, uint32_t btn);

#endif /* CREATURES_USB_H_ */