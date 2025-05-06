#pragma once

#include <stdint.h>
#include "logic/program.h"
#include "logic/programs.h"
#include "logic/tap_handler.h"
#include "logic/expr_handler.h"
#include "logic/pot_handler.h"
#include "utils/enum_utils.h"

enum class BypassState : uint8_t {
  kBypassed,
  kActive
};

using BypassStateValidator = EnumUtils::EnumValidator<BypassState, BypassState::kBypassed, BypassState::kActive>;

enum class ProgramMode : uint8_t {
  kProgram,
  kPreset
};

using ProgramModeValidator = EnumUtils::EnumValidator<ProgramMode, ProgramMode::kProgram, ProgramMode::kPreset>;

struct LogicalState {
  BypassState m_bypassState = BypassState::kActive;
  ProgramMode m_programMode = ProgramMode::kProgram;
  const Program* m_activeProgram = &ProgramsDefinitions::kPrograms[0];
  uint8_t m_currentProgram = 0;
  uint8_t m_currentPreset = 0;
  uint8_t m_midiChannel = 0;

  TapState m_tapState = TapState::kDisabled;
  DivState m_divState = DivState::kDisabled;
  DivValue m_divValue = DivValue::kQuarter;
  uint16_t m_interval = 0;
  uint16_t m_divInterval = 0;

  uint16_t m_tempo = 0;

  struct PotParams {
    PotState m_state = PotState::kActive;
    uint16_t m_value = 0;
    uint16_t m_minValue = 0;
    uint16_t m_maxValue = 1023;
  };

  PotParams m_potParams[PotConstants::c_potCount];

  struct ExprParams {
      ExprState m_state = ExprState::kInactive;
      MappedPot m_mappedPot = MappedPot::kPot0;
      Direction m_direction = Direction::kNormal;
      uint16_t m_heelValue = 0;
      uint16_t m_toeValue = 1023;
  };

  ExprParams m_exprParams[ProgramConstants::c_maxPrograms];
};
