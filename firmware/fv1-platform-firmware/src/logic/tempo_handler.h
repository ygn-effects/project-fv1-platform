#pragma once

#include <stdint.h>
#include "periphs/dac.h"
#include "utils/utils.h"

enum class TempoSource : uint8_t {
  kTap,
  kPot,
  kMenu
};

struct TempoHandler {
  uint16_t m_interval = 0;
  uint16_t m_minInterval = 0;
  uint16_t m_maxInterval = 0;
  TempoSource m_source = TempoSource::kPot;

  bool m_ledIncreasing = true;
  uint32_t m_ledLastUpdate = 0;
  uint16_t m_ledValue = 0;

  uint16_t mapInterval(int16_t t_value);

  uint8_t calculateTempoLedValue(uint32_t t_now);
};
