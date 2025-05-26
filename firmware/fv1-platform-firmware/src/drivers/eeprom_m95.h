#pragma once

#include <stdint.h>
#include <Arduino.h>
#include <SPI.h>
#include "core/event.h"
#include "drivers/gpio_driver.h"
#include "periphs/eeprom.h"
#include "utils/utils.h"

namespace hal {

class M95Driver : EEPROM {
  private:
    DigitalGpioDriver m_csPin;

    void select();
    void deselect();

    uint8_t readStatusRegister();
    void waitUntilReady();

    void sendAddress(uint16_t t_address);

    static constexpr uint8_t c_OpCodeWRITE = 0x02; // 0000 0010
    static constexpr uint8_t c_OpCodeREAD = 0x03; // 0000 0011
    static constexpr uint8_t c_OpCodeRDSR = 0x05; // 0000 0101
    static constexpr uint8_t c_OpCodeWREN = 0x06; // 0000 0110
    static constexpr uint8_t c_OpCodeWIP = 0x01; // 0000 0001
    static constexpr uint8_t c_pageSize = 64;

  public:
  M95Driver(DigitalGpioDriver t_csPin);

    void init() override;
    virtual void read(uint16_t t_address, uint8_t* t_data, size_t t_length) override;
    virtual void write(uint16_t t_address, const uint8_t* t_data, size_t t_length) override;
};

}
