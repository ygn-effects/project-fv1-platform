#include <stdint.h>

namespace Utils {
  const char* numberToString(uint8_t number);

  void pack16(uint16_t value, uint8_t& lowByte, uint8_t& highByte);
  void unpack16(const uint8_t lowByte, const uint8_t highByte, uint16_t& value);

  template<typename T>
  static inline T clamp(T val, T min_val, T max_val) {
    return (val < min_val) ? min_val : (val > max_val) ? max_val : val;
  }

  template<typename T>
  static inline T mapValue(T x, T in_min, T in_max, T out_min, T out_max) {
    return (in_max == in_min)
      ? out_min  // Error value
      : (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  }

  template<typename T>
  constexpr T mapClamped(T x, T in_min, T in_max, T out_min, T out_max) {
    x = clamp(x, in_min, in_max);
    return mapValue(x, in_min, in_max, out_min, out_max);
  }
} // namespace utils
