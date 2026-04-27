#pragma once

#include <stdint.h>
#include <Arduino.h>
#include "core/event.h"
#include "periphs/gpio.h"
#include "periphs/pollable.h"

namespace hal {


class ExprDriver : public Pollable {
  private:
    DigitalGpio& m_detectPin;
    Pollable& m_pot;
    uint8_t m_state{0};
    bool m_isDebouncing{false};
    uint32_t m_stateMs{0};
    uint16_t m_debounceTime = 100;
    bool m_firstPoll{true};

  public:
    ExprDriver(DigitalGpio& t_pin, Pollable& t_pot) :
      m_detectPin(t_pin),
      m_pot(t_pot) {}

    void init() override;
    size_t poll(Event* t_outEvents, size_t t_maxEvents) override;
};

}
