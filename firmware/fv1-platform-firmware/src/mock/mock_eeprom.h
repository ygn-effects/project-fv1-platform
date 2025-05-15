#pragma once

#include <stdint.h>
#include <stddef.h>
#include <algorithm>
#include "hal/eeprom.h"

class MockEEPROM : public EEPROM {
  public:
    static constexpr size_t c_size = 32768;
    uint8_t m_memory[c_size] = {0};

    void read(uint16_t t_address, uint8_t* t_data, size_t t_length) override {
      if (t_address + t_length > c_size) return;
      std::copy(m_memory + t_address, m_memory + t_address + t_length, t_data);
    }

    void write(uint16_t t_address, const uint8_t* t_data, size_t t_length) override  {
      if (t_address + t_length > c_size) return;
      std::copy(t_data, t_data + t_length, m_memory + t_address);
    }
};
