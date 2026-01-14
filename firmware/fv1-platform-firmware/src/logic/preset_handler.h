#pragma once

#include <stdint.h>
#include "logic/logical_state.h"
#include "logic/preset.h"

struct PresetHandler {
  Preset m_snapshot;

  void snapshotFromState(const LogicalState& t_lState);
  void applyToState(LogicalState& t_lState, uint8_t t_presetIndex);
};
