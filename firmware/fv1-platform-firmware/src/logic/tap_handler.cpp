#include "logic/tap_handler.h"

void TapHandler::calculateInterval() {
  m_interval = (m_lastTapTime - m_firstTapTime) / m_timesTapped;
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
    m_interval = 0;
    m_divInterval = 0;
    m_timesTapped = 1;
  }
  else {
    // Subsequent taps
    m_timesTapped++;

    if (m_timesTapped >= c_minTaps) {
      calculateInterval();

      if (m_divState == DivState::kEnabled) {
        calculateDivInterval();
      }

      m_isNewIntervalSet = true;
    }
  }

  m_lastTapTime = t_currentTime;
}

TapState TapHandler::getTapState() const {
  return m_tapState;
}

DivState TapHandler::getDivState() const {
  return m_divState;
}

DivValue TapHandler::getDivValue() const {
  return m_divValue;
}

uint16_t TapHandler::getInterval() const {
  return m_interval;
}

uint16_t TapHandler::getDivInterval() const {
  return m_divInterval;
}

bool TapHandler::getIsNewIntervalSet() const {
  return m_isNewIntervalSet;
}

void TapHandler::setTapState(TapState t_state) {
  m_tapState = t_state;
}

void TapHandler::setDivState(DivState t_state) {
  m_divState = t_state;
}

void TapHandler::setDivValue(DivValue t_value) {
  m_divValue = t_value;
}

void TapHandler::setInterval(uint16_t t_interval) {
  m_interval = t_interval;
}

void TapHandler::setDivInterval(uint16_t t_divInterval) {
  m_divInterval = t_divInterval;
}

void TapHandler::setTapTimeout(uint16_t t_timeout) {
  m_tapTimeout = t_timeout;
}
