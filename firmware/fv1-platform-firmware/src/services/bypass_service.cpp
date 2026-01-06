#include "services/bypass_service.h"

void BypassService::publishSaveBypassEvent(const Event& t_event) const {
  Event e;
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kBypass;
  e.m_action = EventAction::kSave;
  e.m_timestamp = t_event.m_timestamp;

  EventBus::publish(e);
}

void BypassService::publishBypassToggledEvent(const Event& t_event) const {
  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kBypass;
  e.m_action = EventAction::kToggled;
  e.m_timestamp = t_event.m_timestamp;

  EventBus::publish(e);
}

void BypassService::init() {
  m_bypass.init();

  m_logicalState.m_bypassState == BypassState::kActive
    ? m_bypass.on()
    : m_bypass.off();
}

void BypassService::handleEvent(const Event& t_event) {
  switch (t_event.m_domain) {
    case EventDomain::kPhysical:
      m_logicalState.m_bypassState = m_logicalState.m_bypassState == BypassState::kActive
        ? BypassState::kBypassed
        : BypassState::kActive;

      m_bypass.toggle();

      publishBypassToggledEvent(t_event);
      publishSaveBypassEvent(t_event);
      break;

    case EventDomain::kMidi:
      if (t_event.m_data.value == MidiCCValues::c_bypassDisable
          && m_logicalState.m_bypassState != BypassState::kBypassed) {
        m_logicalState.m_bypassState = BypassState::kBypassed;
        m_bypass.off();

        publishBypassToggledEvent(t_event);
        publishSaveBypassEvent(t_event);
      }
      else if (t_event.m_data.value == MidiCCValues::c_bypassEnable
              && m_logicalState.m_bypassState != BypassState::kActive) {
        m_logicalState.m_bypassState = BypassState::kActive;
        m_bypass.on();

        publishBypassToggledEvent(t_event);
        publishSaveBypassEvent(t_event);
      }

      break;

    default:
      break;
  }
}

void BypassService::update() {

}

bool BypassService::interestedIn(const Event& t_event) const {
  if (t_event.m_domain == EventDomain::kPhysical) {
    if (t_event.m_subject == EventSubject::kSwitch
        && t_event.m_action == EventAction::kPressed
        && t_event.matchesId(SwitchId::kBypass)) return true;
  }

  if (t_event.m_domain == EventDomain::kMidi) {
    if (t_event.m_subject == EventSubject::kSwitch
        && t_event.m_action == EventAction::kValueChanged
        && t_event.matchesId(SwitchId::kBypass)) return true;
  }

  return false;
}
