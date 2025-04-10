#pragma once

#include <Arduino.h>

class ToggleDevice {
  protected:
    uint8_t m_pin;
    uint8_t m_state;
    bool m_activeHigh;

    void updatePin();

  public:
    ToggleDevice(uint8_t t_pin, bool t_activeHigh = true)
      : m_pin(t_pin), m_state(0), m_activeHigh(t_activeHigh) {}

    void setup();

    void on();

    void off();

    void toggle();

    uint8_t getState() const;

    void setState(uint8_t t_state);

  protected:
};

class Relay : public ToggleDevice {
  public:
    Relay(uint8_t t_pin, bool t_activeHigh = true) : ToggleDevice(t_pin, t_activeHigh) {}
};

class Optocoupler : public ToggleDevice {
  public:
    Optocoupler(uint8_t t_pin, bool t_activeHigh = true) : ToggleDevice(t_pin, t_activeHigh) {}
};
