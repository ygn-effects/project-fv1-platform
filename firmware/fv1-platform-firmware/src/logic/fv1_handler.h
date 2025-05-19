#pragma once

#include <stdint.h>
#include "periphs/dac.h"
#include "logic/pot_handler.h"
#include "utils/utils.h"

struct ParamConfig {
  uint16_t m_minLogical = 0;
  uint16_t m_maxLogical = 0;
  uint16_t m_minRaw = DACConstants::c_minDacValue;
  uint16_t m_maxRaw = DACConstants::c_maxDacValue;
};

struct Fv1Handler {
  ParamConfig m_potConfigs[PotConstants::c_potCount];
  ParamConfig m_tempoConfig;

  uint16_t mapTempoValue(uint16_t t_value);
  uint16_t mapPotValue(uint8_t t_potIndex, uint16_t t_value);
};
