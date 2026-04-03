#pragma once

#include <stdint.h>

class GpioExpander {
  public:
    virtual bool readPin(uint8_t t_pin) = 0;
    virtual void writePin(uint8_t t_pin, bool t_value) = 0;
};
