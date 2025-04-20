#include "services/program_service.h"

void ProgramService::syncActiveProgram() {
  m_logicState.m_activeProgram = &ProgramsDefinitions::kPrograms[m_logicState.m_currentProgram];
}

void ProgramService::init() {
  syncActiveProgram();
}

void ProgramService::handleEvent(const Event& e) {
  if (e.m_type != EventType::kProgramChanged) return;

  int max = static_cast<int>(ProgramConstants::c_maxPrograms);
  int delta = static_cast<int>(e.m_data.delta);
  if (delta < -max || delta > max) return;

  int curr = static_cast<int>(m_logicState.m_currentProgram);
  int next = (curr + delta) % max;
  if (next < 0) next += max;

  m_logicState.m_currentProgram = static_cast<uint8_t>(next);
  syncActiveProgram();
}

void ProgramService::update() {

}
