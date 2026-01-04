#include "services/program_mode_service.h"
#include "ui/inputs.h"

void ProgramModeService::publishSaveProgramModeEvent(const Event& t_event) const {
  Event e;
  e.m_domain = EventDomain::kMemory;
  e.m_subject = EventSubject::kProgramMode;
  e.m_action = EventAction::kSave;
  e.m_timestamp = t_event.m_timestamp;

  EventBus::publish(e);
}

void ProgramModeService::publishProgramModeToggledEvent(const Event& t_event) const {
  Event e;
  e.m_domain = EventDomain::kLogic;
  e.m_subject = EventSubject::kProgramMode;
  e.m_action = EventAction::kToggled;
  e.m_timestamp = t_event.m_timestamp;

  EventBus::publish(e);
}

void ProgramModeService::init() {

}

void ProgramModeService::handleEvent(const Event& t_event) {
  m_logicalState.m_programMode == ProgramMode::kProgram
    ? m_logicalState.m_programMode = ProgramMode::kPreset
    : m_logicalState.m_programMode = ProgramMode::kProgram;

  publishProgramModeToggledEvent(t_event);
  publishSaveProgramModeEvent(t_event);
}

void ProgramModeService::update() {

}

bool ProgramModeService::interestedIn(const Event& t_event) const {
  if (t_event.m_domain == EventDomain::kPhysical
      && t_event.m_subject == EventSubject::kSwitch
      && t_event.m_action == EventAction::kLongPressed
      && t_event.matchesId(SwitchId::kProgramMode)) return true;

  return false;
}
