#pragma once

#include <stdint.h>
#include <stddef.h>
#include <algorithm>

class MockEEPROM {
  public:
    static constexpr size_t c_size = 32768;
    uint8_t m_memory[c_size] = {0};

    void write(uint16_t address, const uint8_t* data, size_t length) {
      if (address + length > c_size) return;
      std::copy(data, data + length, m_memory + address);
    }

    void read(uint16_t address, uint8_t* data, size_t length) const {
      if (address + length > c_size) return;
      std::copy(m_memory + address, m_memory + address + length, data);
    }
};