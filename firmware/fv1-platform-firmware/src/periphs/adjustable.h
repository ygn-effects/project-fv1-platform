#pragma once

#include <stdint.h>

class Adjustable {
  private:
    virtual void setDutyCycle(uint16_t t_cycle) = 0;

  public:
    virtual void init() = 0;
    virtual void update() = 0;
};
