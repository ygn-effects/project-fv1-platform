#pragma once

#include <stdint.h>

class PwmOutput {
  public:
    virtual void init() = 0;
    virtual void setDutyCycle(uint16_t t_cycle) = 0;
};
