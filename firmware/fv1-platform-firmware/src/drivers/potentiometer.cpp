#include "drivers/potentiometer.h"

namespace hal {

uint16_t PotDriver::readRaw() {
  uint32_t total = 0;

  for (uint8_t i = 0; i < c_sampleSize; i++) {
    total += m_gpio.read();
    delayMicroseconds(200);
  }

  return total / c_sampleSize;
}

PotDriver::PotDriver(AnalogGpioDriver t_gpio, PotId t_id, uint16_t t_threshold)
  : m_gpio(t_gpio), m_potId(t_id), m_threshold(t_threshold), m_lastValue(0) {}

void PotDriver::init() {
  m_gpio.init();
  m_lastValue = readRaw();
}

size_t PotDriver::poll(Event* t_outEvents, size_t t_maxEvents) {
  size_t eventCount = 0;
  uint16_t value = readRaw();

  if (abs(value - m_lastValue) > m_threshold) {
    m_lastValue = value;
    if (eventCount >= t_maxEvents) return eventCount;

    Event e;
    e.m_type = EventType::kPotMoved;
    e.m_timestamp = millis();
    e.m_data.id = static_cast<uint8_t>(m_potId);
    e.m_data.value = value;
    t_outEvents[eventCount++] = e;
  }

  return eventCount;
}

}
