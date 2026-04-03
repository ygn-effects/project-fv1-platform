#pragma once

#include <stdint.h>

enum class ExprParam : uint8_t {
  kState,
  kMappedPot,
  kDirection,
  kHeel,
  kToe
};

enum class PotParam : uint8_t {
  kState,
  kMinValue,
  kMaxValue
};

enum class SavePresetParam : uint8_t {
  kTargetBank,
  kTargetPreset
};
