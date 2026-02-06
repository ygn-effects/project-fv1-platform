#pragma once

#include <stdint.h>
#include "logic/logical_state.h"
#include "logic/preset.h"

struct PresetHandler {
  void snapshotFromState(LogicalState& t_lState, uint8_t t_presetIndex);
  void applyToState(LogicalState& t_lState, uint8_t t_presetIndex);
};
