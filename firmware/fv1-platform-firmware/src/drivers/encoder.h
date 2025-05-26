#pragma once

#include <stdint.h>
#include <Arduino.h>
#include "core/event.h"
#include "drivers/gpio_driver.h"
#include "periphs/pollable.h"
#include "ui/inputs.h"

namespace hal {

class EncoderDriver : public Pollable {
  private:
    DigitalGpioDriver m_gpioA;
    DigitalGpioDriver m_gpioB;
    uint8_t m_prevState;
    int8_t m_accumulator;
    EncoderId m_encoderId;

    static constexpr uint8_t c_stepsPerDetent = 4;
    static constexpr int8_t c_lookupTable[16] = {
      /*00→00*/  0, /*00→01*/ +1, /*00→10*/ -1, /*00→11*/  0,
      /*01→00*/ -1, /*01→01*/  0, /*01→10*/  0, /*01→11*/ +1,
      /*10→00*/ +1, /*10→01*/  0, /*10→10*/  0, /*10→11*/ -1,
      /*11→00*/  0, /*11→01*/ -1, /*11→10*/ +1, /*11→11*/  0,
    };

    uint8_t readState();

  public:
    EncoderDriver(DigitalGpioDriver t_gpioA, DigitalGpioDriver t_gpioB, EncoderId t_id);

    void init() override;
    size_t poll(Event* t_outEvents, size_t t_maxEvents) override;
};

}
