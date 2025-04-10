#include "peripherals/bypass.h"

void Bypass::setup() {
  m_relay.setup();
  m_optocoupler.setup();
}

void Bypass::setState(uint8_t t_state) {
  m_optocoupler.on();
  delay(10);
  m_relay.setState(t_state);
  delay(10);
  m_optocoupler.off();
}

void Bypass::switchState() {
  m_optocoupler.on();
  delay(10);
  m_relay.toggle();
  delay(10);
  m_optocoupler.off();
}
