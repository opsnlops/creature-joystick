
#pragma once

/**
* Main configuration for the controller
*/

// Just because it's funny
#define EVER ;;

#define MAX_NUMBER_OF_AXEN          8
#define MAX_NUMBER_OF_BUTTONS       8

// If this is defined, suspend the reader when there's no USB connection
#define SUSPEND_READER_WHEN_NO_USB  0

// How many milliseconds should we treat each frame?
#define POLLING_INTERVAL            10


/**
 * Analog Read Filter
 *
 * From the code:
 *
 *    SnapMultiplier is a value from 0 to 1 that controls the amount of easing. Increase this to lessen
 *    the amount of easing (such as 0.1) and make the responsive values more responsive, but doing so may
 *    cause more noise to seep through when sleep is not enabled.
 */
#define ANALOG_READ_FILTER_SNAP_VALUE 0.3




/*
 * Display Stuff
 */

// Update every 33ms (roughly 30Hz)
#define DISPLAY_UPDATE_TIME_MS      33
#define DISPLAY_BUFFER_SIZE         26
#define DISPLAY_NUMBER_OF_LINES     7


/*
 * Logging Config
 */
#define LOGGING_LEVEL               LOG_LEVEL_DEBUG
#define LOGGING_QUEUE_LENGTH        80
#define LOGGING_MESSAGE_MAX_LENGTH  256


#define BUTTON_0_PIN                6
#define BUTTON_1_PIN                7
#define BUTTON_2_PIN                8
#define BUTTON_3_PIN                9
#define BUTTON_4_PIN                18
#define BUTTON_5_PIN                19
#define BUTTON_6_PIN                20
#define BUTTON_7_PIN                21
#define BUTTON_8_PIN                22
