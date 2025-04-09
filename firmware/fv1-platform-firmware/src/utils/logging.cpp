#include "logging.h"
#include <stdarg.h>  // Required for handling variable arguments (va_list)

void logMessage(const char *level, const char *format, ...) {
  // Start UART transmission with the log level
  Serial.print("[");
  Serial.print(level);
  Serial.print("] ");

  // Handle the variable argument list (for formatted strings)
  va_list args;
  va_start(args, format);
  char buffer[128];  // Buffer for the formatted string
  vsnprintf(buffer, sizeof(buffer), format, args);  // Format the string
  va_end(args);

  // Print the formatted string over UART
  Serial.println(buffer);
}
