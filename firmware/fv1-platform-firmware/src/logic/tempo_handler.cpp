#include "logic/tempo_handler.h"

uint16_t TempoHandler::mapInterval(int16_t t_value) {
  switch (m_source) {
    case TempoSource::kTap:
      m_interval = Utils::clamp<uint16_t>(static_cast<uint16_t>(t_value), m_minInterval, m_maxInterval);
      break;

    case TempoSource::kPot:
    m_interval = Utils::mapValue<uint16_t>(static_cast<uint16_t>(t_value), DACConstants::c_minDacValue, DACConstants::c_maxDacValue, m_minInterval, m_maxInterval);
      break;

    case TempoSource::kMenu:
    m_interval = Utils::clampedAdd(m_interval, t_value, m_minInterval, m_maxInterval);
      break;

    default:
      break;
  }

  return m_interval;
}

uint8_t TempoHandler::calculateTempoLedValue(uint32_t t_now) {
  uint32_t elapsed = t_now - m_ledLastUpdate;

  if (elapsed >= 20) {
    m_ledLastUpdate = t_now;
    uint16_t delta = (255 * elapsed) / (m_interval / 2);

    if (m_ledIncreasing) {
      m_ledValue += delta;

      if (m_ledValue >= 255) {
        m_ledValue = 255;
        m_ledIncreasing = false;
      }
    }
    else {
      m_ledValue -= delta;

      if (m_ledValue <= 0) {
        m_ledValue = 0;
        m_ledIncreasing = true;
      }
    }

  }
  return Utils::mapValue<uint8_t>(m_ledValue, 0, 255, 32, 255);
}
