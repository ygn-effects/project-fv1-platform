#pragma once

#include <Arduino.h>
#include "periphs/dac.h"
#include "periphs/gpio.h"
#include "periphs/fv1.h"

class Fv1Driver : public Fv1 {
  private:
    DigitalGpio& m_gpioS0;
    DigitalGpio& m_gpioS1;
    DigitalGpio& m_gpioS2;

    Dac& m_dacP0;
    Dac& m_dacP1;
    Dac& m_dacP2;

  public:
    Fv1Driver(DigitalGpio& t_gpioS0, DigitalGpio& t_gpioS1, DigitalGpio& t_gpioS2,
              Dac& t_dacP0, Dac& t_dacP1, Dac& t_dacP2) :
      m_gpioS0(t_gpioS0),
      m_gpioS1(t_gpioS1),
      m_gpioS2(t_gpioS2),
      m_dacP0(t_dacP0),
      m_dacP1(t_dacP1),
      m_dacP2(t_dacP2) {}

    void init() override {
      m_gpioS0.init();
      m_gpioS0.write(0);
      m_gpioS1.init();
      m_gpioS1.write(0);
      m_gpioS2.init();
      m_gpioS2.write(0);

      m_dacP0.init();
      m_dacP0.write(0);
      m_dacP1.init();
      m_dacP1.write(0);
      m_dacP2.init();
      m_dacP2.write(0);
    }

    void sendProgramChange(uint8_t t_program) override {
      m_gpioS0.write(t_program & 0x1);
      m_gpioS1.write((t_program >> 1) & 0x1);
      m_gpioS2.write((t_program >> 2) & 0x1);
    }

    void sendPotValue(Fv1Pot t_pot, uint16_t t_value) override {
      switch (t_pot) {
        case Fv1Pot::Pot0:
          m_dacP0.write(t_value);
          break;
        case Fv1Pot::Pot1:
          m_dacP1.write(t_value);
          break;
        case Fv1Pot::Pot2:
          m_dacP2.write(t_value);
          break;

        default:
          break;
      }
    }
};
