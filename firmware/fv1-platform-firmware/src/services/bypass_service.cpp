#include "services/bypass_service.h"

void BypassService::init() {
  m_bypass.init();

  m_logicalState.m_bypassState == BypassState::kActive
    ? m_bypass.on()
    : m_bypass.off();
}

void BypassService::handleEvent(const Event& t_event) {
  switch (t_event.m_type) {
    case EventType::kBypassPressed:
      m_logicalState.m_bypassState = m_logicalState.m_bypassState == BypassState::kActive
        ? BypassState::kBypassed
        : BypassState::kActive;

      m_bypass.toggle();
      break;

    case EventType::kMidiBypassPressed:
      if (t_event.m_data.value == MidiCCValues::c_bypassDisable) {
        m_logicalState.m_bypassState = BypassState::kBypassed;
        m_bypass.off();
      }
      else if (t_event.m_data.value == MidiCCValues::c_bypassEnable) {
        m_logicalState.m_bypassState = BypassState::kActive;
        m_bypass.on();
      }

      break;

    default:
      break;
  }

  EventBus::publish({m_logicalState.m_bypassState == BypassState::kActive
    ? EventType::kBypassEnabled
    : EventType::kBypassDisabled, t_event.m_timestamp, {}});
}

void BypassService::update() {

}

bool BypassService::interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const {
  return (t_category == EventCategory::kPhysicalEvent && t_subCategory == EventSubCategory::kBypassEvent)
      || (t_category == EventCategory::kMidiEvent && t_subCategory == EventSubCategory::kMidiBypassPressedEvent);
}
