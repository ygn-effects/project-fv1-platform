#pragma once

#include <Arduino.h>
#include <Wire.h>

class EEPROM {
  private:
    uint8_t i2cAddress;

  public:
    EEPROM(uint8_t address) : i2cAddress(address) { }

    void setup();

    bool isReady();

    uint8_t writeByte(uint16_t address, uint8_t data);

    uint8_t readByte(uint16_t address);

    uint8_t writePage(uint16_t address, const uint8_t *data, size_t length);

    void readPage(uint16_t address, uint8_t *data, size_t length);
};
