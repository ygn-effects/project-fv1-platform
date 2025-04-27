#include "logic/pot_handler.h"

void PotHandler::setState(PotState t_state, uint8_t t_potIndex) {
  m_state[t_potIndex] = t_state;
}

void PotHandler::setMinValue(uint16_t t_min, uint8_t t_potIndex) {
  m_minValue[t_potIndex] = t_min;
}

void PotHandler::setMaxValue(uint16_t t_max, uint8_t t_potIndex) {
  m_maxValue[t_potIndex] = t_max;
}
