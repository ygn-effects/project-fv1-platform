#include "eeprom.h"

void EEPROM::setup() {
  Wire.begin();
  Wire.setClock(100000);
}

bool EEPROM::isReady() {
  Wire.beginTransmission(m_i2cAddress);
  return Wire.endTransmission() == 0;
}

bool EEPROM::waitForReady(uint16_t timeout) {
  uint32_t start = millis();
  while (millis() - start < timeout) {
    if (isReady()) return true;
    delay(1);
  }
  return false; // Timeout
}

EEPROMResult EEPROM::writeByte(uint16_t address, uint8_t data, uint8_t maxRetries) {
  for (uint8_t attempt = 0; attempt < maxRetries; attempt++) {
    Wire.beginTransmission(m_i2cAddress);
    Wire.write(highByte(address));
    Wire.write(lowByte(address));
    Wire.write(data);

    if (Wire.endTransmission() == 0 && waitForReady())
      return EEPROMResult::Success;

    delay(2);
  }
  return EEPROMResult::WriteError;
}

EEPROMResult EEPROM::readByte(uint16_t address, uint8_t &data, uint8_t maxRetries) {
  for (uint8_t attempt = 0; attempt < maxRetries; attempt++) {
    Wire.beginTransmission(m_i2cAddress);
    Wire.write(highByte(address));
    Wire.write(lowByte(address));

    if (Wire.endTransmission(false) == 0) {
      Wire.requestFrom(m_i2cAddress, (uint8_t)1);
      if (Wire.available()) {
        data = Wire.read();
        return EEPROMResult::Success;
      }
    }

    delay(2);
  }
  return EEPROMResult::ReadError;
}

EEPROMResult EEPROM::writePage(uint16_t address, const uint8_t *data, size_t length, uint8_t maxRetries) {
  for (uint8_t attempt = 0; attempt < maxRetries; attempt++) {
    Wire.beginTransmission(m_i2cAddress);
    Wire.write(highByte(address));
    Wire.write(lowByte(address));
    Wire.write(data, length);

    if (Wire.endTransmission() == 0 && waitForReady(20))
      return EEPROMResult::Success;

    delay(5);
  }
  return EEPROMResult::WriteError;
}

EEPROMResult EEPROM::readPage(uint16_t address, uint8_t *data, size_t length, uint8_t maxRetries) {
  for (uint8_t attempt = 0; attempt < maxRetries; attempt++) {
    Wire.beginTransmission(m_i2cAddress);
    Wire.write(highByte(address));
    Wire.write(lowByte(address));

    if (Wire.endTransmission(false) == 0) {
      Wire.requestFrom(m_i2cAddress, length);
      size_t count = 0;
      uint32_t start = millis();

      while ((count < length) && (millis() - start < 10)) {
        if (Wire.available())
          data[count++] = Wire.read();
      }

      if (count == length)
        return EEPROMResult::Success;
    }

    delay(2);
  }

  return EEPROMResult::ReadError;
}
