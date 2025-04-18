#pragma once

#include <stdint.h>

enum class AppState : uint8_t {
  kBoot,
  kRestoreState,
  kProgramIdle,
  kProgramEdit,
  kPresetIdle
};
