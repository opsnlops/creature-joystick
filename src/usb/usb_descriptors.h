/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef USB_DESCRIPTORS_H_
#define USB_DESCRIPTORS_H_

#include "controller-config.h"

#ifdef __cplusplus
extern "C"
{
#endif

void usb_descriptors_init(void);

#define TUD_HID_REPORT_DESC_ACW_JOYSTICK(...) \
  HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP     )                 ,\
  HID_USAGE      ( HID_USAGE_DESKTOP_GAMEPAD  )                 ,\
  HID_COLLECTION ( HID_COLLECTION_APPLICATION )                 ,\
    /* Report ID if any */\
    __VA_ARGS__ \
    /* 8 bit X, Y, Z, Rz, Rx, Ry (min -127, max 127 ) */ \
    HID_USAGE_PAGE     ( HID_USAGE_PAGE_DESKTOP                 ) ,\
    HID_USAGE          ( HID_USAGE_DESKTOP_X                    ) ,\
    HID_USAGE          ( HID_USAGE_DESKTOP_Y                    ) ,\
    HID_USAGE          ( HID_USAGE_DESKTOP_Z                    ) ,\
    HID_USAGE          ( HID_USAGE_DESKTOP_RX                   ) ,\
    HID_USAGE          ( HID_USAGE_DESKTOP_RY                   ) ,\
    HID_USAGE          ( HID_USAGE_DESKTOP_RZ                   ) ,\
    HID_USAGE          ( HID_USAGE_DESKTOP_DIAL                 ) ,\
    HID_USAGE          ( HID_USAGE_DESKTOP_WHEEL                ) ,\
    HID_LOGICAL_MIN    ( 0x81                                   ) ,\
    HID_LOGICAL_MAX    ( 0x7f                                   ) ,\
    HID_REPORT_COUNT   ( 8                                      ) ,\
    HID_REPORT_SIZE    ( 8                                      ) ,\
    HID_INPUT          ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ) ,\
    /* Eight buttons */ \
    HID_USAGE_PAGE     ( HID_USAGE_PAGE_BUTTON                  ) ,\
    HID_USAGE_MIN      ( 1                                      ) ,\
    HID_USAGE_MAX      ( 4                                      ) ,\
    HID_LOGICAL_MIN    ( 0                                      ) ,\
    HID_LOGICAL_MAX    ( 1                                      ) ,\
    HID_REPORT_COUNT   ( 4                                      ) ,\
    HID_REPORT_SIZE    ( sizeof(button_t)                       ) ,\
    HID_INPUT          ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ) ,\
    /* LEDs */                                \
    HID_USAGE_PAGE     ( HID_USAGE_PAGE_LED                     ) ,\
    HID_USAGE          ( 0x17                                   ) ,\
    HID_USAGE          ( 0x18                                   ) ,\
    HID_USAGE          ( 0x20                                   ) ,\
    HID_USAGE          ( 0x2D                                   ) ,\
    HID_USAGE          ( 0x36                                   ) ,\
    HID_USAGE          ( 0x38                                   ) ,\
    HID_LOGICAL_MIN    ( 0                                      ) ,\
    HID_LOGICAL_MAX    ( 1                                      ) ,\
    HID_REPORT_COUNT   ( 6                                      ) ,\
    HID_REPORT_SIZE    ( 1                                      ) ,\
    HID_OUTPUT         ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ) ,\
    /* Discard the last two bits */ \
    HID_REPORT_COUNT   ( 1                                       ) ,\
    HID_REPORT_SIZE    ( 2                                       ) ,\
    HID_OUTPUT         ( HID_CONSTANT                            ) ,\
    HID_COLLECTION_END


enum {
    REPORT_ID_GAMEPAD = 1,
    REPORT_ID_CDC,
    REPORT_ID_COUNT
};

#ifdef __cplusplus
}
#endif

#endif /* USB_DESCRIPTORS_H_ */