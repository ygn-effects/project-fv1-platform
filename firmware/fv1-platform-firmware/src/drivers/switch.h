#pragma once

#include <stdint.h>
#include <Arduino.h>
#include "core/event.h"
#include "drivers/gpio_driver.h"
#include "periphs/pollable.h"
#include "ui/inputs.h"

namespace hal {

enum class SwitchState : uint8_t {
  kIdle,
  kDebouncingDown,
  kPressed,
  kDebouncingUp,
};

class SwitchDriver : public Pollable {
  private:
    DigitalGpio& m_gpio;
    SwitchId m_switchId;
    uint16_t m_debounceMs;
    bool m_longPress;
    uint16_t m_longPressMs;
    uint32_t m_stateMs;
    SwitchState m_state;

  public:
    SwitchDriver(DigitalGpio& t_gpio, SwitchId t_id, uint32_t t_debounceMs = 20, uint32_t t_longPressMs = 500);

    void init() override;
    size_t poll(Event* t_outEvents, size_t t_maxEvents) override;
};

}
