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

void ProgramService::publishSaveCurrentProgram(const Event& t_event) {
  EventBus::publish({EventType::kSaveCurrentProgram, t_event.m_timestamp /*millis()*/, {}});
}

void ProgramService::init() {
  syncActiveProgram();
}

void ProgramService::handleEvent(const Event& t_event) {
  switch (t_event.m_type) {
    case EventType::kMenuProgramChanged: {
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
      break;
    }

    case EventType::kMenuPresetChanged:
    case EventType::kPresetBankLoaded:
    case EventType::kProgramModeChanged:
      syncActiveProgram();
      publishProgramChangeEvent(t_event);
      break;

    default:
      break;
  }
}

void ProgramService::update() {

}

bool ProgramService::interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const {
  return (t_category == EventCategory::kMenuEvent && t_subCategory == EventSubCategory::kMenuProgramChangedEvent)
      || (t_category == EventCategory::kMenuEvent && t_subCategory == EventSubCategory::kMenuPresetChangedEvent)
      || (t_category == EventCategory::kProgramEvent && t_subCategory == EventSubCategory::kProgramChangedEvent)
      || (t_category == EventCategory::kLoadEvent && t_subCategory == EventSubCategory::kBankLoadEvent)
      || (t_category == EventCategory::kProgramEvent && t_subCategory == EventSubCategory::kProgramModeChangedEvent);
}
