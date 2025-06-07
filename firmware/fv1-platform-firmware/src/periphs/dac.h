#pragma once

#include <stdint.h>

namespace DACConstants {
  static constexpr uint16_t c_minDacValue = 0;
  static constexpr uint16_t c_maxDacValue = 1023;
}

class Dac {
  public:
    virtual void init() = 0;
    virtual void write(uint16_t t_value) = 0;
};
