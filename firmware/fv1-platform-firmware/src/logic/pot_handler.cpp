#include "logic/pot_handler.h"

uint16_t PotHandler::mapMidiValue(uint8_t t_midiValue, uint8_t t_potIndex) {
  return Utils::mapClamped<uint16_t>(t_midiValue, MidiMessageConstants::c_minValue, MidiMessageConstants::c_maxValue, m_minValue[t_potIndex], m_maxValue[t_potIndex]);
}

uint16_t PotHandler::mapAdcValue(uint16_t t_adcValue, uint8_t t_potIndex) {
  // Remove magic numbers after ADC is written
  return Utils::mapClamped<uint16_t>(t_adcValue, 0, 1023, m_minValue[t_potIndex], m_maxValue[t_potIndex]);
}

uint16_t PotHandler::mapMenuValue(uint16_t t_currentValue, int8_t t_delta, uint8_t t_potIndex) {
  return Utils::clampedAdd(t_currentValue, t_delta, m_minValue[t_potIndex], m_maxValue[t_potIndex]);
}

PotState PotHandler::togglePotState(uint8_t t_potIndex) {
  m_state[t_potIndex] == PotState::kActive
    ? m_state[t_potIndex] = PotState::kDisabled
    : m_state[t_potIndex] = PotState::kActive;

  return m_state[t_potIndex];
}

uint16_t PotHandler::changePotMinValue(int8_t t_delta, uint8_t t_potIndex) {
  m_minValue[t_potIndex] = Utils::clampedAdd(m_minValue[t_potIndex], t_delta, PotConstants::c_potMaxValue);

  return m_minValue[t_potIndex];
}

uint16_t PotHandler::changePotMaxValue(int8_t t_delta, uint8_t t_potIndex) {
  m_maxValue[t_potIndex] = Utils::clampedAdd(m_maxValue[t_potIndex], t_delta, PotConstants::c_potMaxValue);

  return m_maxValue[t_potIndex];
}
