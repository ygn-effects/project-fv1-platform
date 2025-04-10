#pragma once

#include <Arduino.h>

class Potentiometer {
  private:
    uint8_t m_pin;
    uint16_t m_lastValue{0};
    static constexpr uint8_t c_numSamples = 4;
    uint16_t m_threshold;

    uint16_t readRaw() const;

  public:
    Potentiometer(uint8_t t_pin, uint16_t t_threshold = 3)
      : m_pin(t_pin), m_threshold(t_threshold) {}

    void setup();

    uint16_t read();

    bool hasChanged();

    uint16_t getLastValue() const;
};
