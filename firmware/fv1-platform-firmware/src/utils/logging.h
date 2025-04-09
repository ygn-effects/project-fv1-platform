#ifndef LOGGING_H
#define LOGGING_H

#include <Arduino.h>

// Define logging levels
#define LOG_LEVEL_NONE  0  // No logs
#define LOG_LEVEL_ERROR 1  // Only errors
#define LOG_LEVEL_INFO  2  // Info and errors
#define LOG_LEVEL_DEBUG 3  // Debug, info, and errors

// Set the current log level (change this based on your needs)
#define CURRENT_LOG_LEVEL LOG_LEVEL_DEBUG

// Generic log function (handles formatted strings)
void logMessage(const char *level, const char *format, ...);

// Logging Macros
#if CURRENT_LOG_LEVEL >= LOG_LEVEL_DEBUG
  #define LOG_DEBUG(format, ...) logMessage("DEBUG", format, ##__VA_ARGS__)
#else
  #define LOG_DEBUG(format, ...)
#endif

#if CURRENT_LOG_LEVEL >= LOG_LEVEL_INFO
  #define LOG_INFO(format, ...) logMessage("INFO", format, ##__VA_ARGS__)
#else
  #define LOG_INFO(format, ...)
#endif

#if CURRENT_LOG_LEVEL >= LOG_LEVEL_ERROR
  #define LOG_ERROR(format, ...) logMessage("ERROR", format, ##__VA_ARGS__)
#else
  #define LOG_ERROR(format, ...)
#endif

#endif // LOGGING_H
