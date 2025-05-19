#pragma once

#include <stdint.h>

class Bypass {
  public:
    virtual void init() = 0;
    virtual void on() = 0;
    virtual void off() = 0;
    virtual void toggle() = 0;
};
