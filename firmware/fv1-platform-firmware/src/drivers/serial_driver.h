#pragma once

#include <Arduino.h>
#include "periphs/serial.h"

class SerialDriver : public SerialInterface {
  private:
    uint32_t m_baudRate = 0;

  public:
    SerialDriver(uint32_t t_baudrate) :
      m_baudRate(t_baudrate) {}

    void init() override {
      Serial.begin(m_baudRate);
    }

    bool available() const override {
      return Serial.available();
    }

    uint8_t read() override {
      return Serial.read();
    }
};
