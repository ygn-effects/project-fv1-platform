#include "logging.h"
#include <stdarg.h>

void logMessage(const char *level, const char *file, int line, const char *format, ...) {
  if (!Serial) return;

  Serial.print("[");
  Serial.print(millis());
  Serial.print("ms] [");
  Serial.print(level);
  Serial.print("] [");
  Serial.print(file);
  Serial.print(":");
  Serial.print(line);
  Serial.print("] ");

  char buffer[128];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  buffer[sizeof(buffer) - 1] = '\0';
  Serial.println(buffer);
}
