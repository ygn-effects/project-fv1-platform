#pragma once

#include <stdint.h>

namespace Utils {
  constexpr void pack16(uint16_t value, uint8_t& lowByte, uint8_t& highByte) {
    lowByte = value & 0xFF;
    highByte = (value >> 8) & 0xFF;
  }

  constexpr void unpack16(const uint8_t lowByte, const uint8_t highByte, uint16_t& value) {
    value = static_cast<uint16_t>(lowByte) | (static_cast<uint16_t>(highByte) << 8);
  }

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

  inline uint16_t wrappedAdd(uint16_t current, int16_t delta, uint16_t max) {
    int next = static_cast<int>(current) + static_cast<int>(delta);
    next %= static_cast<int>(max);
    if (next < 0) next += max;
    return static_cast<uint16_t>(next);
  }

  inline uint16_t clampedAdd(uint16_t current, int16_t delta, uint16_t max) {
    int next = static_cast<int>(current) + static_cast<int>(delta);
    if (next < 0) next = 0;
    if (next > max) next = max;
    return static_cast<uint16_t>(next);
  }

  inline uint16_t clampedAdd(uint16_t current, int16_t delta, uint16_t min, uint16_t max) {
    int next = static_cast<int>(current) + static_cast<int>(delta);
    if (next < min) next = min;
    if (next > max) next = max;
    return static_cast<uint16_t>(next);
  }
} // namespace utils
