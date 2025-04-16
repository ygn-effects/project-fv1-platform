#ifndef LOGGING_H
#define LOGGING_H

#include <Arduino.h>

#define ENABLE_LOGGING 1 // Global Debug Kill Switch (0 = no logs)

// Logging levels
#define LOG_LEVEL_NONE  0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_INFO  2
#define LOG_LEVEL_DEBUG 3

#define CURRENT_LOG_LEVEL LOG_LEVEL_DEBUG // Adjust as needed

#if ENABLE_LOGGING && (CURRENT_LOG_LEVEL >= LOG_LEVEL_DEBUG)
  #define LOG_DEBUG(format, ...) logMessage("DEBUG", __FILE__, __LINE__, format, ##__VA_ARGS__)
#else
  #define LOG_DEBUG(format, ...)
#endif

#if ENABLE_LOGGING && (CURRENT_LOG_LEVEL >= LOG_LEVEL_INFO)
  #define LOG_INFO(format, ...) logMessage("INFO", __FILE__, __LINE__, format, ##__VA_ARGS__)
#else
  #define LOG_INFO(format, ...)
#endif

#if ENABLE_LOGGING && (CURRENT_LOG_LEVEL >= LOG_LEVEL_ERROR)
  #define LOG_ERROR(format, ...) logMessage("ERROR", __FILE__, __LINE__, format, ##__VA_ARGS__)
#else
  #define LOG_ERROR(format, ...)
#endif

void logMessage(const char *level, const char *file, int line, const char *format, ...);

#endif // LOGGING_H
