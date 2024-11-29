#pragma once

#include <Arduino.h>
#include "circular_buffer.h"

namespace ProgrammerConstants {
  constexpr uint8_t c_startMarker = 30;
  constexpr uint8_t c_endMarker = 31;
}

class Programmer {
  private:
    uint8_t m_fv1Pin;
    CircularBuffer<64> m_buffer;

  public:
    Programmer(uint8_t t_pin) : m_fv1Pin(t_pin) { };

    void setup();
    void receiveData();
    bool getMessage(uint8_t* t_data, uint8_t t_count);
    void sendMessage(uint8_t* t_data, uint8_t t_count);
};