#pragma once

#include <stdint.h>
#include "periphs/adjustable.h"
#include "periphs/toggleable.h"

class MockLed : public Toggleable {
  public:
    bool initialized = false;
    uint8_t m_pinState = 0;

    void init() override {
      initialized = true;
    }

    void on() override {
      if (!initialized) return;
      m_pinState = 1;
    }

    void off() override {
      if (!initialized) return;
      m_pinState = 0;
    }

    void toggle() override {
      if (!initialized) return;
      m_pinState = !m_pinState;
    }
};

class MackAdjustbleLed : public Adjustable {
  public:
    bool initialized = false;
    uint8_t m_pinValue = 0;

    void init() override {
      initialized = true;
    }

    void off() override {
      if (!initialized) return;
      m_pinValue = 255;
    }

    void setValue(uint8_t t_value) override {
      if (!initialized) return;
      m_pinValue = t_value;
    }
};
