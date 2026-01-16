#include "services/midi_service.h"

void MidiService::syncHandler() {
  m_midiHandler.setMidiChannel(m_logicalState.m_midiChannel);
}

void MidiService::publishSaveMidiChannelEvent(const Event& t_event) {
  Event e;
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kGeneral;
  e.m_action = EventAction::kSave;
  e.m_timestamp = t_event.m_timestamp;

  EventBus::publish(e);
}

void MidiService::init() {
  syncHandler();
}

void MidiService::handleEvent(const Event& t_event) {
  // Program change
  if (t_event.m_subject == EventSubject::kProgram && t_event.m_action == EventAction::kValueChanged) {
    syncHandler();
  }

  // MIDI channel change
  if (t_event.m_domain == EventDomain::kMidi && t_event.m_action == EventAction::kValueChanged) {
    m_logicalState.m_midiChannel = Utils::wrappedAdd(m_logicalState.m_midiChannel, t_event.m_data.delta, MidiHandlerConstants::c_maxMidiChannels);

    syncHandler();
    publishSaveMidiChannelEvent(t_event);
  }
}

void MidiService::update() {
  // Add Arduino serial code

  MidiMessage message;

  if (m_midiHandler.popMessage(message)) {
    if (message.m_type == MidiMessageType::kControlChange) {
      if (message.m_param < c_ccMapSize) {
        const MidiEventDefinition& definition = c_ccMap[message.m_param];

        Event e;
        e.m_domain = EventDomain::kMidi;
        e.m_subject = definition.m_subject;
        e.m_action = definition.m_action;
        e.m_timestamp = 0; // millis()
        e.m_id = definition.m_id;
        e.m_data.value = message.m_value;

        EventBus::publish(e);
      }
    }
  }

  if (message.m_type == MidiMessageType::kProgramChange) {
        Event e;
        e.m_domain = EventDomain::kMidi;
        e.m_subject = EventSubject::kProgram;
        e.m_action = EventAction::kValueChanged;
        e.m_timestamp = 0; // millis()
        e.m_id = 0;
        e.m_data.value = message.m_param;

        EventBus::publish(e);
  }
}

bool MidiService::interestedIn(const Event& t_event) const {
  // MIDI channel change
  if (t_event.m_domain == EventDomain::kMidi
      && t_event.m_subject == EventSubject::kGeneral
      && t_event.m_action == EventAction::kValueChanged) return true;

  // Program change
  if (t_event.m_domain == EventDomain::kLogic
      && t_event.m_subject == EventSubject::kProgram
      && t_event.m_action == EventAction::kValueChanged) return true;

  return false;
}

