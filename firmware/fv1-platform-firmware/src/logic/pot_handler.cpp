#include "logic/pot_handler.h"

uint16_t PotHandler::mapMidiValue(uint8_t t_midiValue, uint8_t t_potIndex) {
  return Utils::mapClamped<uint16_t>(t_midiValue, MidiMessageConstants::c_minValue, MidiMessageConstants::c_maxValue, m_minValue[t_potIndex], m_maxValue[t_potIndex]);
}

uint16_t PotHandler::mapAdcValue(uint16_t t_adcValue, uint8_t t_potIndex) {
  // Remove magic numbers after ADC is written
  return Utils::mapClamped<uint16_t>(t_adcValue, 0, 1023, m_minValue[t_potIndex], m_maxValue[t_potIndex]);
}
