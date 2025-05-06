#pragma once

#include <stdint.h>
#include "logic/midi_handler.h"
#include "utils/enum_utils.h"
#include "utils/utils.h"

namespace PotConstants {
  constexpr uint8_t c_potCount = 4;
}

enum class PotState : uint8_t {
  kDisabled,
  kActive
};

using PotStateValidator = EnumUtils::EnumValidator<PotState, PotState::kDisabled, PotState::kActive>;

struct PotHandler {
  PotState m_state[PotConstants::c_potCount] = {PotState::kActive};
  uint16_t m_minValue[PotConstants::c_potCount] = {0};
  uint16_t m_maxValue[PotConstants::c_potCount] = {1023};

  uint16_t mapMidiValue(uint8_t t_midiValue, uint8_t t_potIndex);
  uint16_t mapAdcValue(uint16_t t_adcValue, uint8_t t_potIndex);
};
