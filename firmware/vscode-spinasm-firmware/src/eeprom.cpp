#include "eeprom.h"

void EEPROM::setup() {
  Wire.begin();
  Wire.setClock(100000); // Standard 100kHz I²C speed
}

bool EEPROM::isReady() {
  Wire.beginTransmission(m_i2cAddress);
  return Wire.endTransmission() == 0;
}

bool EEPROM::waitForReady(uint16_t timeout) {
  uint32_t start = millis();

  // Poll until EEPROM responds or timeout
  while (millis() - start < timeout) {
    if (isReady())
      return true;

    delay(1);
  }

  return false; // EEPROM didn't respond within timeout
}

EEPROMResult EEPROM::writeByte(uint16_t address, uint8_t data, uint8_t maxRetries) {
  for (uint8_t attempt = 0; attempt < maxRetries; attempt++) {
    Wire.beginTransmission(m_i2cAddress);
    Wire.write(highByte(address));
    Wire.write(lowByte(address));
    Wire.write(data);

    // Attempt to write byte and confirm EEPROM is ready afterwards
    if (Wire.endTransmission() == 0 && waitForReady())
      return EEPROMResult::Success;

    delay(2); // Brief pause before retrying
  }

  return EEPROMResult::WriteError; // Write failed after retries
}

EEPROMResult EEPROM::readByte(uint16_t address, uint8_t &data, uint8_t maxRetries) {
  for (uint8_t attempt = 0; attempt < maxRetries; attempt++) {
    Wire.beginTransmission(m_i2cAddress);
    Wire.write(highByte(address));
    Wire.write(lowByte(address));

    // Issue repeated-start condition for read operation
    if (Wire.endTransmission(false) == 0) {
      Wire.requestFrom(m_i2cAddress, (uint8_t)1);

      if (Wire.available()) {
        data = Wire.read();
        return EEPROMResult::Success;
      }
    }

    delay(2); // Brief pause before retrying
  }

  return EEPROMResult::ReadError; // Read failed after retries
}

EEPROMResult EEPROM::writePage(uint16_t address, const uint8_t *data, size_t length, uint8_t maxRetries) {
  for (uint8_t attempt = 0; attempt < maxRetries; attempt++) {
    Wire.beginTransmission(m_i2cAddress);
    Wire.write(highByte(address));
    Wire.write(lowByte(address));
    Wire.write(data, length);

    // Attempt to write page and confirm EEPROM readiness
    if (Wire.endTransmission() == 0 && waitForReady(20))
      return EEPROMResult::Success;

    delay(5); // Page write takes longer; wait before retrying
  }

  return EEPROMResult::WriteError; // Page write failed after retries
}

EEPROMResult EEPROM::readPage(uint16_t address, uint8_t *data, size_t length, uint8_t maxRetries) {
  for (uint8_t attempt = 0; attempt < maxRetries; attempt++) {
    Wire.beginTransmission(m_i2cAddress);
    Wire.write(highByte(address));
    Wire.write(lowByte(address));

    // Initiate repeated-start read
    if (Wire.endTransmission(false) == 0) {
      Wire.requestFrom(m_i2cAddress, length);

      size_t count = 0;
      uint32_t start = millis();

      // Read data with timeout safeguard
      while ((count < length) && (millis() - start < 10)) {
        if (Wire.available())
          data[count++] = Wire.read();
      }

      if (count == length)
        return EEPROMResult::Success;
    }

    delay(2); // Brief pause before retrying
  }

  return EEPROMResult::ReadError; // Page read failed after retries
}
