#pragma once

#include <stdint.h>

class Adjustable {
  public:
    virtual void init() = 0;
    virtual void off() = 0;
    virtual void setValue(uint8_t t_value) = 0;
};
