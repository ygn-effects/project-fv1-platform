#pragma once

#include <stdint.h>
#include "periphs/digital_output.h"

class Led : public DigitalOutput {
  uint8_t m_pinState = 0;

  void init() override {

  }

  void on() override {
    m_pinState = 1;
  }

  void off() override {
    m_pinState = 0;
  }

  void toggle() override {
    m_pinState = !m_pinState;
  }
};
