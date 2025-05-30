#pragma once

#include <Arduino.h>
#include "periphs/gpio.h"
#include "periphs/toggleable.h"

namespace hal {

class Bypass : public Toggleable {
  private:
    DigitalGpio& m_relayGpio;
    DigitalGpio& m_optoCouplerGpio;
    DigitalGpio& m_led;
    bool m_state;

    void setState(bool t_state) {
      m_optoCouplerGpio.write(1);
      delay(10);
      m_relayGpio.write(t_state);
      m_led.write(t_state);
      delay(10);
      m_optoCouplerGpio.write(0);
    }

  public:
    Bypass(DigitalGpio& t_relayGpio, DigitalGpio& t_optoGpio, DigitalGpio& t_led)
      : m_relayGpio(t_relayGpio), m_optoCouplerGpio(t_optoGpio), m_led(t_led), m_state(false) {}

    void init() override {
      m_relayGpio.init();
      m_optoCouplerGpio.init();
      setState(m_state);
    }

    void on() override {
      m_state = true;
      setState(m_state);
    }

    void off() override {
      m_state = false;
      setState(m_state);
    }

    void toggle() override {
      m_state = !m_state;
      setState(m_state);
    }
};

}
