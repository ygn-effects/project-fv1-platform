#include "static_string.h"

void StaticString::clear() {
  m_length = 0;
  m_buffer[0] = '\0';
}

bool StaticString::append(char t_char) {
  if (m_length < c_maxSize - 1) {
    m_buffer[m_length++] = t_char;
    m_buffer[m_length] = '\0';

    return true;
  }

  return false;
}

bool StaticString::append(uint8_t t_value) {
  if (m_length < c_maxSize - 1) {
    char temp[4];
    uint8_t len = 0;

    if (t_value >= 100) {
      temp[len++] = '0' + (t_value / 100);
      t_value %= 100;
    }

    if (t_value >= 10 || len > 0) {
      temp[len++] = '0' + (t_value / 10);
      t_value %= 10;
    }

    temp[len++] = '0' + t_value;
    temp[len] = '\0';

    for (uint8_t i = 0; i < len; i++) {
      if (!append(temp[i])) {
        return false;
      }
    }

    return true;
  }

  return false;
}

bool StaticString::append(const char* str) {
  if (!str) return false;

  while (*str) {
    if (!append(*str++)) {
      return false;
    }
  }

  return true;
}

const char* StaticString::c_str() const {
  return m_buffer;
}