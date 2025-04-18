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
  // 1) Clamp the raw ADC to [0,1023]
  uint16_t x = std::clamp(adcValue, uint16_t{0}, uint16_t{1023});

  // 2) Compute the full span between heel and toe
  uint16_t span = m_toeValue - m_heelValue;

  // 3) Depending on direction, choose x or (1023-x)
  uint16_t pos = (m_direction == Direction::kInverted)
                  ? 1023 - x
                  : x;

  // 4) Scale pos/1023 * span with rounding
  //    (pos * span + 1023/2) / 1023
  uint32_t scaled = static_cast<uint32_t>(pos) * span + 511;
  uint16_t delta  = static_cast<uint16_t>(scaled / 1023);

  // 5) Return heel + delta
  return m_heelValue + delta;
}