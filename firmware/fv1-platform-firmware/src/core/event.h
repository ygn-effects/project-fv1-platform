#pragma once

#include <stdint.h>

enum class EventDomain {
  kDriver,
  kPhysical,
  kUI,
  kMidi,
  kLogic,
  kMemory,
  kSystem
};

enum class EventSubject {
  kGeneral,
  kSwitch,
  kPot,
  kEncoder,
  kExpr,
  kProgram,
  kProgramMode,
  kPreset,
  kPresetBank,
  kTempo,
  kBypass,
  kMenu,
  kMidiChannel
};

enum class EventAction {
  kNoAction,
  kBooted,
  kStateChanged,
  kPressed,
  kReleased,
  kLongPressed,
  kValueChanged,
  kDeltaChanged,
  kToggled,
  kSave,
  kLoad,
  kLocked,
  kUnlocked,
  kUpdated,
  kSettingChanged,
  kSettingToggled
};

struct Event {
  EventDomain m_domain;
  EventSubject m_subject;
  EventAction m_action;

  uint8_t m_id;
  uint32_t m_timestamp;

  union {
    uint16_t value;
    int16_t delta;
    void* ptr;
  } m_data;

  template <typename T>
  bool matchesId(T t_targetId) const {
    return m_id == static_cast<uint8_t>(t_targetId);
  }
};
