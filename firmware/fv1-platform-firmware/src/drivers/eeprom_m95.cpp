#include "drivers/eeprom_m95.h"

namespace hal{

void M95Driver::select() {
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  m_csPin.write(0);
}

void M95Driver::deselect() {
  m_csPin.write(1);
  SPI.endTransaction();
}

uint8_t M95Driver::readStatusRegister() {
  select();
  SPI.transfer(c_OpCodeRDSR);
  uint8_t status = SPI.transfer(0x00);
  deselect();

  return status;
}

void M95Driver::writeStatusRegister() {
  waitUntilReady();

  select();
  SPI.transfer(c_OpCodeWREN);
  deselect();

  select();
  SPI.transfer(c_OpCodeWRSR);
  SPI.transfer(0);  // Reset status register to default
  deselect();
}

void M95Driver::waitUntilReady() {
  uint8_t status = 0;

  do {
    status = readStatusRegister();
  } while (status & 0x01);
}

void M95Driver::sendAddress(uint16_t t_address) {
  SPI.transfer(uint8_t(t_address >> 8));
  SPI.transfer(uint8_t(t_address & 0xFF));
}

M95Driver::M95Driver(DigitalGpio& t_csPin)
  : m_csPin(t_csPin) {}

void M95Driver::init() {
  m_csPin.init();
  m_csPin.write(1);
  SPI.begin();

  uint8_t statusRegister = readStatusRegister();
  // SRWD is set
if ((statusRegister & B10000000) ||
  // BP1 is set
  (statusRegister & B00001000) ||
  // BP0 is set
  (statusRegister & B00000100)) {
    writeStatusRegister(); // Reset the status register
  }
}

void M95Driver::read(uint16_t t_address, uint8_t* t_data, size_t t_length) {
  select();
  SPI.transfer(c_OpCodeREAD);
  sendAddress(t_address);

  while (t_length--) {
    *t_data++ = SPI.transfer(0x00);
  }

  deselect();
}

void M95Driver::write(uint16_t t_address, const uint8_t* t_data, size_t t_length) {
  while (t_length) {
    select();
    SPI.transfer(c_OpCodeWREN);
    deselect();

    size_t pageOffset = t_address % c_pageSize;
    size_t chunk = Utils::min<size_t>(t_length, c_pageSize - pageOffset);

    select();
    SPI.transfer(c_OpCodeWRITE);
    sendAddress(t_address);

    for (uint8_t i = 0; i < chunk; i++) {
      SPI.transfer(t_data[i]);
    }

    deselect();

    waitUntilReady();

    t_address += chunk;
    t_data += chunk;
    t_length -= chunk;
  }
}

}
