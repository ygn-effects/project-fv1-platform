#pragma once

#include <stdint.h>

class Serial {
  public:
    virtual bool available() const = 0;
    virtual uint8_t read() = 0;
};
