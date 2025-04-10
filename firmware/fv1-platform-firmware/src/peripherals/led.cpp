#include "peripherals/led.h"

void BlinkLed::blink(uint8_t t_interval) {
  uint32_t currentTime = millis();

  if ((currentTime - m_lastBlinkTime) >= t_interval) {
    toggle();
    m_lastBlinkTime = currentTime;
  }
}

void PwmLed::setBrightness(uint8_t t_brightness) const {
  analogWrite(m_pin, t_brightness);
}