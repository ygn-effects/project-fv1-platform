#pragma once

#include <stdint.h>
#include <Arduino.h>
#include "periphs/gpio.h"

class DigitalGpioDriver : public DigitalGpio {
  private:
      uint8_t m_pin;
      bool m_pullup;

  public:
    DigitalGpioDriver(uint8_t t_pin, bool t_pullup = false)
      : m_pin(t_pin), m_pullup(t_pullup) {}

    void init() override {
      pinMode(m_pin, m_pullup ? INPUT_PULLUP : INPUT);
    }

    bool read() override {
      return digitalRead(m_pin);
    }

    void write(bool t_value) override {
      pinMode(m_pin, OUTPUT);
      digitalWrite(m_pin, t_value);
    }
};

class AnalogGpioDriver : public AnalogGpio {
  private:
    uint8_t m_pin;

  public:
    AnalogGpioDriver(uint8_t t_pin) : m_pin(t_pin) {}

    void init() override {
      pinMode(m_pin, INPUT);
    }

    uint16_t read() override {
      return analogRead(m_pin);
    }

    void write(uint16_t t_value) override {
      pinMode(m_pin, OUTPUT);
      analogWrite(m_pin, t_value);
    }
};

