#include "services/program_mode_service.h"

void ProgramModeService::publishSaveProgramModeEvent(const Event& t_event) {
  EventBus::publish({EventType::kSaveProgramMode, t_event.m_timestamp /*millis()*/, {}});
}

ProgramModeService::ProgramModeService(LogicalState& t_lState)
    : m_logicalState(t_lState) {}

void ProgramModeService::init() {

}

void ProgramModeService::handleEvent(const Event& t_event) {
  m_logicalState.m_programMode == ProgramMode::kProgram
    ? m_logicalState.m_programMode = ProgramMode::kPreset
    : m_logicalState.m_programMode = ProgramMode::kProgram;

  publishSaveProgramModeEvent(t_event);
}

void ProgramModeService::update() {

}

bool ProgramModeService::interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const {
  return t_category == EventCategory::kProgramEvent && t_subCategory == EventSubCategory::kProgramModeChangedEvent;
}
