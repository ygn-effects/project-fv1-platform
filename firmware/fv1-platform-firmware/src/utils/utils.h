#include <Arduino.h>

namespace Utils {
  const char* numberToString(uint8_t number);
  void pack16(uint16_t value, uint8_t& lowByte, uint8_t& highByte);
  void unpack16(const uint8_t lowByte, const uint8_t highByte, uint16_t& value);
} // namespace utils
