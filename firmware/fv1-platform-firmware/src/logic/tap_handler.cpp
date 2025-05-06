#include "logic/tap_handler.h"

void TapHandler::calculateInterval() {
  m_interval = (m_lastTapTime - m_firstTapTime) / (m_timesTapped - 1);
}

void TapHandler::calculateDivInterval() {
  switch (m_divValue) {
    case DivValue::kQuarter:
      m_divInterval = m_interval;
      break;

    case DivValue::kEight:
      m_divInterval = m_interval / 2;
      break;

    case DivValue::kSixteenth:
      m_divInterval = m_interval / 4;
      break;

    case DivValue::kEightTriplet:
      m_divInterval = m_interval / 3;
      break;

    case DivValue::kDottedEight:
      m_divInterval = (m_interval * 3) / 4;
      break;

    default:
      break;
  }
}

void TapHandler::registerTap(uint32_t t_currentTime) {
  m_isNewIntervalSet = false;

  if (m_timesTapped == 0 || (t_currentTime - m_lastTapTime) > m_tapTimeout) {
    // First tap or timeout
    m_firstTapTime = t_currentTime;
    m_lastTapTime = t_currentTime;
    m_interval = 0;
    m_divInterval = 0;
    m_timesTapped = 1;

    m_tapState = TapState::kDisabled;
  }
  else {
    // Subsequent taps
    m_timesTapped++;
    m_lastTapTime = t_currentTime;

    if (m_timesTapped >= c_minTaps) {
      calculateInterval();

      if (m_divState == DivState::kEnabled) {
        calculateDivInterval();
      }

      m_isNewIntervalSet = true;
      m_tapState = TapState::kEnabled;
    }
  }
}

void TapHandler::setNextDivValue() {
  m_divValue = EnumUtils::nextEnumValue<DivValue, TapHandlerConstants::kDivValueCount>(m_divValue);
  calculateDivInterval();

  m_divValue == DivValue::kQuarter
    ? m_divState = DivState::kDisabled
    : m_divState = DivState::kEnabled;
}
