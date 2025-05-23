
#pragma once

#include <stdint.h>

/**
* Main configuration for the controller
*/

// Just because it's funny
#define EVER ;;

// Could be 16 if there's two MCP3208s
#define MAX_NUMBER_OF_AXEN          8


// If this is defined, suspend the reader when there's no USB connection
#define SUSPEND_READER_WHEN_NO_USB  0

// How many milliseconds should we treat each frame?
#define POLLING_INTERVAL            2


/**
 * Analog Read Filter
 *
 * From the code:
 *
 *    SnapMultiplier is a value from 0 to 1 that controls the amount of easing. Increase this to lessen
 *    the amount of easing (such as 0.1) and make the responsive values more responsive, but doing so may
 *    cause more noise to seep through when sleep is not enabled.
 */
#define ANALOG_READ_FILTER_SNAP_VALUE 0.2


#define DEBUG_ADC 0


/*
 * Display Stuff
 */

// Update every 33ms (roughly 30Hz)
#define DISPLAY_UPDATE_TIME_MS      33
#define DISPLAY_BUFFER_SIZE         26
#define DISPLAY_NUMBER_OF_LINES     4


/*
 * Logging Config
 */
#define LOGGING_QUEUE_LENGTH        80
#define LOGGING_MESSAGE_MAX_LENGTH  256

/*
 * Button config
 */
#define BUTTON_MUX0                 18
#define BUTTON_MUX1                 19
#define BUTTON_MUX2                 20
#define BUTTON_MUX3                 21

// Only 8 for now since there's just one mux
typedef uint8_t button_t;                   // These two need to be
#define MAX_NUMBER_OF_BUTTONS       8       // be the same size

// Helper to make masks
#define GPIO_MASK(pin) (1u << (pin))

#define BUTTON_IN                   22

/*
 * NeoPixel stuffs
 */

#define STATUS_LIGHTS_TIME_MS       30
#define STATUS_LIGHTS_PIO           pio1

#define STATUS_LIGHTS_GPIO          7
#define STATUS_LIGHTS_ARE_RGBW      false

#define AXIS_LIGHTS_GPIO            8
#define AXIS_LIGHTS_ARE_RGBW        false

#define BUTTON_LIGHTS_GPIO          9
#define BUTTON_LIGHTS_ARE_RGBW      false

// Max brightness of the lights. Max is 255.
#define STATUS_LIGHTS_BRIGHTNESS    64
#define CASE_LIGHTS_BRIGHTNESS      216

#define ERROR_LIGHT_BRIGHTNESS      64
