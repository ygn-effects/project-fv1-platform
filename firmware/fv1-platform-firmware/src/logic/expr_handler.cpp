#include "logic/expr_handler.h"

uint16_t ExprHandler::mapAdcValue(uint16_t t_adcValue) const {
  // 1) Clamp the raw ADC to [0,1023]
  uint16_t x = Utils::clamp<uint16_t>(t_adcValue, uint16_t{0}, uint16_t{1023});

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

ExprState ExprHandler::toggleExprState() {
  m_state == ExprState::kActive
    ? m_state = ExprState::kInactive
    : m_state = ExprState::kActive;

  return m_state;
}

MappedPot ExprHandler::changeMappedPot(int8_t t_delta) {
  m_mappedPot = EnumUtils::wrappedEnumAdd<MappedPot, ExprHandlerConstants::kMappedPotCount>(m_mappedPot, t_delta);

  return m_mappedPot;
}

Direction ExprHandler::toggleDirection() {
  m_direction == Direction::kNormal
    ? m_direction = Direction::kInverted
    : m_direction = Direction::kNormal;

  return m_direction;
}

uint16_t ExprHandler::changeHeelValue(int8_t t_delta) {
  m_heelValue = Utils::clampedAdd(m_heelValue, t_delta, ExprHandlerConstants::kMaxExprValue);

  return m_heelValue;
}

uint16_t ExprHandler::changeToeValue(int8_t t_delta) {
  m_toeValue = Utils::clampedAdd(m_toeValue, t_delta, ExprHandlerConstants::kMaxExprValue);

  return m_toeValue;
}
