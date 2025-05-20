#include "services/pot_service.h"

void PotService::syncHandler(uint8_t t_potIndex) {
  m_handler.m_state[t_potIndex] = m_logicalState.m_potParams[m_logicalState.m_currentProgram][t_potIndex].m_state;
  m_handler.m_minValue[t_potIndex] = m_logicalState.m_potParams[m_logicalState.m_currentProgram][t_potIndex].m_minValue;
  m_handler.m_maxValue[t_potIndex] = m_logicalState.m_potParams[m_logicalState.m_currentProgram][t_potIndex].m_maxValue;
}

void PotService::init() {
  for (uint8_t i = 0; i < PotConstants::c_potCount; i++) {
    syncHandler(i);
  }
}

void PotService::handleEvent(const Event& t_event) {
  switch (t_event.m_type) {
    case EventType::kPot0Moved:
      m_logicalState.m_potParams[m_logicalState.m_currentProgram][0].m_value = m_handler.mapAdcValue(t_event.m_data.value, 0);
      break;

    case EventType::kPot1Moved:
      m_logicalState.m_potParams[m_logicalState.m_currentProgram][1].m_value = m_handler.mapAdcValue(t_event.m_data.value, 1);
      break;

    case EventType::kPot2Moved:
      m_logicalState.m_potParams[m_logicalState.m_currentProgram][2].m_value = m_handler.mapAdcValue(t_event.m_data.value, 2);
      break;

    case EventType::kMixPotMoved:
      m_logicalState.m_potParams[m_logicalState.m_currentProgram][3].m_value = m_handler.mapAdcValue(t_event.m_data.value, 3);
      break;

    case EventType::kMenuPot0Moved: {
      int16_t newValue = static_cast<int16_t>(m_logicalState.m_potParams[m_logicalState.m_currentProgram][0].m_value) + t_event.m_data.delta;
      if (newValue < 0) newValue = 0;
      m_logicalState.m_potParams[m_logicalState.m_currentProgram][0].m_value = m_handler.mapAdcValue(static_cast<uint16_t>(newValue), 0);
      break;
    }

    case EventType::kMenuPot1Moved: {
      int16_t newValue = static_cast<int16_t>(m_logicalState.m_potParams[m_logicalState.m_currentProgram][1].m_value) + t_event.m_data.delta;
      if (newValue < 0) newValue = 0;
      m_logicalState.m_potParams[m_logicalState.m_currentProgram][1].m_value = m_handler.mapAdcValue(static_cast<uint16_t>(newValue), 1);
      break;
    }

    case EventType::kMenuPot2Moved: {
      int16_t newValue = static_cast<int16_t>(m_logicalState.m_potParams[m_logicalState.m_currentProgram][2].m_value) + t_event.m_data.delta;
      if (newValue < 0) newValue = 0;
      m_logicalState.m_potParams[m_logicalState.m_currentProgram][2].m_value = m_handler.mapAdcValue(static_cast<uint16_t>(newValue), 2);
      break;
    }

    case EventType::kMenuMixPotMoved: {
      int16_t newValue = static_cast<int16_t>(m_logicalState.m_potParams[m_logicalState.m_currentProgram][3].m_value) + t_event.m_data.delta;
      if (newValue < 0) newValue = 0;
      m_logicalState.m_potParams[m_logicalState.m_currentProgram][3].m_value = m_handler.mapAdcValue(static_cast<uint16_t>(newValue), 3);
      break;
    }

    case EventType::kMidiPot0Moved:
      m_logicalState.m_potParams[m_logicalState.m_currentProgram][0].m_value = m_handler.mapMidiValue(t_event.m_data.value, 0);
      break;

    case EventType::kMidiPot1Moved:
      m_logicalState.m_potParams[m_logicalState.m_currentProgram][1].m_value = m_handler.mapMidiValue(t_event.m_data.value, 1);
      break;

    case EventType::kMidiPot2Moved:
      m_logicalState.m_potParams[m_logicalState.m_currentProgram][2].m_value = m_handler.mapMidiValue(t_event.m_data.value, 2);
      break;

    case EventType::kMidiMixPotMoved:
      m_logicalState.m_potParams[m_logicalState.m_currentProgram][3].m_value = m_handler.mapMidiValue(t_event.m_data.value, 3);
      break;

    case EventType::kProgramChanged:
    case EventType::kProgramModeChanged:
      init();
      break;

    default:
      break;
  }
}

void PotService::update() {

}

bool PotService::interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const {
  return (t_category == EventCategory::kPhysicalEvent && t_subCategory == EventSubCategory::kPotEvent)
      || (t_category == EventCategory::kMenuEvent && t_subCategory == EventSubCategory::kMenuPotEvent)
      || (t_category == EventCategory::kMidiEvent && t_subCategory == EventSubCategory::kMidiPotMovedEvent)
      || (t_category == EventCategory::kProgramEvent && t_subCategory == EventSubCategory::kProgramChangedEvent)
      || (t_category == EventCategory::kProgramEvent && t_subCategory == EventSubCategory::kProgramModeChangedEvent);
}
