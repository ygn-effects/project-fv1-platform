#pragma once

#include <stdint.h>

namespace EnumUtils {

template<typename E, E... ValidValues>
struct EnumValidator {
  static constexpr bool isValid(uint8_t t_rawValue) {
    return ((t_rawValue == static_cast<uint8_t>(ValidValues)) || ...);
  }

  static constexpr E sanitize(uint8_t t_rawValue, E t_fallback) {
    return isValid(t_rawValue) ? static_cast<E>(t_rawValue) : t_fallback;
  }
};

template<typename E, uint8_t Count>
constexpr E nextEnumValue(E value) {
  return static_cast<E>((static_cast<uint8_t>(value) + 1) % Count);
}

template<typename E, uint8_t Count>
constexpr E wrappedEnumAdd(E value, int8_t delta) {
    int next = static_cast<int>(static_cast<uint8_t>(value)) + static_cast<int>(delta);
    next %= Count;
    if (next < 0) next += Count;
    return static_cast<E>(next);
}

}
