#pragma once

#include <stdint.h>

class StaticString {
  private:
    static constexpr uint8_t c_maxSize = 32;
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

    StaticString(const char* str) : m_length(0) {
      clear();
      append(str);
    }

    void clear();

    bool append(char c);

    bool append(uint8_t value);

    bool append(const char* str);

    const char* c_str() const;
};
