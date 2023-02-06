
#pragma once

/**
* Main configuration for the controller
*/

// Just because it's funny
#define EVER ;;

// If this is defined, suspend the reader when there's no USB connection
#define SUSPEND_READER_WHEN_NO_USB  1

// How many milliseconds should we treat each frame?
#define POLLING_INTERVAL            10

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
#define LOGGING_QUEUE_LENGTH        40
#define LOGGING_MESSAGE_MAX_LENGTH  256

