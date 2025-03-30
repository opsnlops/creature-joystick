
#include "pico/unique_id.h"

#include "logging/logging.h"

#include "controller-config.h"
#include "tusb.h"
#include "usb_descriptors.h"


// Global variables loaded from EEPROM (set these values before USB init)
extern uint16_t usb_vid;            // e.g. 0x2E8A
extern uint16_t usb_pid;            // e.g. 0x1003
extern uint16_t usb_version;        // e.g. 0x0100 for version 1.0
extern char usb_serial[16];         // e.g. "0250001"
extern char usb_product[16];        // e.g. "Joystick v3"
extern char usb_manufacturer[32];   // e.g. "April's Creature Workshop"

// USB constants
#define USB_BCD                     0x0200   // USB spec version in BCD
#define USB_MANUFACTURER_INDEX      0x01
#define USB_PRODUCT_INDEX           0x02
#define USB_SERIAL_NUMBER_INDEX     0x03

//--------------------------------------------------------------------+
// Device Descriptor (Dynamic)
//--------------------------------------------------------------------+
// We'll create a static, non-const descriptor that we can update at runtime.
static tusb_desc_device_t usb_device_descriptor = {
        .bLength            = sizeof(tusb_desc_device_t),
        .bDescriptorType    = TUSB_DESC_DEVICE,
        .bcdUSB             = USB_BCD,
        .bDeviceClass       = TUSB_CLASS_MISC,
        .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
        .bDeviceProtocol    = MISC_PROTOCOL_IAD,
        .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
        .idVendor           = 0,  // Will be filled in dynamically
        .idProduct          = 0,  // "
        .bcdDevice          = 0,  // "
        .iManufacturer      = USB_MANUFACTURER_INDEX,
        .iProduct           = USB_PRODUCT_INDEX,
        .iSerialNumber      = USB_SERIAL_NUMBER_INDEX,
        .bNumConfigurations = 0x01
};

// Call this after loading EEPROM data, before initializing USB.
void usb_descriptors_init(void) {
    usb_device_descriptor.idVendor  = usb_vid;
    usb_device_descriptor.idProduct = usb_pid;
    usb_device_descriptor.bcdDevice = usb_version;
    debug("USB Descriptor initialized: VID=0x%04X, PID=0x%04X, Version=0x%04X", usb_vid, usb_pid, usb_version);
}

//--------------------------------------------------------------------+
// Device Descriptor Callback
//--------------------------------------------------------------------+
uint8_t const *tud_descriptor_device_cb(void) {
    debug("tud_descriptor_device_cb() called");
    return (uint8_t const *)&usb_device_descriptor;
}

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+
uint8_t const desc_hid_report[] = {
        TUD_HID_REPORT_DESC_ACW_JOYSTICK(HID_REPORT_ID(REPORT_ID_GAMEPAD))
};

uint8_t const *tud_hid_descriptor_report_cb(uint8_t interface) {
    debug("tud_hid_descriptor_report_cb(): interface: %d", interface);
    return desc_hid_report;
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+
enum {
    ITF_NUM_HID = 0,
    ITF_NUM_CDC_0,
    ITF_NUM_CDC_0_DATA,
    ITF_NUM_CDC_1,
    ITF_NUM_CDC_1_DATA,
    ITF_NUM_TOTAL
};

#define CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN + (CFG_TUD_CDC * TUD_CDC_DESC_LEN))
#define EPNUM_HID             0x81
#define EPNUM_CDC_0_NOTIF     0x83
#define EPNUM_CDC_0_OUT       0x02
#define EPNUM_CDC_0_IN        0x84

#define EPNUM_CDC_1_NOTIF     0x85
#define EPNUM_CDC_1_OUT       0x03
#define EPNUM_CDC_1_IN        0x86



uint8_t const desc_configuration[] = {
        TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 200),

        TUD_HID_DESCRIPTOR(ITF_NUM_HID, 4, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report),
                           EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, POLLING_INTERVAL),

        // CDC 0: Used for main communication
        TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_0, 5, EPNUM_CDC_0_NOTIF, 8, EPNUM_CDC_0_OUT, EPNUM_CDC_0_IN, 64),

        // CDC 1: Used for debugging
        TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_1, 5, EPNUM_CDC_1_NOTIF, 8, EPNUM_CDC_1_OUT, EPNUM_CDC_1_IN, 64)
};

uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
    debug("tud_descriptor_configuration_cb: %d", index);
    return desc_configuration;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+
// For indices not associated with EEPROM, we provide fallback static strings.
char const *static_string_desc_arr[] = {
        (const char[]) {0x09, 0x04},   // 0: Supported language (English - 0x0409)
        "Default Manufacturer",        // 1: Fallback Manufacturer
        "Default Product",             // 2: Fallback Product
        "Default Serial",              // 3: Fallback Serial (should normally use chip ID)
        "Knobs and Buttons",           // 4: HID Device Description
        "Debugger",                    // 5: CDC 0 Description
        "Console",                     // 6: CDC 1 Description
        "RPIReset"                     // 7: RPIReset Description
};

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void)langid;
    static uint16_t _desc_str[32];
    uint8_t chr_count = 0;
    const char *str = NULL;

    switch(index) {
        case 0:
            // Language descriptor
            memcpy(&_desc_str[1], static_string_desc_arr[0], 2);
            chr_count = 1;
            break;
        case USB_MANUFACTURER_INDEX:
            // Use EEPROM value if available, otherwise fallback
            str = (usb_manufacturer[0] != '\0') ? usb_manufacturer : static_string_desc_arr[1];
            break;
        case USB_PRODUCT_INDEX:
            str = (usb_product[0] != '\0') ? usb_product : static_string_desc_arr[2];
            break;
        case USB_SERIAL_NUMBER_INDEX:
            str = (usb_serial[0] != '\0') ? usb_serial : static_string_desc_arr[3];
            break;
        default:
            // Use static strings for other indices
            if (index < sizeof(static_string_desc_arr) / sizeof(static_string_desc_arr[0])) {
                str = static_string_desc_arr[index];
            } else {
                return NULL;
            }
            break;
    }

    debug("tud_descriptor_string_cb: %d, %s", index, str);

    if (str) {
        chr_count = strlen(str);
        if (chr_count > 31) chr_count = 31;
        for (uint8_t i = 0; i < chr_count; i++) {
            _desc_str[1 + i] = str[i];
        }
    }
    // First element: descriptor header with length and type
    _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);
    return _desc_str;
}