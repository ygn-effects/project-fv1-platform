#pragma once

#include <stdint.h>

class EEPROM {
  public:
    virtual void init() = 0;
    virtual void read(uint16_t t_address, uint8_t* t_data, size_t t_length) = 0;
    virtual void write(uint16_t t_address, const uint8_t* t_data, size_t t_length) = 0;
};
