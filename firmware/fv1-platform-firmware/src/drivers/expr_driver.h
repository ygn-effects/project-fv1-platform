#pragma once

#include <stdint.h>
#include <Arduino.h>
#include "core/event.h"
#include "periphs/gpio.h"
#include "periphs/pollable.h"

namespace hal {

enum class ExprDriverState : uint8_t {
  connected = 0,
  disconnected = 1,
  debouncing = 2
};

class ExprDriver : public Pollable {
  private:
    DigitalGpio& m_detectPin;
    Pollable& m_pot;
    ExprDriverState m_state{ExprDriverState::disconnected};
    uint32_t m_stateMs{0};
    uint16_t m_debounceTime = 200;

  public:
    ExprDriver(DigitalGpio& t_pin, Pollable& t_pot) :
      m_detectPin(t_pin),
      m_pot(t_pot) {}

    void init() override;
    size_t poll(Event* t_outEvents, size_t t_maxEvents) override;
};

}
