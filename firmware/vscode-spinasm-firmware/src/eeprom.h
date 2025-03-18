#pragma once

#include <Arduino.h>
#include <Wire.h>

enum class EEPROMResult {
  Success,
  Timeout,
  WriteError,
  ReadError,
  CommunicationError
};

class EEPROM {
private:
  uint8_t m_i2cAddress;

  bool waitForReady(uint16_t timeout = 10);

public:
  EEPROM(uint8_t address) : m_i2cAddress(address) {}

  void setup();

  bool isReady();

  EEPROMResult writeByte(uint16_t address, uint8_t data, uint8_t maxRetries = 3);
  EEPROMResult readByte(uint16_t address, uint8_t &data, uint8_t maxRetries = 3);

  EEPROMResult writePage(uint16_t address, const uint8_t *data, size_t length, uint8_t maxRetries = 3);
  EEPROMResult readPage(uint16_t address, uint8_t *data, size_t length, uint8_t maxRetries = 3);
};
