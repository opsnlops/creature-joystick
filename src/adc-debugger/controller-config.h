
#pragma once

/**
* Main configuration for the controller
*/

// Just because it's funny
#define EVER ;;

#define MAX_NUMBER_OF_AXEN          8
#define MAX_NUMBER_OF_BUTTONS       8

// If this is defined, suspend the reader when there's no USB connection
//#define SUSPEND_READER_WHEN_NO_USB  0

// How many milliseconds should we treat each frame?
#define POLLING_INTERVAL            0


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
