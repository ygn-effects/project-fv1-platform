#pragma once

#include <stdint.h>
#include <vector>
#include <tuple>
#include "periphs/fv1.h"

class MockFv1 : public Fv1 {
  public:
    uint8_t m_s0, m_s1, m_s2;
    std::vector<std::tuple<Fv1Pot, uint16_t>> m_potValues;

    MockFv1()
      : m_s0(0), m_s1(0), m_s2(0) {}

    void sendProgramChange(uint8_t t_program) override {
      m_s0 = t_program & 0x1;
      m_s1 = (t_program >> 1) & 0x1;
      m_s2 = (t_program >> 2) & 0x1;
    }

    void sendPotValue(Fv1Pot t_pot, uint16_t t_value) override {
      m_potValues.emplace_back(t_pot, t_value);
    }
};
