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

  void pack16(uint16_t value, uint8_t& lowByte, uint8_t& highByte) {
    lowByte = value & 0xFF;
    highByte = (value >> 8) & 0xFF;
  }

  void unpack16(const uint8_t lowByte, const uint8_t highByte, uint16_t& value) {
    value = static_cast<uint16_t>(lowByte) | (static_cast<uint16_t>(highByte) << 8);
  }
}