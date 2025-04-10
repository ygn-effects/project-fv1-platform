#pragma once

#include "peripherals/toggle_device.h"

class Led : public ToggleDevice {
  public:
    Led(uint8_t t_pin, bool t_activeHigh = true) : ToggleDevice(t_pin, t_activeHigh) {}
};

class BlinkLed : public ToggleDevice {
  private:
    uint32_t m_lastBlinkTime = 0;
    uint8_t m_lastBlinkState = 0;

  public:
    BlinkLed(uint8_t t_pin, bool t_activeHigh = true) : ToggleDevice(t_pin, t_activeHigh) {}
    void blink(uint8_t t_interval);
};

class PwmLed : public ToggleDevice {
  public:
    PwmLed(uint8_t t_pin, bool t_activeHigh = true) : ToggleDevice(t_pin, t_activeHigh) {}
    void setBrightness(uint8_t t_brightness) const;
};