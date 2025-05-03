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

uint16_t PotHandler::mapMidiValue(uint8_t t_midiValue, uint8_t t_potIndex) {
  return Utils::mapValue<uint16_t>(t_midiValue, 0, 127, m_minValue[t_potIndex], m_maxValue[t_potIndex]);
}
