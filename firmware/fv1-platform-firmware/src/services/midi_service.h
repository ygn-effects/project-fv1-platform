#pragma once

#include <stdint.h>
#include "core/service.h"
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "logic/midi_handler.h"
#include "periphs/clock.h"
#include "periphs/serial.h"
#include "ui/inputs.h"
#include "utils/utils.h"

struct MidiEventDefinition {
  EventSubject m_subject;
  EventAction m_action;
  uint8_t m_id;
};

constexpr MidiEventDefinition c_ccMap[] = {
  /* CC 0 */ { EventSubject::kPot, EventAction::kValueChanged, static_cast<uint8_t>(PotId::kPot0) },
  /* CC 1 */ { EventSubject::kPot, EventAction::kValueChanged, static_cast<uint8_t>(PotId::kPot1) },
  /* CC 2 */ { EventSubject::kPot, EventAction::kValueChanged, static_cast<uint8_t>(PotId::kPot2) },
  /* CC 3 */ { EventSubject::kPot, EventAction::kValueChanged, static_cast<uint8_t>(PotId::kMixPot) },
  /* CC 4 */ { EventSubject::kSwitch, EventAction::kValueChanged, static_cast<uint8_t>(SwitchId::kBypass) },
  /* CC 5 */ { EventSubject::kSwitch, EventAction::kValueChanged, static_cast<uint8_t>(SwitchId::kTap) },
  /* CC 6 */ { EventSubject::kTempo, EventAction::kValueChanged, 0 },
  /* CC 7 */ { EventSubject::kProgramMode, EventAction::kValueChanged, 0 },
};

constexpr uint8_t c_ccMapSize = sizeof(c_ccMap) / sizeof(MidiEventDefinition);

namespace MidiCCValues {
  constexpr uint8_t c_bypassDisable = 0;
  constexpr uint8_t c_bypassEnable = 127;
  constexpr uint8_t c_tapShortPress = 0;
  constexpr uint8_t c_tapLongPress = 127;
  constexpr uint8_t c_programMode = 0;
  constexpr uint8_t c_presetMode = 127;
}

class MidiService : public Service {
  private:
    LogicalState& m_logicalState;
    MidiHandler m_midiHandler;
    SerialInterface& m_serial;
    Clock& m_clock;

    void syncHandler();
    void publishSaveMidiChannelEvent(const Event& t_event);

  public:
    MidiService(LogicalState& t_lState, SerialInterface& t_serial, Clock& t_clock) :
      m_logicalState(t_lState),
      m_serial(t_serial),
      m_clock(t_clock) {}

    void init() override;
    void handleEvent(const Event& t_event) override;
    void update() override;
    bool interestedIn(const Event& t_event) const override;
};
