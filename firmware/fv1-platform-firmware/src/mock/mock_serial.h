#pragma once

#include "periphs/serial.h"
#include "utils/circular_buffer.h"

class MockedSerial : public SerialInterface {
  private:
    CircularBuffer<uint8_t, 32> m_buffer;
    bool m_initialized = false;

  public:
    void init() override {
      m_initialized = true;
    }

    bool available() const override {
      return !m_buffer.isEmpty();
    }

    uint8_t read() override {
      uint8_t byte = 0;
      m_buffer.pop(byte);
      return byte;
    }

    void feedByte(uint8_t t_byte) {
      m_buffer.push(t_byte);
    }
};
