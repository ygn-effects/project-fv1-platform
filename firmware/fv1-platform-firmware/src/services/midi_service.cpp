#include "services/midi_service.h"

void MidiService::syncHandler() {
  m_midiHandler.setMidiChannel(m_logicalState.m_midiChannel);
}

void MidiService::init() {
  syncHandler();
}

void MidiService::handleEvent(const Event& t_event) {
  syncHandler();
}

void MidiService::update() {
  // Add Arduino serial code

  MidiMessage message;
  if (m_midiHandler.popMessage(message)) {
    switch (message.m_type) {
      case MidiMessageType::kControlChange: {
        EventType eventType = ccParamToEvent(message.m_param);

        switch (eventType) {
          case EventType::kMidiBypassPressed:
            if (message.m_value != MidiCCValues::c_bypassEnable && message.m_value != MidiCCValues::c_bypassDisable) { return; }
            EventBus::publish({eventType, 0 /*millis()*/, {.value=message.m_value}});
            break;

          case EventType::kMidiTapPressed:
            if (message.m_value != MidiCCValues::c_tapShortPress && message.m_value != MidiCCValues::c_tapLongPress) { return; }
            EventBus::publish({eventType, 0 /*millis()*/, {.value=message.m_value}});
            break;

          default:
            EventBus::publish({eventType, 0 /*millis()*/, {.value=message.m_value}});
            break;
        }
        break;
      }

      case MidiMessageType::kProgramChange:
        EventBus::publish({EventType::kMidiProgramChanged, 0 /*millis()*/, {.value=message.m_param}});
        break;

      default:
        break;
    }
  }
}

bool MidiService::interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const {
  return t_category == EventCategory::kProgramEvent && t_subCategory == EventSubCategory::kProgramChangedEvent;
}

