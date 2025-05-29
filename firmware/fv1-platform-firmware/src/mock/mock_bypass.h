#pragma once

#include <stdint.h>
#include "periphs/toggleable.h"

class MockBypass : public Toggleable {
  public :
    uint8_t m_kState = 0;
    uint8_t m_okState = 0;

    void init() override {

    }

    void on() override {
      m_kState = 1;
      m_okState = 1;
    }

    void off() override {
      m_kState = 0;
      m_okState = 0;
    }

    void toggle() override {
      m_kState = !m_kState;
      m_okState = !m_okState;
    }
};
