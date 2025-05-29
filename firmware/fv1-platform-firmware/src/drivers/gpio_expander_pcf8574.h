#pragma once

#include <stdint.h>
#include <Arduino.h>
#include <Wire.h>
#include "periphs/pollable.h"
#include "periphs/gpio_expander.h"

namespace hal {

class Pcf8574Expander : public Pollable, public GpioExpander {
  private:
    uint8_t m_address;
    uint8_t m_mask;

    uint8_t readAll(){
      Wire.requestFrom(m_address, uint8_t(1));
      return Wire.read();
    }

  public:
    Pcf8574Expander(uint8_t t_address)
    : m_address(t_address), m_mask(0xFF) {}

    void init() override {
      Wire.begin(m_address);
      m_mask = readAll();
    }

    size_t poll(Event* t_outEvents, size_t t_maxEvents) override {
      size_t eventCount = 0;
      m_mask = readAll();

      return eventCount;
    }

    bool readPin(uint8_t t_pin) override {
      return (m_mask >> t_pin) & 1;
    }

    void writePin(uint8_t t_pin, bool t_value) override {
      t_value
        ? m_mask |= (1 << t_pin)
        : m_mask &= ~(1 << t_pin);

      Wire.beginTransmission(m_address);
      Wire.write(m_mask);
      Wire.endTransmission();
    }
};

}
