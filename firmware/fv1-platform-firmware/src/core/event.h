#pragma once

#include <stdint.h>

enum class EventDomain {
  kPhysical,
  kUI,
  kMidi,
  kLogic,
  kSystem
};

enum class EventSUbject {
  kGeneral,
  kSwitch,
  kPot,
  kEncoder,
  kProgram,
  kPreset,
  kTempo,
  kBypass,
  kMenu
};

enum class EventACtion {
  kNoAction,
  kBooted,
  kPressed,
  kReleased,
  kLongPressed,
  kValueChanged,
  kDeltaChanged,
  kSave,
  kLoad,
  kUpdated
};

struct Event {
  EventDomain m_domain;
  EventSUbject m_subject;
  EventACtion m_action;

  uint8_t m_id;
  uint32_t m_timestamp;

  union {
    uint16_t value;
    int16_t delta;
    void* ptr;
  } m_data;
};
