#include "drivers/dac_mcp49xx.h"

namespace hal {

void Mcp49xx::select() {
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  m_csPin.write(0);
}

void Mcp49xx::deselect() {
  m_csPin.write(1);
  SPI.endTransaction();
}

uint16_t Mcp49xx::makeDacWord(uint16_t t_data12, Type t_type, BufferMode t_buffer, GainMode t_gain, ShutdownMode t_shutdown, Channel t_channel) {
  t_data12 &= 0xFFFu;
  uint16_t word = 0;

  if (t_type == Type::kDualChannel) {
    word |= (static_cast<uint16_t>(t_channel) & 0x1) << 15;
  }
  else {
    word |= 0 << 15;
  }

  word |= (static_cast<uint16_t>(t_buffer) & 0x1) << 14;
  word |= (static_cast<uint16_t>(t_gain) & 0x1) << 13;
  word |= (static_cast<uint16_t>(t_shutdown) & 0x1) << 12;
  word |= t_data12;

  return word;
}

uint16_t Mcp49xx::make8bitDacWord(uint8_t t_data8, Type t_type, BufferMode t_buffer, GainMode t_gain, ShutdownMode t_shutdown, Channel t_channel) {
  uint16_t code = (static_cast<uint16_t>(t_data8) & 0x00FFu) << 4;
  uint16_t word = (makeDacWord(code, t_type, t_buffer, t_gain, t_shutdown, t_channel));

  return word;
}

uint16_t Mcp49xx::make10bitDacWord(uint16_t t_data10, Type t_type, BufferMode t_buffer, GainMode t_gain, ShutdownMode t_shutdown, Channel t_channel) {
  uint16_t code = (t_data10 & 0x03FFu) << 2;
  uint16_t word = (makeDacWord(code, t_type, t_buffer, t_gain, t_shutdown, t_channel));

  return word;
}

Mcp49xx::Mcp49xx(DigitalGpio& t_csPin)
  : m_csPin(t_csPin) {}

void Mcp49xx::init() {
  m_csPin.init();
  m_csPin.write(1);
  SPI.begin();
}

Mcp4911::Mcp4911(DigitalGpio& t_csPin)
  : Mcp49xx(t_csPin) {}

void Mcp4911::write(uint16_t t_value) {
  select();

  uint16_t word = make10bitDacWord(t_value, Type::kSingleChannel, BufferMode::kBuffered, GainMode::kGain1x, ShutdownMode::kActive);
  SPI.transfer16(word);

  deselect();
}

Mcp4912::Mcp4912(DigitalGpio& t_csPin, Channel t_channel)
  : Mcp49xx(t_csPin), m_channel(t_channel) {}

void Mcp4912::write(uint16_t t_value) {
  select();

  uint16_t word = make10bitDacWord(t_value, Type::kDualChannel, BufferMode::kBuffered, GainMode::kGain1x, ShutdownMode::kActive, m_channel);
  SPI.transfer16(word);

  deselect();
}

}