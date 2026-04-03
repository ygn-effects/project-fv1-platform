#pragma once

#include <stdint.h>

class SerialInterface {
  public:
    virtual void init() = 0;
    virtual bool available() const = 0;
    virtual uint8_t read() = 0;
};
