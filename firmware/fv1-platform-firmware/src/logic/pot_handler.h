#pragma once

#include <stdint.h>

namespace PotConstants {
  constexpr uint8_t c_potCount = 4;
}

enum class PotState : uint8_t {
  kDisabled,
  kActive
};

class PotHandler {
  private:
    PotState m_state[PotConstants::c_potCount] = {PotState::kActive};
    uint16_t m_minValue[PotConstants::c_potCount] = {0};
    uint16_t m_maxValue[PotConstants::c_potCount] = {1023};

  public:
    void setState(PotState t_state, uint8_t t_potIndex);
    void setMinValue(uint16_t t_min, uint8_t t_potIndex);
    void setMaxValue(uint16_t t_max, uint8_t t_potIndex);
};
