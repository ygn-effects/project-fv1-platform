#include "services/program_service.h"

void ProgramService::syncActiveProgram() {
  m_logicState.m_activeProgram = &ProgramsDefinitions::kPrograms[m_logicState.m_currentProgram];
}

void ProgramService::publishProgramChangeEvent(const Event& t_event) {
  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kValueChanged;
  e.m_timestamp = t_event.m_timestamp;

  EventBus::publish(e);
}

void ProgramService::publishSaveCurrentProgram(const Event& t_event) {
  Event e;
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kProgram;
  e.m_action = EventAction::kSave;
  e.m_timestamp = t_event.m_timestamp;

  EventBus::publish(e);
}

void ProgramService::init() {
  syncActiveProgram();
}

void ProgramService::handleEvent(const Event& t_event) {
  if (t_event.m_domain == EventDomain::kUI && t_event.m_subject == EventSubject::kProgram) {
    int16_t max = static_cast<int16_t>(ProgramConstants::c_maxPrograms);
    int16_t delta = static_cast<int16_t>(t_event.m_data.delta);
    if (delta < -max || delta > max) return;

    int16_t curr = static_cast<int16_t>(m_logicState.m_currentProgram);
    int16_t next = (curr + delta) % max;
    if (next < 0) next += max;

    m_logicState.m_currentProgram = static_cast<uint8_t>(next);

    syncActiveProgram();
    publishProgramChangeEvent(t_event);
    publishSaveCurrentProgram(t_event);
  }
  else {
    syncActiveProgram();
    publishProgramChangeEvent(t_event);
  }
}

void ProgramService::update() {

}

bool ProgramService::interestedIn(const Event& t_event) const {
  if (t_event.m_domain == EventDomain::kUI)
    if (t_event.m_subject == EventSubject::kProgram
        && t_event.m_action == EventAction::kValueChanged) return true;
    if (t_event.m_subject == EventSubject::kPreset
        && t_event.m_action == EventAction::kValueChanged) return true;

  if (t_event.m_domain == EventDomain::kLogic
      && t_event.m_subject == EventSubject::kProgramMode
      && t_event.m_action == EventAction::kToggled) return true;

  if (t_event.m_domain == EventDomain::kMemory
      && t_event.m_subject == EventSubject::kPresetBank
      && t_event.m_action == EventAction::kLoad) return true;

  return false;
}
