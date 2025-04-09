#include "led.h"
#include "utils/logging.h"

void Led::setup() {
  pinMode(m_pin, OUTPUT);
  digitalWrite(m_pin, m_ledState);
}

void Led::turnOn() {
  setState(HIGH);

  LOG_DEBUG("LED pin %d : ON", m_pin);
}

void Led::turnOff() {
  setState(LOW);

  LOG_DEBUG("LED pin %d : OFF", m_pin);
}

void Led::setState(uint8_t t_state) {
  m_ledState = t_state;
  digitalWrite(m_pin, m_ledState);

  LOG_DEBUG("LED pin %d : set %d", m_pin, m_ledState);
}

void Led::toggle() {
  setState(!m_ledState);

  LOG_DEBUG("LED pin %d : toggle %d", m_pin, m_ledState);
}

uint8_t Led::getState() const {
  return m_ledState;
}

void Led::blink(uint8_t t_interval) {
  uint32_t currentTime = millis();

  if ((currentTime - m_lastBlinkTime) >= t_interval) {
    toggle();
    m_lastBlinkTime = currentTime;
  }
}

void PwmLed::setBrightness(uint8_t t_brightness) {
  analogWrite(m_pin, t_brightness);

  LOG_DEBUG("PWM LED pin %d : brightness %d", m_pin, t_brightness);
}
