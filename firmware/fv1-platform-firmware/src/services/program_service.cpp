#include "services/program_service.h"

void ProgramService::syncActiveProgram() {
  m_logicState.m_activeProgram = &ProgramsDefinitions::kPrograms[m_logicState.m_currentProgram];
}

void ProgramService::publishProgramChangeEvent(const Event& t_event) {
  Event e;
  e.m_type = EventType::kProgramChanged;
  e.m_timestamp = t_event.m_timestamp; /*millis()*/
  e.m_data.value = m_logicState.m_currentProgram;
  EventBus::publish(e);
}

void ProgramService::init() {
  syncActiveProgram();
}

void ProgramService::handleEvent(const Event& t_event) {
  int max = static_cast<int>(ProgramConstants::c_maxPrograms);
  int delta = static_cast<int>(t_event.m_data.delta);
  if (delta < -max || delta > max) return;

  int curr = static_cast<int>(m_logicState.m_currentProgram);
  int next = (curr + delta) % max;
  if (next < 0) next += max;

  m_logicState.m_currentProgram = static_cast<uint8_t>(next);

  syncActiveProgram();
  publishProgramChangeEvent(t_event);
}

void ProgramService::update() {

}

bool ProgramService::interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const {
  return t_category == EventCategory::kMenuEvent && t_subCategory == EventSubCategory::kMenuProgramChangedEvent;
}

