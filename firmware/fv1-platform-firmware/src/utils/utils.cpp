#include "utils/utils.h"

namespace Utils {
  const char* numberToString(uint8_t number) {
    static char buffer[4]; // Enough for "255" + null terminator
    uint8_t index = 0;

    // Convert the number to string
    if (number >= 100) {
      buffer[index++] = '0' + (number / 100);
      number %= 100;
    }
    if (number >= 10 || index > 0) {
      buffer[index++] = '0' + (number / 10);
      number %= 10;
    }
    buffer[index++] = '0' + number;
    buffer[index] = '\0';

    return buffer;
  }
}