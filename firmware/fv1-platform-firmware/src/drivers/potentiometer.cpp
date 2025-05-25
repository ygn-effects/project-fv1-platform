#include "drivers/potentiometer.h"

namespace hal {

uint16_t PotDriver::readRaw() {
  uint32_t total = 0;

  for (uint8_t i = 0; i < c_sampleSize; i++) {
    total += analogRead(m_pin);
    delayMicroseconds(200);
  }

  return total / c_sampleSize;
}

PotDriver::PotDriver(uint8_t t_pin, PotId t_id, uint16_t t_threshold)
  : m_pin(t_pin), m_potId(t_id), m_threshold(t_threshold), m_lastValue(0) {}

void PotDriver::init() {
  pinMode(m_pin, INPUT);
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
