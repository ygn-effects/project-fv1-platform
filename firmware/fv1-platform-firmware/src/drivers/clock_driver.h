#pragma once

#include <Arduino.h>
#include "periphs/clock.h"

namespace hal {

class ArduinoClock : public Clock {
  public:
    uint32_t now() const override {
      return millis();
    }
};

}
