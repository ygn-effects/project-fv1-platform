#pragma once

#include <stdint.h>

enum class TapState : uint8_t {
  kDisabled,
  kEnabled
};

enum class DivState : uint8_t {
  kDisabled,
  kEnabled
};

enum class DivValue : uint8_t {
  kQuarter,
  kEight,
  kSixteenth,
  kDottedEight,
  kEightTriplet
};

class TapHandler {
  private:
    TapState m_tapState{TapState::kDisabled};   // Tap state
    DivState m_divState{DivState::kDisabled};   // Interval divider state
    DivValue m_divValue{DivValue::kQuarter};    // Interval divider value
    uint16_t m_interval{0};                     // Interval
    uint16_t m_divInterval{0};                  // Interval / divider value

    uint32_t m_firstTapTime{0};                 // First time tapped
    uint32_t m_lastTapTime{0};                  // Last time tapped
    uint8_t m_timesTapped{0};                   // Times tapped
    uint16_t m_tapTimeout{0};                   // Timeout before reset
    const uint8_t c_minTaps{2};                 // Min # of taps before triggering an interval calculation

    bool m_isNewIntervalSet{false};             // Callback flag to check if a new interval is set

    void calculateInterval();
    void calculateDivInterval();

  public:
    void registerTap(uint32_t t_currentTime);

    TapState getTapState() const;
    DivState getDivState() const;
    DivValue getDivValue() const;
    uint16_t getInterval() const;
    uint16_t getDivInterval() const;
    bool getIsNewIntervalSet() const;

    void setTapState(TapState t_state);
    void setDivState(DivState t_state);
    void setDivValue(DivValue t_value);
    void setInterval(uint16_t t_interval);
    void setDivInterval(uint16_t t_divInterval);
    void setNextDivValue();
    void setTapTimeout(uint16_t t_timeout);
};
