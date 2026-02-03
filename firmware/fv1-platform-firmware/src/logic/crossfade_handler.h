#pragma once

#include <cstdint>
#include "logic/program.h"
#include "periphs/dac.h"
#include "utils/utils.h"

struct CrossfadeResult {
  uint16_t m_dry;
  uint16_t m_wet;
};

struct CrossfadeHandler {
  static constexpr uint16_t c_maxValue = 1023;
  static constexpr uint16_t c_midPoint = 512;

  MixCurve m_currentCurve = MixCurve::kTransition;

  CrossfadeResult calculate(uint16_t t_mixValue);
};
