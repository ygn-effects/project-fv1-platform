#pragma once

#include <stdint.h>

enum class EventType : uint8_t {
  // Boot events
  kBoot,
  kRestoreState,
  kBootCompleted,

  // Raw physical events
  kRawBypassPressed,
  kRawTapPressed,
  kRawTapLongPressed,
  kRawMenuEncoderPressed,
  kRawMenuEncoderLongPressed,
  kRawMenuEncoderMoved,
  kRawPot0Moved,
  kRawPot1Moved,
  kRawPot2Moved,
  kRawMixPotMoved,
  kRawExprMoved,

  // Filtered physical events
  kBypassPressed,
  kTapPressed,
  kTapLongPressed,
  kMenuEncoderPressed,
  kMenuEncoderLongPressed,
  kMenuEncoderMoved,
  kPot0Moved,
  kPot1Moved,
  kPot2Moved,
  kMixPotMoved,
  kExprMoved,

  // App state events
  kStateChanged,

  // Menu events
  kMenuUnlocked,
  kMenuLocked,
  kMenuProgramChanged,
  kMenuPresetChanged,
  kMenuPresetBankChanged,
  kMenuSavePreset,
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

  // Program events
  kProgramChanged,
  kPresetChanged,
  kPresetBankChanged,

  // Tempo events
  kTapIntervalChanged,
  kTempoChanged,

  // Bypass events
  kBypassEnabled,
  kBypassDisabled,

  // MIDI events
  kMidiBypassPressed,
  kMidiTapPressed,
  kMidiPot0Moved,
  kMidiPot1Moved,
  kMidiPot2Moved,
  kMidiMixPotMoved,
  kMidiTempoChanged,
  kMidiProgramChanged,
  kMidiProgramModeChanged,

  // Memory events
  kSaveBypass,
  kSaveProgramMode,
  kSaveCurrentProgram,
  kSaveCurrentPreset,
  kSaveMidiChannel,
  kSaveDeviceState,
  kSaveLogicalState,
  kSaveTap,
  kSaveTempo,
  kSavePot,
  kSaveExpr,
  kSavePreset,
  kLoadPresetBank,
  kPresetBankLoaded
};

enum class EventCategory : uint8_t {
  kBootEvent,
  kRawPhysicalEvent,
  kPhysicalEvent,
  kStateEvent,
  kMenuEvent,
  kProgramEvent,
  kTempoEvent,
  kBypassEvent,
  kMidiEvent,
  kSaveEvent,
  kLoadEvent
};

enum class EventSubCategory : uint8_t {
  kBootEvent,
  kRawBypassEvent,
  kRawTapEvent,
  kRawEncoderEvent,
  kRawPotEvent,
  kRawExprEvent,
  kBypassEvent,
  kTapEvent,
  kEncoderEvent,
  kPotEvent,
  kExprEvent,
  kStateChangedEvent,
  kMenuLockEvent,
  kMenuProgramChangedEvent,
  kMenuPresetChangedEvent,
  kMenuPresetBankChangedEvent,
  kMenuPresetSaveEvent,
  kMenuPotEvent,
  kMenuTempoEvent,
  kMenuExprEvent,
  kProgramChangedEvent,
  kPresetChangedEvent,
  kPresetBankChangedEvent,
  kTapIntervalEvent,
  kTempoChangedEvent,
  kBypassEnabledEvent,
  kBypassDisabledEvent,
  kMidiBypassPressedEvent,
  kMidiTapPressedEvent,
  kMidiPotMovedEvent,
  kMidiTempoChangedEvent,
  kMidiProgramChangedEvent,
  kMidiProgramModeChangedEvent,
  kSaveEvent,
  kBankLoadEvent
};

constexpr EventCategory eventCategoryMap[] = {
  EventCategory::kBootEvent,          // Boot
  EventCategory::kBootEvent,          // kRestoreState
  EventCategory::kBootEvent,          // kBootCompleted
  EventCategory::kRawPhysicalEvent,   // kRawBypassPressed
  EventCategory::kRawPhysicalEvent,   // kRawTapPressed
  EventCategory::kRawPhysicalEvent,   // kRawTapLongPressed
  EventCategory::kRawPhysicalEvent,   // kRawMenuEncoderPressed
  EventCategory::kRawPhysicalEvent,   // kRawMenuEncoderLongPressed
  EventCategory::kRawPhysicalEvent,   // kRawMenuEncoderMoved
  EventCategory::kRawPhysicalEvent,   // kRawPot0Moved
  EventCategory::kRawPhysicalEvent,   // kRawPot1Moved
  EventCategory::kRawPhysicalEvent,   // kRawPot2Moved
  EventCategory::kRawPhysicalEvent,   // kRawMixPotMoved
  EventCategory::kRawPhysicalEvent,   // kRawExprMoved
  EventCategory::kPhysicalEvent,      // kBypassPressed
  EventCategory::kPhysicalEvent,      // kTapPressed
  EventCategory::kPhysicalEvent,      // kTapLongPressed
  EventCategory::kPhysicalEvent,      // kMenuEncoderPressed
  EventCategory::kPhysicalEvent,      // kMenuEncoderLongPressed
  EventCategory::kPhysicalEvent,      // kMenuEncoderMoved
  EventCategory::kPhysicalEvent,      // kPot0Moved
  EventCategory::kPhysicalEvent,      // kPot1Moved
  EventCategory::kPhysicalEvent,      // kPot2Moved
  EventCategory::kPhysicalEvent,      // kMixPotMoved
  EventCategory::kPhysicalEvent,      // kExprMoved
  EventCategory::kStateEvent,         // kStateChanged
  EventCategory::kMenuEvent,          // kMenuUnlocked
  EventCategory::kMenuEvent,          // kMenuLocked
  EventCategory::kMenuEvent,          // kMenuProgramChanged
  EventCategory::kMenuEvent,          // kMenuPresetChanged
  EventCategory::kMenuEvent,          // kMenuPresetBankChanged
  EventCategory::kMenuEvent,          // kMenuSavePreset
  EventCategory::kMenuEvent,          // kMenuPot0Moved
  EventCategory::kMenuEvent,          // kMenuPot1Moved
  EventCategory::kMenuEvent,          // kMenuPot2Moved
  EventCategory::kMenuEvent,          // kMenuMixPotMoved
  EventCategory::kMenuEvent,          // kMenuTempoChanged
  EventCategory::kMenuEvent,          // kMenuExprStateToggled
  EventCategory::kMenuEvent,          // kMenuExprMappedPotMoved
  EventCategory::kMenuEvent,          // kMenuExprDirectionToggled
  EventCategory::kMenuEvent,          // kMenuExprHeelValueMoved
  EventCategory::kMenuEvent,          // kMenuExprToeValueMoved
  EventCategory::kProgramEvent,       // kProgramChanged
  EventCategory::kProgramEvent,       // kPresetChanged
  EventCategory::kProgramEvent,       // kPresetBankChanged
  EventCategory::kTempoEvent,         // kTapIntervalChanged
  EventCategory::kTempoEvent,         // kTempoChanged
  EventCategory::kBypassEvent,        // kBypassEnabled
  EventCategory::kBypassEvent,        // kBypassDisabled
  EventCategory::kMidiEvent,          // kMidiBypassPressed
  EventCategory::kMidiEvent,          // kMidiTapPressed
  EventCategory::kMidiEvent,          // kMidiPot0Moved
  EventCategory::kMidiEvent,          // kMidiPot1Moved
  EventCategory::kMidiEvent,          // kMidiPot2Moved
  EventCategory::kMidiEvent,          // kMidiMixPotMoved
  EventCategory::kMidiEvent,          // kMidiTempoChanged
  EventCategory::kMidiEvent,          // kMidiProgramChanged
  EventCategory::kMidiEvent,          // kMidiProgramModeChanged
  EventCategory::kSaveEvent,          // kSaveBypass
  EventCategory::kSaveEvent,          // kSaveProgramMode
  EventCategory::kSaveEvent,          // kSaveCurrentProgram
  EventCategory::kSaveEvent,          // kSaveCurrentPreset
  EventCategory::kSaveEvent,          // kSaveMidiChannel
  EventCategory::kSaveEvent,          // kSaveDeviceState
  EventCategory::kSaveEvent,          // kSaveLogicalState
  EventCategory::kSaveEvent,          // kSaveTap
  EventCategory::kSaveEvent,          // kSaveTempo
  EventCategory::kSaveEvent,          // kSavePot
  EventCategory::kSaveEvent,          // kSaveExpr
  EventCategory::kSaveEvent,          // kSavePreset
  EventCategory::kLoadEvent,          // kLoadPresetBank
  EventCategory::kLoadEvent           // kPresetBankLoaded
};

constexpr EventSubCategory eventSubCategoryMap[] = {
  EventSubCategory::kBootEvent,                   // Boot
  EventSubCategory::kBootEvent,                   // kRestoreState
  EventSubCategory::kBootEvent,                   // kBootCompleted
  EventSubCategory::kRawBypassEvent,              // kRawBypassPressed
  EventSubCategory::kRawTapEvent,                 // kRawTapPressed
  EventSubCategory::kRawTapEvent,                 // kRawTapLongPressed
  EventSubCategory::kRawEncoderEvent,             // kRawMenuEncoderPressed
  EventSubCategory::kRawEncoderEvent,             // kRawMenuEncoderLongPressed
  EventSubCategory::kRawEncoderEvent,             // kRawMenuEncoderMoved
  EventSubCategory::kRawPotEvent,                 // kRawPot0Moved
  EventSubCategory::kRawPotEvent,                 // kRawPot1Moved
  EventSubCategory::kRawPotEvent,                 // kRawPot2Moved
  EventSubCategory::kRawPotEvent,                 // kRawMixPotMoved
  EventSubCategory::kRawExprEvent,                // kRawExprMoved
  EventSubCategory::kBypassEvent,                 // kBypassPressed
  EventSubCategory::kTapEvent,                    // kTapPressed
  EventSubCategory::kTapEvent,                    // kTapLongPressed
  EventSubCategory::kEncoderEvent,                // kMenuEncoderPressed
  EventSubCategory::kEncoderEvent,                // kMenuEncoderLongPressed
  EventSubCategory::kEncoderEvent,                // kMenuEncoderMoved
  EventSubCategory::kPotEvent,                    // kPot0Moved
  EventSubCategory::kPotEvent,                    // kPot1Moved
  EventSubCategory::kPotEvent,                    // kPot2Moved
  EventSubCategory::kPotEvent,                    // kMixPotMoved
  EventSubCategory::kExprEvent,                   // kExprMoved
  EventSubCategory::kStateChangedEvent,           // kStateChanged
  EventSubCategory::kMenuLockEvent,               // kMenuUnlocked
  EventSubCategory::kMenuLockEvent,               // kMenuLocked
  EventSubCategory::kMenuProgramChangedEvent,     // kMenuProgramChanged
  EventSubCategory::kMenuPresetChangedEvent,      // kMenuPresetChanged
  EventSubCategory::kMenuPresetBankChangedEvent,  // kMenuPresetBankChanged
  EventSubCategory::kMenuPresetSaveEvent,         // kMenuSavePreset
  EventSubCategory::kMenuPotEvent,                // kMenuPot0Moved
  EventSubCategory::kMenuPotEvent,                // kMenuPot1Moved
  EventSubCategory::kMenuPotEvent,                // kMenuPot2Moved
  EventSubCategory::kMenuPotEvent,                // kMenuMixPotMoved
  EventSubCategory::kMenuTempoEvent,              // kMenuTempoChanged
  EventSubCategory::kMenuExprEvent,               // kMenuExprStateToggled
  EventSubCategory::kMenuExprEvent,               // kMenuExprMappedPotMoved
  EventSubCategory::kMenuExprEvent,               // kMenuExprDirectionToggled
  EventSubCategory::kMenuExprEvent,               // kMenuExprHeelValueMoved
  EventSubCategory::kMenuExprEvent,               // kMenuExprToeValueMoved
  EventSubCategory::kProgramChangedEvent,         // kProgramChanged
  EventSubCategory::kPresetChangedEvent,          // kPresetChanged
  EventSubCategory::kPresetBankChangedEvent,      // kPresetBankChanged
  EventSubCategory::kTapIntervalEvent,            // kTapIntervalChanged
  EventSubCategory::kTempoChangedEvent,           // kTempoChanged
  EventSubCategory::kBypassEnabledEvent,          // kBypassEnabled
  EventSubCategory::kBypassDisabledEvent,         // kBypassDisabled
  EventSubCategory::kMidiBypassPressedEvent,      // kMidiBypassPressed
  EventSubCategory::kMidiTapPressedEvent,         // kMidiTapPressed
  EventSubCategory::kMidiPotMovedEvent,           // kMidiPot0Moved
  EventSubCategory::kMidiPotMovedEvent,           // kMidiPot1Moved
  EventSubCategory::kMidiPotMovedEvent,           // kMidiPot2Moved
  EventSubCategory::kMidiPotMovedEvent,           // kMidiMixPotMoved
  EventSubCategory::kMidiTempoChangedEvent,       // kMidiTempoChanged
  EventSubCategory::kMidiProgramChangedEvent,     // kMidiProgramChanged
  EventSubCategory::kMidiProgramModeChangedEvent, // kMidiProgramModeChange
  EventSubCategory::kSaveEvent,                   // kSaveBypass
  EventSubCategory::kSaveEvent,                   // kSaveProgramMode
  EventSubCategory::kSaveEvent,                   // kSaveCurrentProgram
  EventSubCategory::kSaveEvent,                   // kSaveCurrentPreset
  EventSubCategory::kSaveEvent,                   // kSaveMidiChannel
  EventSubCategory::kSaveEvent,                   // kSaveDeviceState
  EventSubCategory::kSaveEvent,                   // kSaveLogicalState
  EventSubCategory::kSaveEvent,                   // kSaveTap
  EventSubCategory::kSaveEvent,                   // kSaveTempo
  EventSubCategory::kSaveEvent,                   // kSavePot
  EventSubCategory::kSaveEvent,                   // kSaveExpr
  EventSubCategory::kSaveEvent,                   // kSavePreset
  EventSubCategory::kBankLoadEvent,               // kLoadPresetBank
  EventSubCategory::kBankLoadEvent                // kPresetBankLoaded
};

constexpr EventCategory eventToCategory(EventType t_type) {
  return eventCategoryMap[static_cast<uint8_t>(t_type)];
}

constexpr EventSubCategory EventToSubCategory(EventType t_type) {
  return eventSubCategoryMap[static_cast<uint8_t>(t_type)];
}

struct Event {
  EventType m_type;
  uint32_t m_timestamp;

  union {
    uint16_t value;
    int8_t delta;
  } m_data;
};

