#include "logic/pot_handler.h"

uint16_t PotHandler::mapMidiValue(uint8_t t_midiValue, uint8_t t_potIndex) {
  return Utils::mapClamped<uint16_t>(t_midiValue, MidiMessageConstants::c_minValue, MidiMessageConstants::c_maxValue, m_minValue[t_potIndex], m_maxValue[t_potIndex]);
}

uint16_t PotHandler::mapAdcValue(uint16_t t_adcValue, uint8_t t_potIndex) {
  return Utils::mapClamped<uint16_t>(t_adcValue, DACConstants::c_minDacValue, DACConstants::c_maxDacValue, m_minValue[t_potIndex], m_maxValue[t_potIndex]);
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

bool PotHandler::checkPickup(uint16_t t_mappedValue, uint16_t t_storedValue, uint8_t t_potIndex) {
  if (m_pickedUp[t_potIndex]) return true;

  if (!m_hasReference[t_potIndex]) {
    m_lastMappedValue[t_potIndex] = t_mappedValue;
    m_hasReference[t_potIndex] = true;
    return false;
  }

  bool crossed = (m_lastMappedValue[t_potIndex] <= t_storedValue && t_mappedValue >= t_storedValue)
              || (m_lastMappedValue[t_potIndex] >= t_storedValue && t_mappedValue <= t_storedValue);

  m_lastMappedValue[t_potIndex] = t_mappedValue;

  if (crossed) {
    m_pickedUp[t_potIndex] = true;
    return true;
  }

  return false;
}

void PotHandler::resetPickup() {
  for (uint8_t i = 0; i < PotConstants::c_potCount; i++) {
    m_pickedUp[i] = false;
    m_hasReference[i] = false;
  }
}
