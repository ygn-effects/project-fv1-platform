#pragma once

#include <stdint.h>

enum class AppState : uint8_t {
  kBoot,
  kRestoreState,
  kBypassed,
  kProgramIdle,
  kProgramEdit,
  kPresetIdle
};
