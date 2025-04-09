#include <SPI.h>

#include "eeprom.h"

void Eeprom::setup() {
  pinMode(m_csPin, OUTPUT);
  digitalWrite(m_csPin, HIGH);
  SPI.begin();

  uint8_t statusRegister = readStatusRegister();

    // SRWD is set
  if ((statusRegister & B10000000) == 1 ||
    // BP1 is set
    (statusRegister & B00001000) == 1 ||
    // BP0 is set
    (statusRegister & B00000100) == 1) {
      writeStatusRegister(); // Reset the status register
    }
}

void Eeprom::select() {
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  digitalWrite(m_csPin, LOW);
}

void Eeprom::deselect() {
  digitalWrite(m_csPin, HIGH);
  SPI.endTransaction();
}

void Eeprom::enableWrite() {
  select();
  SPI.transfer(EEPROM_WREN);
  deselect();
}

void Eeprom::sendAddress(uint16_t t_address) {
  SPI.transfer(highByte(t_address));
  SPI.transfer(lowByte(t_address));
}

void Eeprom::writeStatusRegister() {
  while (isWip()) {}

  enableWrite();
  select();
  SPI.transfer(EEPROM_WRSR);
  SPI.transfer(0);  // Reset status register to default
  deselect();
}

uint8_t Eeprom::readStatusRegister() {
  select();
  SPI.transfer(EEPROM_RDSR);
  uint8_t data = SPI.transfer(0x00);
  deselect();

  return data;
}

bool Eeprom::isWip() {
  uint8_t status = readStatusRegister();

  return (status & 0x01);
}

uint8_t Eeprom::readInt8(uint16_t t_address) {
  while (isWip()) {}

  select();
  SPI.transfer(EEPROM_READ);
  sendAddress(t_address);
  uint8_t data = SPI.transfer(0x00);
  deselect();

  return data;
}

void Eeprom::readInt8(uint16_t t_address, uint8_t* t_data) {
  while (isWip()) {}

  select();
  SPI.transfer(EEPROM_READ);
  sendAddress(t_address);
  *t_data = SPI.transfer(0x00);
  deselect();
}

void Eeprom::writeInt8(uint16_t t_address, uint8_t t_data) {
  while (isWip()) {}

  enableWrite();
  select();
  SPI.transfer(EEPROM_WRITE);
  sendAddress(t_address);
  SPI.transfer(t_data);
  deselect();
}

uint16_t Eeprom::readInt16(uint16_t t_address) {
  while (isWip()) {}

  select();
  SPI.transfer(EEPROM_READ);
  sendAddress(t_address);
  uint8_t highbyte = SPI.transfer(0x00);
  uint8_t lowbyte = SPI.transfer(0x00);
  deselect();

  uint16_t result = (highbyte << 8) | lowbyte;
  return result;
}

void Eeprom::writeInt16(uint16_t t_address, uint16_t t_data) {
  while (isWip()) {}

  enableWrite();
  select();
  SPI.transfer(EEPROM_WRITE);
  sendAddress(t_address);
  SPI.transfer(t_data >> 8);
  SPI.transfer(t_data & 0xFF);
  deselect();
}

void Eeprom::readArray(uint16_t t_address, uint8_t* t_data, uint8_t t_length) {
  for (uint8_t i = 0; i < t_length; i++) {
    t_data[i] = readInt8(t_address + i);
  }
}

void Eeprom::writeArray(uint16_t t_address, uint8_t* t_data, uint8_t t_length) {
  for (uint8_t i = 0; i < t_length; i++) {
    writeInt8(t_address + i, t_data[i]);
  }
}
