#include <stdint.h>
#include "logic/logical_state.h"

namespace PresetConstants {
  constexpr uint8_t c_maxPreset = 16;
  constexpr uint8_t c_presetBankCount = 16;
  constexpr uint8_t c_presetPerBank = 4;
}

struct Preset {
  uint8_t m_id = 0;

  uint8_t m_programIndex = 0;
  TapState m_tapState = TapState::kDisabled;
  DivState m_divState = DivState::kDisabled;
  DivValue m_divValue = DivValue::kQuarter;
  uint16_t m_interval = 0;
  uint16_t m_divInterval = 0;
  uint16_t m_tempo = 0;

  ExprState m_exprState = ExprState::kInactive;
  MappedPot m_mappedPot = MappedPot::kPot0;
  Direction m_direction = Direction::kNormal;
  uint16_t m_heelValue = 0;
  uint16_t m_toeValue = 1023;

  struct PotParams {
    PotState m_state = PotState::kActive;
    uint16_t m_value = 0;
    uint16_t m_minValue = 0;
    uint16_t m_maxValue = 1023;
  };

  PotParams m_potParams[PotConstants::c_potCount];
};

struct PresetBank {
  uint8_t m_id = 0;

  Preset m_presets[PresetConstants::c_presetPerBank];
};
