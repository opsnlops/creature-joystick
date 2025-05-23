
#pragma once

// Mark this as being in C to C++ apps
#ifdef __cplusplus
extern "C"
{
#endif

#include "controller-config.h"

#include <FreeRTOS.h>
#include <task.h>

#include <string.h>
#include <stdio.h>

#define LOG_LEVEL_VERBOSE 5
#define LOG_LEVEL_DEBUG 4
#define LOG_LEVEL_INFO 3
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_FATAL 0

portTASK_FUNCTION_PROTO(log_queue_reader_task, pvParameters);

struct LogMessage {
    uint8_t level;
    char message[LOGGING_MESSAGE_MAX_LENGTH];
} __attribute__((packed));


char* log_level_to_string(uint8_t level);

void logger_init();

void __unused verbose(const char *message, ...);

void debug(const char *message, ...);

void info(const char *message, ...);

void warning(const char *message, ...);

void error(const char *message, ...);

void __unused fatal(const char *message, ...);

struct LogMessage createMessageObject(uint8_t level, const char *message, va_list args);


void start_log_reader();

#ifdef __cplusplus
}
#endif