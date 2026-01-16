#include "services/fv1_service.h"

void Fv1Service::init() {
  m_fv1.sendProgramChange(m_logicalState.m_currentProgram);

  if (m_logicalState.m_activeProgram->m_isDelayEffect) {
    m_handler.m_tempoConfig.m_minLogical = m_logicalState.m_activeProgram->m_minDelayMs;
    m_handler.m_tempoConfig.m_maxLogical = m_logicalState.m_activeProgram->m_maxDelayMs;
    m_fv1.sendPotValue(Fv1Pot::Pot0, m_handler.mapTempoValue(m_logicalState.m_tempo));
  }
  else {
    m_fv1.sendPotValue(Fv1Pot::Pot0, m_logicalState.m_potParams[m_logicalState.m_currentProgram][0].m_value);
  }

  m_fv1.sendPotValue(Fv1Pot::Pot1, m_logicalState.m_potParams[m_logicalState.m_currentProgram][1].m_value);
  m_fv1.sendPotValue(Fv1Pot::Pot2, m_logicalState.m_potParams[m_logicalState.m_currentProgram][2].m_value);
}

void Fv1Service::handleEvent(const Event& t_event) {
  if (t_event.m_domain == EventDomain::kLogic) {
    if (t_event.m_subject == EventSubject::kProgram
        && t_event.m_action == EventAction::kValueChanged) {
      init();
    }

    if (t_event.m_subject == EventSubject::kPot
        && t_event.m_action == EventAction::kValueChanged) {
      m_fv1.sendPotValue(static_cast<Fv1Pot>(t_event.m_id), m_logicalState.m_potParams[m_logicalState.m_currentProgram][t_event.m_id].m_value);
    }

    if (t_event.m_subject == EventSubject::kTempo
        && t_event.m_action == EventAction::kValueChanged) {
      m_fv1.sendPotValue(Fv1Pot::Pot0, m_handler.mapTempoValue(m_logicalState.m_tempo));
    }
  }
}

void Fv1Service::update() {

}

bool Fv1Service::interestedIn(const Event& t_event) const {
  if (t_event.m_domain == EventDomain::kLogic) {
    if (t_event.m_subject == EventSubject::kProgram
        && t_event.m_action == EventAction::kValueChanged) return true;

    if (t_event.m_subject == EventSubject::kTempo
        && t_event.m_action == EventAction::kValueChanged) return true;

    if (t_event.m_subject == EventSubject::kPot
        && t_event.m_action == EventAction::kValueChanged) return true;
  }

  return false;
}
