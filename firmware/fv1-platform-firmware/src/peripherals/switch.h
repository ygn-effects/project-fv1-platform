#pragma once

#include <Arduino.h>

/// @brief Represents a momentary switch connected to a pin#,
/// it can be polled and checks for switched,
/// pushed, long pressed and released states.
class MomentarySwitch {
  private:
    uint8_t m_pin;

    uint8_t m_switchState = 0;
    uint8_t m_lastSwitchState = 0;
    uint8_t m_debouncedState = 1;

    uint32_t m_now = 0;
    uint32_t m_lastDebounceTime = 0;
    uint32_t m_lastPushedTime = 0;
    uint16_t m_longPressPeriod;
    uint8_t m_debouncePeriod;

    uint8_t m_tempSwitchSwitched = 0;
    uint8_t m_tempSwitchLongPress = 0;
    uint8_t m_longPressActive = 0;

  public:
    /// @brief Create a temporary switch object
    /// @param pin pin #
    /// @param lpperiod Long press threshold, default 1000ms
    /// @param dperiod Debounce period, default 50ms
    MomentarySwitch(
      uint8_t t_pin,
      uint16_t t_lpperiod = 1000,
      uint8_t t_dperiod = 50
      ) :
        m_pin(t_pin),
        m_longPressPeriod(t_lpperiod),
        m_debouncePeriod(t_dperiod) { };

    /// @brief Setup the switch by initializing the pin
    void setup();

    /// @brief Poll the switch to check its state (pressed, released, etc.)
    void poll();

    /// @brief Check if the switch state has been toggled (on/off)
    bool isSwitched() const;

    /// @brief Check if the switch is currently on (pressed down)
    bool isOn() const;

    /// @brief Check if the switch was pushed (pressed down this cycle)
    bool isPushed() const;

    /// @brief Check if the switch was released (let go this cycle)
    bool isReleased() const;

    /// @brief Check if the switch has been held down for a long press
    bool isLongPress() const;

    /// @brief Reset the temporary state flags (switched, long press)
    void reset();
};
