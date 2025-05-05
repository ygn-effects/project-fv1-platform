#pragma once

#include <stdint.h>

template<typename E, E... ValidValues>
struct EnumValidator {
  static constexpr bool isValid(uint8_t t_rawValue) {
    return ((t_rawValue == static_cast<uint8_t>(ValidValues)) || ...);
  }

  static constexpr E sanitize(uint8_t t_rawValue, E t_fallback) {
    return isValid(t_rawValue) ? static_cast<E>(t_rawValue) : t_fallback;
  }
};
