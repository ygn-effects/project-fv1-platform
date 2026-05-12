#include "eeprom.h"

void EEPROM::setup() {
  Wire.begin();
  // 100kHz is safe for all 24xx series EEPROMs.
  // If your hardware has pull-ups < 4.7k, you could try 400000 for speed.
  Wire.setClock(100000);
}

bool EEPROM::isReady() {
  // Send a dummy write command to check for ACK
  Wire.beginTransmission(m_i2cAddress);
  return Wire.endTransmission() == 0;
}

bool EEPROM::waitForReady(uint16_t timeout) {
  uint32_t start = millis();

  // ACK Polling Loop
  // The EEPROM will NACK all requests while it is performing an internal write cycle.
  // As soon as the write is complete, it will ACK.
  do {
    if (isReady()) {
      return true;
    }
    // No delay needed here; the I2C transaction time acts as a natural yield.
  } while (millis() - start < timeout);

  return false; // Timed out
}

EEPROMResult EEPROM::writeByte(uint16_t address, uint8_t data, uint8_t maxRetries) {
  for (uint8_t attempt = 0; attempt < maxRetries; attempt++) {
    Wire.beginTransmission(m_i2cAddress);
    Wire.write(highByte(address));
    Wire.write(lowByte(address));
    Wire.write(data);

    uint8_t error = Wire.endTransmission();

    if (error == 0) {
      // Transmission succeeded, now wait for the internal write cycle to finish
      if (waitForReady(10)) { // Standard max write time is 5ms, 10ms provides safety margin
        return EEPROMResult::Success;
      } else {
        return EEPROMResult::Timeout;
      }
    }

    // If endTransmission failed (e.g., bus error), we loop again to retry sending.
  }

  return EEPROMResult::WriteError;
}

EEPROMResult EEPROM::readByte(uint16_t address, uint8_t &data, uint8_t maxRetries) {
  for (uint8_t attempt = 0; attempt < maxRetries; attempt++) {
    // 1. Set Address Pointer
    Wire.beginTransmission(m_i2cAddress);
    Wire.write(highByte(address));
    Wire.write(lowByte(address));

    if (Wire.endTransmission(false) != 0) {
      continue; // Failed to set address, retry
    }

    // 2. Request Data
    if (Wire.requestFrom(m_i2cAddress, (uint8_t)1) == 1) {
      data = Wire.read();
      return EEPROMResult::Success;
    }
  }

  return EEPROMResult::ReadError;
}

EEPROMResult EEPROM::writePage(uint16_t address, const uint8_t *data, size_t length, uint8_t maxRetries) {
  for (uint8_t attempt = 0; attempt < maxRetries; attempt++) {
    Wire.beginTransmission(m_i2cAddress);
    Wire.write(highByte(address));
    Wire.write(lowByte(address));
    Wire.write(data, length);

    uint8_t error = Wire.endTransmission();

    if (error == 0) {
      // Transmission succeeded, wait for write cycle to complete
      if (waitForReady(10)) {
        return EEPROMResult::Success;
      } else {
        return EEPROMResult::Timeout;
      }
    }
  }

  return EEPROMResult::WriteError;
}

EEPROMResult EEPROM::readPage(uint16_t address, uint8_t *data, size_t length, uint8_t maxRetries) {
  for (uint8_t attempt = 0; attempt < maxRetries; attempt++) {
    // 1. Set Address Pointer
    Wire.beginTransmission(m_i2cAddress);
    Wire.write(highByte(address));
    Wire.write(lowByte(address));

    // Use restart (false) to keep control of the bus
    if (Wire.endTransmission(false) != 0) {
      continue;
    }

    // 2. Request Data Block
    // Note: wire.requestFrom returns the number of bytes received
    if (Wire.requestFrom(m_i2cAddress, (uint8_t)length) == length) {
      for (size_t i = 0; i < length; i++) {
        if (Wire.available()) {
          data[i] = Wire.read();
        } else {
          return EEPROMResult::ReadError; // Should not happen if requestFrom returned length
        }
      }
      return EEPROMResult::Success;
    }
  }

  return EEPROMResult::ReadError;
}