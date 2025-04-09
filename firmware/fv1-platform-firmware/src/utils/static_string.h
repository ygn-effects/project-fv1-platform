#pragma once

#include <Arduino.h>

class StaticString {
  private:
    static constexpr uint8_t c_maxSize = 5;
    char m_buffer[c_maxSize];
    uint8_t m_length;

  public:
    StaticString() : m_length(0) {
      clear();
    }

    StaticString(uint8_t t_value) {
      clear();
      append(t_value);
    }

    void clear();

    bool append(char c);

    bool append(uint8_t value);

    const char* c_str() const;
};
