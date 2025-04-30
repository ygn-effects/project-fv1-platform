#include "services/pot_service.h"

void PotService::syncHandler(uint8_t t_potIndex) {
  m_handler.setState(m_logicalState.m_potParams[t_potIndex].m_state, t_potIndex);
  m_handler.setMinValue(m_logicalState.m_potParams[t_potIndex].m_minValue, t_potIndex);
  m_handler.setMaxValue(m_logicalState.m_potParams[t_potIndex].m_maxValue, t_potIndex);
}

void PotService::init() {

}

void PotService::handleEvent(const Event& t_event) {
  switch (t_event.m_type) {
    case EventType::kPot0Moved:
      m_logicalState.m_potParams[0].m_value = t_event.m_data.value;
      break;

    case EventType::kPot1Moved:
      m_logicalState.m_potParams[1].m_value = t_event.m_data.value;
      break;

    case EventType::kPot2Moved:
      m_logicalState.m_potParams[2].m_value = t_event.m_data.value;
      break;

    case EventType::kMixPotMoved:
      m_logicalState.m_potParams[3].m_value = t_event.m_data.value;
      break;

    default:
      break;
  }
}

void PotService::update() {

}

bool PotService::interestedIn(EventCategory t_category, EventSubCategory t_subCategory) const {
  return t_category == EventCategory::kPhysicalEvent && t_subCategory == EventSubCategory::kPotEvent;
}
