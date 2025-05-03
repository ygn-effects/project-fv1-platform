#pragma once

#include <stdint.h>
#include "core/service.h"
#include "core/event_bus.h"
#include "logic/logical_state.h"
#include "logic/midi_handler.h"

constexpr EventType ccParamEventMap[] = {
  EventType::kMidiPot0Moved,
  EventType::kMidiPot1Moved,
  EventType::kMidiPot2Moved,
  EventType::kMidiMixPotMoved,
  EventType::kMidiBypassPressed,
  EventType::kMidiTapPressed,
  EventType::kMidiTempoChanged,
  EventType::kMidiProgramChanged,
  EventType::kMidiProgramModeChanged
};

namespace MidiCCValues {
  constexpr uint8_t c_bypassDisable = 0;
  constexpr uint8_t c_bypassEnable = 127;
  constexpr uint8_t c_tapShortPress = 0;
  constexpr uint8_t c_tapLongPress = 127;
}

constexpr EventType ccParamToEvent(uint8_t t_ccParam) {
  return ccParamEventMap[t_ccParam];
}

class MidiService : public Service {
  private:
    LogicalState& m_logicalState;
    MidiHandler m_midiHandler;

    void syncHandler();

  public:
    MidiService(LogicalState& t_lState) :
      m_logicalState(t_lState) {}

    void init() override;
    void handleEvent(const Event& t_event) override;
    void update() override;
    bool interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const override;

    // Tests
    MidiHandler* getMidiHandler() { return &m_midiHandler; }
};
