#pragma once

#include <stdint.h>
#include "utils/enum_utils.h"
#include "utils/utils.h"

/**
 * @brief Expression pedal state (active/inactive)
 */
enum class ExprState : uint8_t {
  kInactive,
  kActive
};

using ExprStateValidator = EnumUtils::EnumValidator<ExprState, ExprState::kInactive, ExprState::kActive>;

/**
 * @brief Parameter controlled by the expression pedal
 */
enum class MappedPot : uint8_t {
  kPot0,
  kPot1,
  kPot2,
  kMixPot
};

using MappedPotValidator = EnumUtils::EnumValidator<MappedPot, MappedPot::kPot0, MappedPot::kPot1, MappedPot::kPot2, MappedPot::kMixPot>;

/**
 * @brief Direction of the expression pedal mapping
 */
enum class Direction : uint8_t {
  kNormal,   // Heel = minimum, Toe = maximum
  kInverted  // Heel = maximum, Toe = minimum
};

using DirectionValidator = EnumUtils::EnumValidator<Direction, Direction::kNormal, Direction::kInverted>;

namespace ExprHandlerConstants {
  static constexpr uint8_t kMappedPotCount = static_cast<uint8_t>(MappedPot::kMixPot) + 1;
  static constexpr uint16_t kMinExprValue = 0;
  static constexpr uint16_t kMaxExprValue = 1023;
}

/**
 * @brief Handles mapping and logic for expression pedal input.
 */
struct ExprHandler {
  ExprState m_state = ExprState::kInactive;
  MappedPot m_mappedPot = MappedPot::kPot0;
  Direction m_direction = Direction::kNormal;
  uint16_t m_heelValue = 0;
  uint16_t m_toeValue = 1023;

  uint16_t mapAdcValue(uint16_t t_adcValue) const;
  ExprState toggleExprState();
  MappedPot changeMappedPot(int8_t t_delta);
  Direction toggleDirection();
  uint16_t changeHeelValue(int8_t t_delta);
  uint16_t changeToeValue(int8_t t_delta);
};
