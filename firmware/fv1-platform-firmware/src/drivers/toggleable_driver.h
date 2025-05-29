#pragma once

#include <Arduino.h>
#include <stdint.h>
#include "periphs/gpio.h"
#include "periphs/toggleable.h"

namespace hal {

class ToggleableDriver : public Toggleable {
  private:
    DigitalGpio& m_gpio;
    bool m_state;

  public:
    ToggleableDriver(DigitalGpio& t_gpio, bool t_state = false)
      : m_gpio(t_gpio), m_state(t_state) {}

    void init() override {
      m_gpio.init();
      m_gpio.write(m_state);
    }

    void on() override {
      m_state = true;
      m_gpio.write(m_state);
    }

    void off() override {
      m_state = false;
      m_gpio.write(m_state);
    }

    void toggle() override {
      m_state = !m_state;
      m_gpio.write(m_state);
    }
};

}
