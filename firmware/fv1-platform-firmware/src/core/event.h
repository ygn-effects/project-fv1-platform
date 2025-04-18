#pragma once

#include <stdint.h>

enum class EventType : uint8_t {
  kBypassPressed,
  kTapPressed,
  kTapLongPressed,
  kMenuEncoderPressed,
  kMenuEncoderMoved,
  kPot0Moved,
  kPot1Moved,
  kPot2Moved,
  kMixPotMoved,
  kProgramChanged,
  kPresetChanged,
  kTempoChanged,
  kBypassEnabled,
  kBypassDisabled
};

struct Event {
  EventType m_type;
  uint32_t m_timestamp;

  union {
    uint16_t value;
    uint8_t delta;
  } m_data;
};

