#pragma once

#include <stdint.h>
#include "periphs/adjustable.h"

class MockAdjustable : public Adjustable {
  public:
    bool m_initialized = false;
    bool m_isOff = true;
    uint8_t m_value = 0;

    void init() override {
      m_initialized = true;
    }

    void off() override {
      m_isOff = true;
      m_value = 0;
    }

    void setValue(uint8_t t_value) override {
      m_isOff = false;
      m_value = t_value;
    }

    void reset() {
      m_initialized = false;
      m_isOff = true;
      m_value = 0;
    }
};
