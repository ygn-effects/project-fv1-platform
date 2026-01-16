#include "services/tap_service.h"

void TapService::syncHandler() {
  m_tapHandler.m_tapState = m_logicalState.m_tapState;
  m_tapHandler.m_divState = m_logicalState.m_divState;
  m_tapHandler.m_divValue = m_logicalState.m_divValue;
  m_tapHandler.m_interval = m_logicalState.m_interval;
  m_tapHandler.m_divInterval = m_logicalState.m_divInterval;
}

void TapService::publishTapIntervalEvent(const Event& t_event) const {
  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kTap;
  e.m_action = EventAction::kValueChanged;
  e.m_data.value = m_tapHandler.m_divState == DivState::kDisabled
                    ? m_tapHandler.m_interval
                    : m_tapHandler.m_divInterval;

  EventBus::publish(e);
}

void TapService::publishSaveTapEvent(const Event& t_event) const {
  Event e;
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kTap;
  e.m_action = EventAction::kSave;

  EventBus::publish(e);
}

void TapService::init() {
  syncHandler();
}

void TapService::handleEvent(const Event& t_event) {
  if (t_event.m_domain == EventDomain::kLogic) {
    if (t_event.m_subject == EventSubject::kProgram
        && t_event.m_action == EventAction::kValueChanged) {
      if (! m_logicalState.m_activeProgram->m_isDelayEffect
          || ! m_logicalState.m_activeProgram->m_supportsTap) {
        m_logicalState.m_tapState = TapState::kDisabled;
        m_logicalState.m_divState = DivState::kDisabled;
        m_logicalState.m_divValue = DivValue::kQuarter;

        publishSaveTapEvent(t_event);
        syncHandler();
        return;
      }
      else {
        syncHandler();
        return;
      }
    }
  }

  if (! m_logicalState.m_activeProgram->m_supportsTap) return;

  if (t_event.matchesId(SwitchId::kTap)) {
    if (t_event.m_action == EventAction::kPressed
        || t_event.m_action == EventAction::kValueChanged
        && t_event.m_data.value == MidiCCValues::c_tapShortPress) {
      m_tapHandler.registerTap(t_event.m_timestamp);

      if (m_tapHandler.m_isNewIntervalSet) {
        m_logicalState.m_interval = m_tapHandler.m_interval;

        publishTapIntervalEvent(t_event);
        publishSaveTapEvent(t_event);
      }

      m_logicalState.m_tapState = m_tapHandler.m_tapState;
    }

    if (m_logicalState.m_tapState == TapState::kEnabled) {
      if (t_event.m_action == EventAction::kLongPressed
          || t_event.m_action == EventAction::kValueChanged
          && t_event.m_data.value == MidiCCValues::c_tapLongPress) {
        m_tapHandler.setNextDivValue();

        m_logicalState.m_divState = m_tapHandler.m_divState;
        m_logicalState.m_divValue = m_tapHandler.m_divValue;
        m_logicalState.m_divInterval = m_tapHandler.m_divInterval;

        publishTapIntervalEvent(t_event);
        publishSaveTapEvent(t_event);
      }
    }
  }

  if (t_event.matchesId(PotId::kPot0)
      || t_event.m_domain == EventDomain::kUI
      && t_event.m_subject == EventSubject::kTempo) {
    m_logicalState.m_tapState = TapState::kDisabled;
    m_logicalState.m_divState = DivState::kDisabled;
    m_logicalState.m_divValue = DivValue::kQuarter;

    publishSaveTapEvent(t_event);
    syncHandler();
  }
}

void TapService::update() {

}

bool TapService::interestedIn(const Event& t_event) const {
  if (t_event.m_domain == EventDomain::kLogic) {
    if (t_event.m_subject == EventSubject::kProgram
        && t_event.m_action == EventAction::kValueChanged) return true;
  }

  if (t_event.m_domain == EventDomain::kUI) {
    if (t_event.m_subject == EventSubject::kTempo
        && t_event .m_action == EventAction::kValueChanged) return true;
  }

  if (t_event.m_domain == EventDomain::kPhysical) {
    if (t_event.m_subject == EventSubject::kSwitch
        && t_event.matchesId(SwitchId::kTap)) return true;

    if (t_event.m_subject == EventSubject::kPot
        && t_event.m_action == EventAction::kValueChanged
        && t_event.matchesId(PotId::kPot0)) return true;
  }

  if (t_event.m_domain == EventDomain::kMidi) {
    if (t_event.m_subject == EventSubject::kSwitch
        && t_event.matchesId(SwitchId::kTap)) return true;
  }

  return false;
}
