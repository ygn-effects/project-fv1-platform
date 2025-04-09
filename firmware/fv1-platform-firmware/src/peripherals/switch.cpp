#include <Arduino.h>
#include "switch.h"
#include "utils/logging.h"

void MomentarySwitch::setup() {
  pinMode(m_pin, INPUT_PULLUP);
}

void MomentarySwitch::poll() {
  // Read the current state of the switch
  m_switchState = digitalRead(m_pin);
  m_now = millis();

  // Debounce handling
  if (m_switchState != m_lastSwitchState) {
    // Reset the debounce timer
    m_lastDebounceTime = m_now;
  }

  if ((m_now - m_lastDebounceTime) > m_debouncePeriod) {
    // If the button state has changed
    if (m_switchState != m_debouncedState)
    {
      m_debouncedState = m_switchState;

      if (m_debouncedState == HIGH) {
        // Button released
        m_longPressActive = false;

        LOG_DEBUG("Switch pin %d : released", m_pin);
      }
      else {
        // Button pressed
        m_lastPushedTime = m_now;

        LOG_DEBUG("Switch pin %d : pushed", m_pin);
      }

      // Record the switch event as toggled
      m_tempSwitchSwitched = true;

      LOG_DEBUG("Switch pin %d : switched", m_pin);
    }
    else if (m_debouncedState == LOW && !m_longPressActive)
    {
      // Check for long press
      if ((m_now - m_lastPushedTime) > m_longPressPeriod) {
        m_longPressActive = true;
        m_tempSwitchLongPress = true;

        LOG_DEBUG("Switch pin %d : long press", m_pin);
      }
    }
  }

  // Save the reading for next time
  m_lastSwitchState = m_switchState;
}

bool MomentarySwitch::isSwitched() const {
  return m_tempSwitchSwitched;
}

bool MomentarySwitch::isOn() const {
  return !m_debouncedState;
}

bool MomentarySwitch::isPushed() const {
  return m_tempSwitchSwitched && !m_debouncedState;
}

bool MomentarySwitch::isReleased() const {
  return m_tempSwitchSwitched && m_debouncedState;
}

bool MomentarySwitch::isLongPress() const {
  return m_tempSwitchLongPress;
}

void MomentarySwitch::reset() {
  m_tempSwitchSwitched = false;
  m_tempSwitchLongPress = false;
}
