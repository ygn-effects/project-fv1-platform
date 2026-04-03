#pragma once

#include <stdint.h>

enum class SwitchId : uint8_t {
  kBypass = 0,
  kTap = 1,
  kMenuEncoder = 2,
  kProgramMode = 3,
  kMenuLock = 4,
};

enum class EncoderId : uint8_t {
  kMenuEncoder = 0
};

enum class PotId : uint8_t {
  kPot0 = 0,
  kPot1 = 1,
  kPot2 = 2,
  kMixPot = 3,
  kExpr = 4
};
