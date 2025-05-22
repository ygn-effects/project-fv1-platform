#pragma once

#include <stdint.h>
#include "periphs/pwm_output.h"

class PwmLed : public PwmOutput {
  uint16_t m_pintState = 0;

  void init() override {

  }

  void setDutyCycle(uint16_t t_cycle) override {
    m_pintState = t_cycle;
  }
};
