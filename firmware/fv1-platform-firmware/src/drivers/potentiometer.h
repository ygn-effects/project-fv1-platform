#pragma once

#include <stdint.h>
#include <Arduino.h>
#include "core/event.h"
#include "periphs/pollable.h"
#include "ui/inputs.h"

namespace hal {

class PotDriver : public Pollable {
  private:
    uint8_t m_pin;
    PotId m_potId;
    uint16_t m_threshold;
    uint16_t m_lastValue;

    static constexpr uint8_t c_sampleSize = 3;

    uint16_t readRaw();

  public:
    PotDriver(uint8_t t_pin, PotId t_id, uint16_t t_threshold = 4);

    void init() override;
    size_t poll(Event* t_outEvents, size_t t_maxEvents) override;
};

}
