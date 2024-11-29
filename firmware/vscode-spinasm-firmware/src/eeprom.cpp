#include "EEPROM.h"

void EEPROM::setup() {
  Wire.setClock(10000);
  Wire.begin();
}

bool EEPROM::isReady() {
  Wire.beginTransmission(i2cAddress);
  return Wire.endTransmission() == 0;
}

uint8_t EEPROM::writeByte(uint16_t address, uint8_t data) {
  Wire.beginTransmission(i2cAddress);
  Wire.write(highByte(address));
  Wire.write(lowByte(address));
  Wire.write(data);
  return Wire.endTransmission();
}

uint8_t EEPROM::readByte(uint16_t address) {
  Wire.beginTransmission(i2cAddress);
  Wire.write(highByte(address));
  Wire.write(lowByte(address));
  Wire.endTransmission();

  Wire.requestFrom(i2cAddress, (uint8_t)1);
  return Wire.available() ? Wire.read() : 0;
}

uint8_t EEPROM::writePage(uint16_t address, const uint8_t *data, size_t length) {
  Wire.beginTransmission(i2cAddress);
  Wire.write(highByte(address));
  Wire.write(lowByte(address));
  for (size_t i = 0; i < length; ++i) {
      Wire.write(data[i]);
  }
  return Wire.endTransmission();
}

void EEPROM::readPage(uint16_t address, uint8_t *data, size_t length) {
  Wire.beginTransmission(i2cAddress);
  Wire.write(highByte(address));
  Wire.write(lowByte(address));
  Wire.endTransmission();

  Wire.requestFrom(i2cAddress, length);
  for (size_t i = 0; i < length; ++i) {
    if (Wire.available()) {
      data[i] = Wire.read();
    }
  }
}
