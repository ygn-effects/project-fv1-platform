#include "logic/expr_handler.h"

// Setters
void ExprHandler::setMappedPot(MappedPot pot) {
  m_mappedPot = pot;
}

void ExprHandler::setDirection(Direction direction) {
  m_direction = direction;
}

void ExprHandler::setHeelToeValues(uint16_t heel, uint16_t toe) {
  m_heelValue = heel;
  m_toeValue = toe;
}

void ExprHandler::setState(ExprState state) {
  m_state = state;
}

uint16_t ExprHandler::mapAdcValue(uint16_t adcValue) const {
  // Ensure ADC input is constrained to expected range
  adcValue = constrain(adcValue, 0, 1023);

  // Normalized value (0.0 to 1.0)
  float normalized = adcValue / 1023.0;

  // Map based on direction
  if (m_direction == Direction::kInverted) {
    normalized = 1.0 - normalized;
  }

  // Scale normalized value between heel and toe values
  return (m_heelValue + normalized * (m_toeValue - m_heelValue));
}
