#pragma once

#include <stdint.h>

enum class Fv1Pot : uint8_t {
  Pot0,
  Pot1,
  Pot2
};

class Fv1 {
  public:
    virtual void sendProgramChange(uint8_t t_program) = 0;
    virtual void sendPotValue(Fv1Pot t_pot, uint16_t t_value) = 0;
};
