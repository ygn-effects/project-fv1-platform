#include "services/tempo_service.h"

void TempoService::syncHandler() {
  m_handler.m_interval = m_logicState.m_tempo;
  m_handler.m_minInterval = m_logicState.m_activeProgram->m_minDelayMs;
  m_handler.m_maxInterval = m_logicState.m_activeProgram->m_maxDelayMs;
}

void TempoService::publishTempoEvent(uint16_t t_interval) const {
  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kTempo;
  e.m_action = EventAction::kValueChanged;
  e.m_timestamp = m_clock.now();
  e.m_data.value = t_interval;

  EventBus::publish(e);
}

void TempoService::publishSaveTempoEvent(uint16_t t_interval) const {
  Event e;
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kTempo;
  e.m_action = EventAction::kSave;

  EventBus::publish(e);
}

void TempoService::init() {
  syncHandler();
  m_tempoLed.init();
}

void TempoService::handleEvent(const Event& t_event) {
  if (m_logicState.m_programMode != ProgramMode::kProgram) return;

  if (t_event.m_domain == EventDomain::kLogic
      && t_event.m_subject == EventSubject::kProgram
      && t_event.m_action == EventAction::kValueChanged) {
    if (m_logicState.m_activeProgram->m_isDelayEffect) {
      syncHandler();
      m_handler.m_source = TempoSource::kTap;
      m_logicState.m_tempo = m_handler.mapInterval(m_handler.m_interval);

      // publishTempoEvent(m_logicState.m_tempo);
      publishSaveTempoEvent(m_logicState.m_tempo);
    }
    else {
      m_tempoLed.off();
    }
  }

  if (! m_logicState.m_activeProgram->m_isDelayEffect) return;

  if (t_event.m_domain == EventDomain::kLogic) {
    if (t_event.m_subject == EventSubject::kTap
        && t_event.m_action == EventAction::kValueChanged) {
      m_handler.m_source = TempoSource::kTap;
      m_logicState.m_tempo = m_handler.mapInterval(t_event.m_data.value);

      publishTempoEvent(m_logicState.m_tempo);
      publishSaveTempoEvent(m_logicState.m_tempo);

      return;
    }

    if (t_event.m_subject == EventSubject::kTempo
        && t_event.m_action == EventAction::kInputChanged) {
      m_handler.m_source = TempoSource::kPot;
      m_logicState.m_tempo = m_handler.mapInterval(t_event.m_data.value);

      publishTempoEvent(m_logicState.m_tempo);
      publishSaveTempoEvent(m_logicState.m_tempo);

      return;
    }
  }

  if (t_event.m_domain == EventDomain::kUI) {
    if (t_event.m_subject == EventSubject::kTempo
        && t_event.m_action == EventAction::kValueChanged) {
      m_handler.m_source = TempoSource::kMenu;
      m_logicState.m_tempo = m_handler.mapInterval(t_event.m_data.delta);

      publishTempoEvent(m_logicState.m_tempo);
      publishSaveTempoEvent(m_logicState.m_tempo);

      return;
    }
  }
}

void TempoService::update() {
  if (m_logicState.m_activeProgram->m_isDelayEffect && m_logicState.m_tempo > 0) {
    m_tempoLed.setValue(m_handler.calculateTempoLedValue(m_clock.now()));
  }
}

bool TempoService::interestedIn(const Event& t_event) const {
  if (t_event.m_domain == EventDomain::kLogic) {
    if (t_event.m_subject == EventSubject::kProgram
        && t_event.m_action == EventAction::kValueChanged) return true;
    if (t_event.m_subject == EventSubject::kTap
        && t_event.m_action == EventAction::kValueChanged) return true;
    if (t_event.m_subject == EventSubject::kTempo
        && t_event.m_action == EventAction::kInputChanged) return true;
  }

  if (t_event.m_domain == EventDomain::kUI) {
    if (t_event.m_subject == EventSubject::kTempo
        && t_event.m_action == EventAction::kValueChanged) return true;
  }

  return false;
}
