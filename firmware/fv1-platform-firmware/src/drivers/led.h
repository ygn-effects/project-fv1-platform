#pragma once

#include <Arduino.h>
#include <stdint.h>
#include <drivers/toggleable_driver.h>
#include "periphs/adjustable.h"
#include "periphs/gpio.h"
#include "utils/utils.h"

namespace hal {

using LedDriver = ToggleableDriver;

class BlinkingLedDriver : public LedDriver {
  private:
    uint16_t m_interval;
    uint32_t m_lastBlinkTime;

  public:
    BlinkingLedDriver(DigitalGpio& t_gpio, uint16_t t_interval, bool t_state = false)
      : LedDriver(t_gpio, t_state), m_interval(t_interval), m_lastBlinkTime(0) {}

    void update(uint32_t t_now) {
      if ((t_now - m_lastBlinkTime) >= m_interval) {
        m_lastBlinkTime = t_now;
        toggle();
      }
    }
};

class AdjustableLedDriver : public Adjustable {
  private:
    AnalogGpio& m_gpio;

  public:
    AdjustableLedDriver(AnalogGpio& t_gpio)
      : m_gpio(t_gpio) {}

    void init() override {
      m_gpio.init();
    }

    void off() override {
      m_gpio.write(0);
    }

    void setValue(uint8_t t_value) {
      m_gpio.write(t_value);
    }
};

}
