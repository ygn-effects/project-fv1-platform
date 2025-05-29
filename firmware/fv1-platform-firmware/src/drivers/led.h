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

class BreathingLedDriver : public Adjustable {
  private:
    AnalogGpio& m_gpio;
    bool m_increasing;
    uint32_t m_lastUpdate;
    int16_t m_duty;

    void setDutyCycle(uint16_t t_cycle) override {
      m_gpio.write(t_cycle);
    }

  public:
    uint16_t m_period;

    BreathingLedDriver(AnalogGpio& t_gpio, uint16_t t_period)
      : m_gpio(t_gpio), m_increasing(true), m_lastUpdate(0), m_duty(0), m_period(t_period) {}

    void init() override {
      m_gpio.init();
    }

    void update() override {
      uint32_t now = millis();
      uint32_t elapsed = now - m_lastUpdate;

      if (elapsed >= 20) {
        m_lastUpdate = now;

        int delta = (255 * elapsed) / (m_period / 2);

        if (m_increasing) {
          m_duty += delta;
          if (m_duty >= 255) {
            m_duty = 255;
            m_increasing = false;
          }
        } else {
          m_duty -= delta;
          if (m_duty <= 0) {
            m_duty = 0;
            m_increasing = true;
          }
        }
      }

      setDutyCycle(Utils::mapValue<uint8_t>(m_duty, 0, 255, 32, 255));
    }
};

}
