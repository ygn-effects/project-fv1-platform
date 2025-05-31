#pragma once

#include "periphs/clock.h"

class MockedClock : public Clock {
  private:
    uint32_t m_clock;

  public:
    MockedClock()
      : m_clock(0) {}

    uint32_t now() const override {
      return m_clock;
    }

    void setClock(uint32_t t_value) {
      m_clock = t_value;
    }

    void advanceBy(uint32_t t_delta) {
      m_clock += t_delta;
    }
};
