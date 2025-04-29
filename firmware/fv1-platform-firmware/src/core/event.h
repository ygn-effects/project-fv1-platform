#pragma once

#include <stdint.h>

enum class EventType : uint8_t {
  // Boot events
  kBoot,
  kRestoreState,
  kBootCompleted,

  // Physical state events
  kBypassPressed,
  kRawBypassPressed,
  kTapPressed,
  kRawTapPressed,
  kTapLongPressed,
  kRawTapLongPressed,
  kMenuEncoderPressed,
  kRawMenuEncoderPressed,
  kMenuEncoderLongPressed,
  kRawMenuEncoderLongPressed,
  kMenuEncoderMoved,
  kRawMenuEncoderMoved,
  kPot0Moved,
  kRawPot0Moved,
  kPot1Moved,
  kRawPot1Moved,
  kPot2Moved,
  kRawPot2Moved,
  kMixPotMoved,
  kRawMixPotMoved,
  kRawExprMoved,
  kExprMoved,

  // App state events
  kStateChanged,

  // Menu events
  kMenuUnlocked,
  kMenuLocked,
  kMenuProgramChanged,
  kMenuPot0Moved,
  kMenuPot1Moved,
  kMenuPot2Moved,
  kMenuMixPotMoved,
  kMenuTempoChanged,
  kMenuExprStateToggled,
  kMenuExprMappedPotMoved,
  kMenuExprDirectionToggled,
  kMenuExprHeelValueMoved,
  kMenuExprToeValueMoved,

  // Logical state events
  kProgramChanged,
  kPresetChanged,
  kTapIntervalChanged,
  kTempoChanged,
  kBypassEnabled,
  kBypassDisabled
};

struct Event {
  EventType m_type;
  uint32_t m_timestamp;

  union {
    uint16_t value;
    int8_t delta;
  } m_data;
};

