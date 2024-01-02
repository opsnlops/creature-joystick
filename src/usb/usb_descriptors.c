

#include "controller-config.h"

#include "tusb.h"
#include "usb_descriptors.h"

#include "pico/stdlib.h"
#include "pico/unique_id.h"
#include "logging/logging.h"


#define USB_VID     0x0666
#define USB_PID     0x0001          // The joystick is our first product! Woo! :)
#define USB_BCD     0x0200

#define USB_MANUFACTURER_INDEX      0x01
#define USB_PRODUCT_INDEX           0x02
#define USB_SERIAL_NUMBER_INDEX     0x03


//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device =
    {
            .bLength            = sizeof(tusb_desc_device_t),
            .bDescriptorType    = TUSB_DESC_DEVICE,
            .bcdUSB             = USB_BCD,
            .bDeviceClass       = TUSB_CLASS_MISC,
            .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
            .bDeviceProtocol    = MISC_PROTOCOL_IAD,
            .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

            .idVendor           = USB_VID,
            .idProduct          = USB_PID,
            .bcdDevice          = 0x0130,       // Version number

            .iManufacturer      = USB_MANUFACTURER_INDEX,
            .iProduct           = USB_PRODUCT_INDEX,
            .iSerialNumber      = USB_SERIAL_NUMBER_INDEX,

            .bNumConfigurations = 0x01
    };

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const *tud_descriptor_device_cb(void) {

    debug("tud_descriptor_device_cb");
    return (uint8_t const *) &desc_device;
}

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+

// We've got two gamepads, so they're the same
uint8_t const desc_hid_joystick[] =
{
        TUD_HID_REPORT_DESC_ACW_JOYSTICK (HID_REPORT_ID(REPORT_ID_GAMEPAD))
};


// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *tud_hid_descriptor_report_cb(uint8_t interface) {
    debug("tud_hid_descriptor_report_cb(): interface: %d", interface);
    return desc_hid_joystick;
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

enum {
    ITF_NUM_HID = 0,
    ITF_NUM_CDC,
    ITF_NUM_CDC_DATA,
    ITF_NUM_TOTAL
};

// Total number of devices
#define  CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN + (CFG_TUD_CDC * TUD_CDC_DESC_LEN))

//#define  CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)

#define EPNUM_HID_LEFT      0x81
#define EPNUM_CDC_NOTIF     0x83
#define EPNUM_CDC_OUT       0x02
#define EPNUM_CDC_IN        0x84

uint8_t const desc_configuration[] =
    {
            // Config number, interface count, string index, total length, attribute, power in mA
            TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 200),

            // Interface number, string index, protocol, report descriptor len, EP In address, size & polling interval
            TUD_HID_DESCRIPTOR(ITF_NUM_HID, 4, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_joystick),
                               EPNUM_HID_LEFT, CFG_TUD_HID_EP_BUFSIZE, POLLING_INTERVAL),

            TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 5, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, EPNUM_CDC_IN, 64)

    };



// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
    debug("tud_descriptor_configuration_cb: %d", index);

    // This example use the same configuration for both high and full speed mode
    return desc_configuration;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

// array of pointer to string descriptors
char const *string_desc_arr[] =
{
        (const char[]) {0x09, 0x04},        // 0: is supported language is English (0x0409)
        "April's Creature Workshop",        // 1: Manufacturer
        "Joystick",                         // 2: Product
        NULL,                               // 3: Serials, should use chip ID
        "Knobs and Buttons",                // 4: Description
        "Debug Console"
};

static uint16_t _desc_str[32];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void) langid;

    uint8_t chr_count;

    // Null out the description string
    memset(_desc_str, '\0', 32);

    if (index == 0) {
        memcpy(&_desc_str[1], string_desc_arr[0], 2);
        chr_count = 1;
    }
    else if (index == USB_SERIAL_NUMBER_INDEX) {

        // Grab our unique_id
        pico_unique_board_id_t board_id;
        pico_get_unique_board_id(&board_id);
        char* pico_board_id = (char *) pvPortMalloc(sizeof(char) * (2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1));
        pico_get_unique_board_id_string(pico_board_id, 2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1);

        chr_count = strlen(pico_board_id);

        // Shove this into the array
        for (uint8_t i = 0; i < chr_count; i++) {
            _desc_str[1 + i] = pico_board_id[i];
        }

        vPortFree(pico_board_id);
    }
    else {
        // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
        // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

        if (index >= sizeof(string_desc_arr) / sizeof(string_desc_arr[0])) return NULL;

        const char *str = string_desc_arr[index];

        // Cap at max char
        chr_count = strlen(str);
        if (chr_count > 31) chr_count = 31;

        // Convert ASCII string into UTF-16
        for (uint8_t i = 0; i < chr_count; i++) {
            _desc_str[1 + i] = str[i];
        }
    }

    // first byte is length (including header), second byte is string type
    _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);

    return _desc_str;
}