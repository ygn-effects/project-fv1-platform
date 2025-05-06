#pragma once

#include <stdint.h>
#include "utils/enum_utils.h"

enum class TapState : uint8_t {
  kDisabled,
  kEnabled
};

using TapStateValidator = EnumUtils::EnumValidator<TapState, TapState::kDisabled, TapState::kEnabled>;

enum class DivState : uint8_t {
  kDisabled,
  kEnabled
};

using DivStateValidator = EnumUtils::EnumValidator<DivState, DivState::kDisabled, DivState::kEnabled>;

enum class DivValue : uint8_t {
  kQuarter,
  kEight,
  kSixteenth,
  kDottedEight,
  kEightTriplet
};

using DivValueValidator = EnumUtils::EnumValidator<DivValue, DivValue::kQuarter, DivValue::kEight, DivValue::kSixteenth, DivValue::kDottedEight, DivValue::kEightTriplet>;

namespace TapHandlerConstants {
  static constexpr uint8_t kDivValueCount = static_cast<uint8_t>(DivValue::kEightTriplet) + 1;
}

struct TapHandler {
  private:
    void calculateInterval();
    void calculateDivInterval();

    uint32_t m_firstTapTime{0};                 // First time tapped
    uint32_t m_lastTapTime{0};                  // Last time tapped
    uint8_t m_timesTapped{0};                   // Times tapped
    const uint8_t c_minTaps{2};                 // Min # of taps before triggering an interval calculation

  public:
    TapState m_tapState{TapState::kDisabled};   // Tap state
    DivState m_divState{DivState::kDisabled};   // Interval divider state
    DivValue m_divValue{DivValue::kQuarter};    // Interval divider value
    uint16_t m_interval{0};                     // Interval
    uint16_t m_divInterval{0};                  // Interval / divider value

    uint16_t m_tapTimeout{1000};                // Timeout before reset
    bool m_isNewIntervalSet{false};             // Callback flag to check if a new interval is set

    void registerTap(uint32_t t_currentTime);
    void setNextDivValue();
};
