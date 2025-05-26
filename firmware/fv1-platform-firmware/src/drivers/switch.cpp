#include "drivers/switch.h"

namespace hal {

SwitchDriver::SwitchDriver(DigitalGpioDriver t_gpio, SwitchId t_id, uint32_t t_debounceMs, uint32_t t_longPressMs)
  : m_gpio(t_gpio), m_switchId(t_id), m_debounceMs(t_debounceMs),
    m_longPressMs(t_longPressMs), m_stateMs(0), m_state(SwitchState::kIdle) {}

void SwitchDriver::init() {
  m_gpio.init();
}

size_t SwitchDriver::poll(Event* t_outEvents, size_t t_maxEvents) {
  size_t eventCount = 0;
  bool switchEvent = !m_gpio.read();

  uint32_t now = millis();
  uint32_t elapsed = now - m_stateMs;

  switch (m_state) {
    case SwitchState::kIdle:
      if (switchEvent) {
        m_state = SwitchState::kDebouncingDown;
        m_stateMs = now;
      }
      break;

    case SwitchState::kDebouncingDown:
      if (! switchEvent) { m_state = SwitchState::kIdle; }
      else if (elapsed >= m_debounceMs) {
        m_state = SwitchState::kPressed;
        m_stateMs = now;

        Event e;
        e.m_type = EventType::kSwitchPressed;
        e.m_timestamp = now;
        e.m_data.id = static_cast<uint8_t>(m_switchId);
        t_outEvents[eventCount++] = e;
      }
      break;

    case SwitchState::kPressed:
      if (switchEvent && elapsed >= m_longPressMs) {
        m_state = SwitchState::kLongPressed;

        Event e;
        e.m_type = EventType::kSwitchLongPressed;
        e.m_timestamp = now;
        e.m_data.id = static_cast<uint8_t>(m_switchId);
        t_outEvents[eventCount++] = e;
      }
      else if (! switchEvent) {
        m_state = SwitchState::kDebouncingUp;
        m_stateMs = now;
      }
      break;

    case SwitchState::kLongPressed:
      if (! switchEvent) {
        m_state = SwitchState::kDebouncingUp;
        m_stateMs = now;
      }
      break;

    case SwitchState::kDebouncingUp:
      if (switchEvent) {
        m_state = SwitchState::kPressed;
        m_stateMs = now;
      }
      else if (elapsed >= m_debounceMs) {
        m_state = SwitchState::kIdle;
        m_stateMs = now;

        Event e;
        e.m_type = EventType::kSwitchReleased;
        e.m_timestamp = now;
        e.m_data.id = static_cast<uint8_t>(m_switchId);
        t_outEvents[eventCount++] = e;
      }
      break;

    default:
      break;
  }

  return eventCount;
}

}
