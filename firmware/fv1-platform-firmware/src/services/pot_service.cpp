#include "services/pot_service.h"

void PotService::init() {
  for (uint8_t i = 0; i < PotConstants::c_potCount; i++) {
    m_handler.setState(m_logicalState.m_potParams[i].m_state, i);
    m_handler.setMinValue(m_logicalState.m_potParams[i].m_minValue, i);
    m_handler.setMaxValue(m_logicalState.m_potParams[i].m_maxValue, i);
  }
}

void PotService::handleEvent(const Event& t_event) {

}

void PotService::update() {

}
