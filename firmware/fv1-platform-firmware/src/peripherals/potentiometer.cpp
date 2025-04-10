#include "peripherals/potentiometer.h"

uint16_t Potentiometer::readRaw() const {
  uint32_t total = 0;

  for (uint8_t i = 0; i < c_numSamples; ++i) {
    total += analogRead(m_pin);
    delayMicroseconds(250);
  }

  return total / c_numSamples;
}

void Potentiometer::setup() {
  pinMode(m_pin, INPUT);
  m_lastValue = readRaw();
}

uint16_t Potentiometer::read() {
  m_lastValue = readRaw();

  return m_lastValue;
}

bool Potentiometer::hasChanged() {
  uint16_t newValue = readRaw();

  if (abs(newValue - m_lastValue) > m_threshold) {
    m_lastValue = newValue;

    return true;
  }

  return false;
}

uint16_t Potentiometer::getLastValue() const {
  return m_lastValue;
}
