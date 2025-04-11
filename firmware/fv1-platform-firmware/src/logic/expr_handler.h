#pragma once

#include <Arduino.h>

/**
 * @brief Expression pedal state (active/inactive)
 */
enum class ExprState : uint8_t {
  kInactive,
  kActive
};

/**
 * @brief Parameter controlled by the expression pedal
 */
enum class MappedPot : uint8_t {
  kPot0,
  kPot1,
  kPot2,
  kMixPot
};

/**
 * @brief Direction of the expression pedal mapping
 */
enum class Direction : uint8_t {
  kNormal,   // Heel = minimum, Toe = maximum
  kInverted  // Heel = maximum, Toe = minimum
};

/**
 * @brief Handles mapping and logic for expression pedal input.
 */
class ExprHandler {
  private:
    ExprState m_state{ExprState::kInactive};
    MappedPot m_mappedPot{MappedPot::kPot0};
    Direction m_direction{Direction::kNormal};
    uint16_t m_heelValue{0};
    uint16_t m_toeValue{1023};

  public:
    ExprHandler();

    void setMappedPot(MappedPot pot);
    void setDirection(Direction direction);
    void setHeelToeValues(uint16_t heel, uint16_t toe);
    void setState(ExprState state);

    uint16_t mapAdcValue(uint16_t adcValue) const;

    ExprState getState() const { return m_state; }
    MappedPot getMappedPot() const { return m_mappedPot; }
    Direction getDirection() const { return m_direction; }
    uint16_t getHeelValue() const { return m_heelValue; }
    uint16_t getToeValue() const { return m_toeValue; }
};
