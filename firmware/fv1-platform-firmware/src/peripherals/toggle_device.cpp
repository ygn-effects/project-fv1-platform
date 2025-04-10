#include "toggle_device.h"

void ToggleDevice::updatePin()  {
  digitalWrite(m_pin, m_state == m_activeHigh ? HIGH : LOW);
}

void ToggleDevice::setup() {
  pinMode(m_pin, OUTPUT);
  updatePin();
}

void ToggleDevice::on() {
  m_state = true;
  updatePin();
}

void ToggleDevice::off() {
  m_state = false;
  updatePin();
}

void ToggleDevice::toggle() {
  m_state = !m_state;
  updatePin();
}

uint8_t ToggleDevice::getState() const {
  return m_state;
}

void ToggleDevice::setState(uint8_t t_state) {
  m_state = t_state;
  updatePin();
}
