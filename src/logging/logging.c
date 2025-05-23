
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <FreeRTOS.h>
#include <queue.h>
#include "pico/time.h"

#include "logging.h"

#include "usb/usb.h"

TaskHandle_t log_queue_reader_task_handle;
QueueHandle_t creature_log_message_queue_handle;

bool logging_queue_exists = false;

extern uint8_t configured_logging_level;

void logger_init() {
    creature_log_message_queue_handle = xQueueCreate(LOGGING_QUEUE_LENGTH, sizeof(struct LogMessage));
    vQueueAddToRegistry(creature_log_message_queue_handle, "log_message_queue");
    logging_queue_exists = true;
    start_log_reader();
}

char* log_level_to_string(uint8_t level) {
    switch (level) {
        case LOG_LEVEL_VERBOSE:
            return "Verbose";
        case LOG_LEVEL_DEBUG:
            return "Debug";
        case LOG_LEVEL_INFO:
            return "Info";
        case LOG_LEVEL_WARNING:
            return "Warning";
        case LOG_LEVEL_ERROR:
            return "Error";
        case LOG_LEVEL_FATAL:
            return "Fatal";
        default:
            return "Unknown";
    }
}

void __unused verbose(const char *message, ...) {
    if (configured_logging_level > 4) {
        // Copy the arguments to a new va_list
        va_list args;
        va_start(args, message);

        struct LogMessage lm = createMessageObject(LOG_LEVEL_VERBOSE, message, args);
        va_end(args);

        if (logging_queue_exists)
            xQueueSendToBack(creature_log_message_queue_handle, &lm, (TickType_t) 10);
    }
}

void debug(const char *message, ...) {
    if (configured_logging_level > 3) {
        // Copy the arguments to a new va_list
        va_list args;
        va_start(args, message);

        struct LogMessage lm = createMessageObject(LOG_LEVEL_DEBUG, message, args);
        va_end(args);

        if (logging_queue_exists)
            xQueueSendToBack(creature_log_message_queue_handle, &lm, (TickType_t) 10);
    }
}

void info(const char *message, ...) {
    if (configured_logging_level > 2) {
        // Copy the arguments to a new va_list
        va_list args;
        va_start(args, message);

        struct LogMessage lm = createMessageObject(LOG_LEVEL_INFO, message, args);
        va_end(args);

        if (logging_queue_exists)
            xQueueSendToBack(creature_log_message_queue_handle, &lm, (TickType_t) 10);
    }
}

void warning(const char *message, ...) {
    if (configured_logging_level > 1) {
        // Copy the arguments to a new va_list
        va_list args;
        va_start(args, message);

        struct LogMessage lm = createMessageObject(LOG_LEVEL_WARNING, message, args);
        va_end(args);

        if (logging_queue_exists)
            xQueueSendToBack(creature_log_message_queue_handle, &lm, (TickType_t) 10);
    }
}

void error(const char *message, ...) {
    if (configured_logging_level > 0) {
        // Copy the arguments to a new va_list
        va_list args;
        va_start(args, message);

        struct LogMessage lm = createMessageObject(LOG_LEVEL_ERROR, message, args);
        va_end(args);

        if (logging_queue_exists)
            xQueueSendToBack(creature_log_message_queue_handle, &lm, (TickType_t) 10);
    }
}

void __unused fatal(const char *message, ...) {
    // Copy the arguments to a new va_list
    va_list args;
    va_start(args, message);

    struct LogMessage lm = createMessageObject(LOG_LEVEL_FATAL, message, args);
    va_end(args);

    if (logging_queue_exists)
        xQueueSendToBack(creature_log_message_queue_handle, &lm, (TickType_t) 10);
}

struct LogMessage createMessageObject(uint8_t level, const char *message, va_list args) {
    char buffer[LOGGING_MESSAGE_MAX_LENGTH + 1];
    memset(buffer, '\0', LOGGING_MESSAGE_MAX_LENGTH + 1);

    vsnprintf(buffer, LOGGING_MESSAGE_MAX_LENGTH, message, args);

    struct LogMessage lm;
    lm.level = level;
    memcpy(lm.message, buffer, LOGGING_MESSAGE_MAX_LENGTH);
    return lm;
}

void start_log_reader() {
    xTaskCreate(log_queue_reader_task,
                "log_queue_reader_task",
                1512,
                NULL,
                1,
                &log_queue_reader_task_handle);
}

/**
 * @brief Creates a task that polls the logging queue
 *
 * It then spits things to the Serial port, and optionally to syslog so that a
 * Linux host can handle the heavy lifting.
 */
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

portTASK_FUNCTION(log_queue_reader_task, pvParameters) {

    struct LogMessage lm;
    char levelBuffer[4];
    memset(&levelBuffer, '\0', 4);

    // Output buffer for messages to live in
    char *output_buffer = (char *) pvPortMalloc(sizeof(char) * LOGGING_MESSAGE_MAX_LENGTH + 1);
    memset(output_buffer, '\0', LOGGING_MESSAGE_MAX_LENGTH + 1);

    for (EVER) {
        if (xQueueReceive(creature_log_message_queue_handle, &lm, (TickType_t) portMAX_DELAY) == pdPASS) {
            switch (lm.level) {
                case LOG_LEVEL_VERBOSE:
                    strncpy(levelBuffer, "[V] ", 3);
                    break;
                case LOG_LEVEL_DEBUG:
                    strncpy(levelBuffer, "[D] ", 3);
                    break;
                case LOG_LEVEL_INFO:
                    strncpy(levelBuffer, "[I] ", 3);
                    break;
                case LOG_LEVEL_WARNING:
                    strncpy(levelBuffer, "[W] ", 3);
                    break;
                case LOG_LEVEL_ERROR:
                    strncpy(levelBuffer, "[E] ", 3);
                    break;
                case LOG_LEVEL_FATAL:
                    strncpy(levelBuffer, "[F] ", 3);
                    break;
                default:
                    strncpy(levelBuffer, "[?] ", 3);
            }

            // Format our message
            uint32_t time = to_ms_since_boot(get_absolute_time());

            snprintf(output_buffer, 2048, "[%lu]%s %s\n\r", time, levelBuffer, lm.message);

            printf("%s", output_buffer);
            cdc_send(output_buffer);

            // Wipe the buffers for next time
            memset(&levelBuffer, '\0', 4);
            memset(output_buffer, '\0', LOGGING_MESSAGE_MAX_LENGTH + 1);

        }
    }
}

#pragma clang diagnostic pop
