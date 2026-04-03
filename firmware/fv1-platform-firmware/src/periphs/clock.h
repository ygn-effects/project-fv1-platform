#pragma once

#include <stdint.h>

class Clock {
  public:
    virtual uint32_t now() const = 0;
};
