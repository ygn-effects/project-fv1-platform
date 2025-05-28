#pragma once

#include <stdint.h>
#include <Arduino.h>
#include <Wire.h>
#include "periphs/pollable.h"

class Pcf8574Expander : public Pollable {
  private:
    uint8_t m_address;
    uint8_t m_mask;

    uint8_t readAll();

  public:
    Pcf8574Expander(uint8_t t_address);

    void init() override;
    size_t poll(Event* t_outEvents, size_t t_maxEvents) override;
    bool readPin(uint8_t t_pin) const;
    void WritePin(uint8_t t_pin, bool t_value);
};
